///
/// @file RenderBuffer.cpp
/// @brief The Render Buffer for displaying renderered frames.

#include <pxr/base/gf/half.h>
#include <tbb/parallel_for.h>

#include "RenderParam.h"

#include "RenderBuffer.h"

PXR_NAMESPACE_OPEN_SCOPE

//-------------------------------------------------------------------------
// Construction / Destruction
//-------------------------------------------------------------------------

HdGraviRenderBuffer::HdGraviRenderBuffer(SdfPath const& _id)
    : HdRenderBuffer(_id)
    , m_width(0)
    , m_height(0)
    , m_format(HdFormatInvalid)
    , m_multiSampled(false)
    , m_buffer()
    , m_sampleBuffer()
    , m_sampleCount()
    , m_mappers(0)
    , m_converged(false)
    {}

//-------------------------------------------------------------------------
// Hydra Interface
//-------------------------------------------------------------------------

void HdGraviRenderBuffer::Sync(HdSceneDelegate *_sceneDelegate, HdRenderParam *_renderParam, HdDirtyBits *_dirtyBits)
{
    if (*_dirtyBits & DirtyDescription) {
        // Gravi has the background thread write directly into render buffers,
        // so we need to stop the render thread before reallocating them.
         dynamic_cast<HdGraviRenderParam*>(_renderParam)->Edit();
    }

    HdRenderBuffer::Sync(_sceneDelegate, _renderParam, _dirtyBits);
}


void HdGraviRenderBuffer::Finalize(HdRenderParam *_renderParam)
{
    // Gravi has the background thread write directly into render buffers,
    // so we need to stop the render thread before removing them.
    dynamic_cast<HdGraviRenderParam*>(_renderParam)->Edit();

    HdRenderBuffer::Finalize(_renderParam);
}

bool HdGraviRenderBuffer::Allocate(GfVec3i const& _dimensions, const HdFormat _format, bool _multiSampled)
{
    _Deallocate();

    if (_dimensions[2] != 1) {
        TF_WARN(
               "Render buffer allocated with dims <%d, %d, %d> and"
               " format %s; depth must be 1!",
               _dimensions[0], _dimensions[1], _dimensions[2],
               TfEnum::GetName(_format).c_str());
        return false;
    }

    m_multiSampled = _multiSampled;

    m_width = _dimensions[0];
    m_height = _dimensions[1];
    m_format = _format;
    m_buffer.resize(_GetBufferSize(GfVec2i(m_width, m_height), _format));

    if (m_multiSampled) {
        m_sampleBuffer.resize(_GetBufferSize(GfVec2i(m_width, m_height), _GetSampleFormat(_format)));
        m_sampleCount.resize(m_width * m_height);
    }

    return true;
}

unsigned int HdGraviRenderBuffer::GetWidth() const { return m_width; }

unsigned int HdGraviRenderBuffer::GetHeight() const { return m_height; }

unsigned int HdGraviRenderBuffer::GetDepth() const { return 1; }

HdFormat HdGraviRenderBuffer::GetFormat() const { return m_format; }

bool HdGraviRenderBuffer::IsMultiSampled() const { return false; }

void* HdGraviRenderBuffer::Map()
{
    ++m_mappers;
    return m_buffer.data();
}

void HdGraviRenderBuffer::Unmap() { --m_mappers; }

bool HdGraviRenderBuffer::IsMapped() const { return m_mappers.load() != 0; }

bool HdGraviRenderBuffer::IsConverged() const { return m_converged.load(); }

void HdGraviRenderBuffer::SetConverged(const bool _cv) { m_converged.store(_cv); }

void HdGraviRenderBuffer::Resolve()
{
    if (!m_multiSampled || m_sampleCount.size() != m_width * m_height) {
        return;
    }

    const HdFormat componentFormat = HdGetComponentFormat(m_format);
    const size_t componentCount = HdGetComponentCount(m_format);
    const size_t formatSize = HdDataSizeOfFormat(m_format);
    const size_t sampleSize = HdDataSizeOfFormat(_GetSampleFormat(m_format));

    for (unsigned int i = 0; i < m_width * m_height; ++i) {
        const int sampleCount = m_sampleCount[i];
        if (sampleCount == 0) {
            continue;
        }

        uint8_t* dst = &m_buffer[i * formatSize];
        uint8_t* src = &m_sampleBuffer[i * sampleSize];
        for (size_t c = 0; c < componentCount; ++c) {
            if (componentFormat == HdFormatInt32) {
                reinterpret_cast<int32_t *>(dst)[c] = reinterpret_cast<int32_t *>(src)[c] / sampleCount;
            } else if (componentFormat == HdFormatFloat16) {
                reinterpret_cast<uint16_t *>(dst)[c]
                       = GfHalf(reinterpret_cast<float *>(src)[c] / static_cast<float>(sampleCount)).bits();
            } else if (componentFormat == HdFormatFloat32) {
                reinterpret_cast<float *>(dst)[c] = reinterpret_cast<float *>(src)[c] / static_cast<float>(sampleCount);
            } else if (componentFormat == HdFormatUNorm8) {
                dst[c]
                       = static_cast<uint8_t>(reinterpret_cast<float *>(src)[c] * 255.0f / static_cast<float>(sampleCount));
            } else if (componentFormat == HdFormatSNorm8) {
                reinterpret_cast<int8_t *>(dst)[c]
                       = static_cast<int8_t>(reinterpret_cast<float *>(src)[c] * 127.0f / static_cast<float>(sampleCount));
            }
        }
    }
}

//-------------------------------------------------------------------------
// Pixel Operations
//-------------------------------------------------------------------------

template <typename T> static void _WriteSample(const HdFormat _format, uint8_t* _dst, const size_t _valueComponents, T const* _value)
{
    const HdFormat componentFormat = HdGetComponentFormat(_format);
    const size_t componentCount = HdGetComponentCount(_format);

    for (size_t c = 0; c < componentCount; ++c) {
        if (componentFormat == HdFormatInt32) {
            reinterpret_cast<int32_t *>(_dst)[c]
                   += (c < _valueComponents) ? static_cast<int32_t>(_value[c]) : 0;
        } else {
            reinterpret_cast<float *>(_dst)[c]
                   += (c < _valueComponents) ? static_cast<float>(_value[c]) : 0.0f;
        }
    }
}

template <typename T> static void _WriteOutput(const HdFormat _format, uint8_t* _dst, const size_t _valueComponents, T const* _value)
{
    const HdFormat componentFormat = HdGetComponentFormat(_format);
    const size_t componentCount = HdGetComponentCount(_format);

    for (size_t c = 0; c < componentCount; ++c) {
        if (componentFormat == HdFormatInt32) {
            reinterpret_cast<int32_t *>(_dst)[c]
                   = (c < _valueComponents) ? static_cast<int32_t>(_value[c]) : 0;
        } else if (componentFormat == HdFormatFloat16) {
            reinterpret_cast<uint16_t *>(_dst)[c]
                   = (c < _valueComponents) ? GfHalf(_value[c]).bits() : 0;
        } else if (componentFormat == HdFormatFloat32) {
            reinterpret_cast<float *>(_dst)[c] = (c < _valueComponents) ? static_cast<float>(_value[c]) : 0.0f;
        } else if (componentFormat == HdFormatUNorm8) {
            _dst[c] = (c < _valueComponents)
                   ? static_cast<uint8_t>(_value[c] * 255.0f)
                   : 0.0f;
        } else if (componentFormat == HdFormatSNorm8) {
            reinterpret_cast<int8_t *>(_dst)[c]
                   = (c < _valueComponents) ? static_cast<int8_t>(_value[c] * 127.0f) : 0.0f;
        }
    }
}

void HdGraviRenderBuffer::Write(GfVec3i const& _pixel, const size_t _numComponents, float const* _value)
{
    const size_t idx = _pixel[1] * m_width + _pixel[0];
    if (m_multiSampled) {
        const size_t formatSize = HdDataSizeOfFormat(_GetSampleFormat(m_format));
        uint8_t* dst = &m_sampleBuffer[idx * formatSize];
        _WriteSample(m_format, dst, _numComponents, _value);
        m_sampleCount[idx]++;
    } else {
        const size_t formatSize = HdDataSizeOfFormat(m_format);
        uint8_t* dst = &m_buffer[idx * formatSize];
        _WriteOutput(m_format, dst, _numComponents, _value);
    }
}

void HdGraviRenderBuffer::Write(GfVec3i const& _pixel, const size_t _numComponents, int const* _value)
{
    const size_t idx = _pixel[1] * m_width + _pixel[0];
    if (m_multiSampled) {
        const size_t formatSize = HdDataSizeOfFormat(_GetSampleFormat(m_format));
        uint8_t* dst = &m_sampleBuffer[idx * formatSize];
        _WriteSample(m_format, dst, _numComponents, _value);
        m_sampleCount[idx]++;
    } else {
        const size_t formatSize = HdDataSizeOfFormat(m_format);
        uint8_t* dst = &m_buffer[idx * formatSize];
        _WriteOutput(m_format, dst, _numComponents, _value);
    }
}

void HdGraviRenderBuffer::Clear(const size_t _numComponents, float const* _value)
{
    const size_t formatSize = HdDataSizeOfFormat(m_format);

    parallel_for(tbb::blocked_range<int>(0, (m_width * m_height)),
                      [&](const tbb::blocked_range<int> r) {
                          for (int i = r.begin(); i < r.end(); ++i) {
                              uint8_t* dst = &m_buffer[i * formatSize];
                              _WriteOutput(m_format, dst, _numComponents, _value);
                          }
                      });

    if (m_multiSampled) {
        std::fill(m_sampleCount.begin(), m_sampleCount.end(), 0);
        std::fill(m_sampleBuffer.begin(), m_sampleBuffer.end(), 0);
    }
}

void HdGraviRenderBuffer::Clear(const size_t _numComponents, int const* _value)
{
    const size_t formatSize = HdDataSizeOfFormat(m_format);
    parallel_for(tbb::blocked_range<int>(0, (m_width * m_height)),
                      [&](const tbb::blocked_range<int> r) {
                          for (int i = r.begin(); i < r.end(); ++i) {
                              uint8_t* dst = &m_buffer[i * formatSize];
                              _WriteOutput(m_format, dst, _numComponents, _value);
                          }
                      });

    if (m_multiSampled) {
        std::fill(m_sampleCount.begin(), m_sampleCount.end(), 0);
        std::fill(m_sampleBuffer.begin(), m_sampleBuffer.end(), 0);
    }
}

//-------------------------------------------------------------------------
// Private Implementation
//-------------------------------------------------------------------------

size_t HdGraviRenderBuffer::_GetBufferSize(GfVec2i const& _dims, const HdFormat _format) { return _dims[0] * _dims[1] * HdDataSizeOfFormat(_format); }

HdFormat HdGraviRenderBuffer::_GetSampleFormat(const HdFormat _format)
{
    const HdFormat component = HdGetComponentFormat(_format);
    const size_t arity = HdGetComponentCount(_format);

    if (component == HdFormatUNorm8 || component == HdFormatSNorm8
        || component == HdFormatFloat16 || component == HdFormatFloat32) {
        if (arity == 1) { return HdFormatFloat32; }
        if (arity == 2) { return HdFormatFloat32Vec2; }
        if (arity == 3) { return HdFormatFloat32Vec3; }
        if (arity == 4) { return HdFormatFloat32Vec4; }
        } else if (component == HdFormatInt32) {
            if (arity == 1) { return HdFormatInt32; }
            if (arity == 2) { return HdFormatInt32Vec2; }
            if (arity == 3) { return HdFormatInt32Vec3; }
            if (arity == 4) { return HdFormatInt32Vec4; }
        }
    return HdFormatInvalid;
}

void HdGraviRenderBuffer::_Deallocate()
{

    TF_VERIFY(!IsMapped());

    m_width = 0;
    m_height = 0;
    m_format = HdFormatInvalid;
    m_multiSampled = false;
    m_buffer.resize(0);
    m_sampleBuffer.resize(0);
    m_sampleCount.resize(0);

    m_mappers.store(0);
    m_converged.store(false);

}

PXR_NAMESPACE_CLOSE_SCOPE