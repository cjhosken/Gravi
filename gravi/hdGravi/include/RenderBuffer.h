//
// Copyright 2025 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//

/// @file RenderBuffer.h
/// @brief Render buffer implementation for Gravi's frame output
///
/// Implements a render target buffer supporting:
/// - Single and multi-sampled rendering
/// - Various pixel formats (HdFormat)
/// - Thread-safe pixel access operations
/// - Progressive rendering convergence tracking
/// - Double-buffering for multi-sampling
/// - Efficient buffer resolution
///
/// @author Christopher Hosken
/// @version 1.0
/// @date 07/05/2025 Pre-release cleanup
/// @ingroup hdGravi

#ifndef HDGRAVI_HDGRAVIRENDERBUFFER_H_
#define HDGRAVI_HDGRAVIRENDERBUFFER_H_

#include <atomic>
#include <vector>

#include <pxr/pxr.h>
#include <pxr/imaging/hd/renderBuffer.h>

PXR_NAMESPACE_OPEN_SCOPE

//-----------------------------------------------------------------------------
/// @class HdGraviRenderBuffer
/// @brief Hydra render buffer implementation for Gravi
///
/// Manages pixel storage with features:
/// - Multiple HdFormat pixel formats
/// - Single and multi-sampled operation
/// - Thread-safe mapping operations
/// - Progressive rendering support
/// - Double-buffering for multi-sampling
/// - Automatic buffer resolution
//-----------------------------------------------------------------------------
class HdGraviRenderBuffer final : public HdRenderBuffer
{
public:
    //-------------------------------------------------------------------------
    /// @name Construction / Destruction
    //-------------------------------------------------------------------------
    /// @{

    /// @brief Constructor
    /// @param _id Scene path identifier for this buffer
    explicit HdGraviRenderBuffer(SdfPath const& _id);

    /// @brief Destructor
    ///
    /// @note Automatically deallocates any allocated buffers
    ~HdGraviRenderBuffer() override = default;

    /// @}

    //-------------------------------------------------------------------------
    /// @name Hydra Interface
    //-------------------------------------------------------------------------
    /// @{

    /// @brief Synchronize buffer state with scene delegate
    /// @param _sceneDelegate Scene data provider
    /// @param _renderParam Global render parameters
    /// @param _dirtyBits Bitmask of changed states
    ///
    /// @note Safely stops rendering before potential reallocation
    /// @note Thread-safe for parallel execution
    void Sync(HdSceneDelegate* _sceneDelegate,
              HdRenderParam* _renderParam,
              HdDirtyBits* _dirtyBits) override;

    /// @brief Finalize buffer before destruction
    /// @param _renderParam Global render parameters
    ///
    /// @note Safely stops rendering before deallocation
    void Finalize(HdRenderParam* _renderParam) override;

    /// @brief Allocate pixel storage
    /// @param _dimensions Buffer dimensions (width, height, depth)
    /// @param _format Pixel format from HdFormat enum
    /// @param _multiSampled Whether to allocate multi-sample buffers
    /// @return True if allocation succeeded
    ///
    /// @note Only depth=1 is currently supported
    bool Allocate(GfVec3i const& _dimensions,
                 HdFormat _format,
                 bool _multiSampled) override;

    /// @brief Get buffer width
    /// @return Width in pixels
    [[nodiscard]] unsigned int GetWidth() const override;

    /// @brief Get buffer height
    /// @return Height in pixels
    [[nodiscard]] unsigned int GetHeight() const override;

    /// @brief Get buffer depth
    /// @return Depth in pixels (always 1 currently)
    [[nodiscard]] unsigned int GetDepth() const override;

    /// @brief Get pixel format
    /// @return The buffer's HdFormat
    [[nodiscard]] HdFormat GetFormat() const override;

    /// @brief Check if multi-sampled
    /// @return True if buffer is multi-sampled
    [[nodiscard]] bool IsMultiSampled() const override;

    /// @brief Map buffer for access
    /// @return Pointer to pixel data
    ///
    /// @note Must be followed by Unmap() when done accessing
    void* Map() override;

    /// @brief Unmap buffer after access
    void Unmap() override;

    /// @brief Check if buffer is mapped
    /// @return True if mapped (active access)
    [[nodiscard]] bool IsMapped() const override;

    /// @brief Check if rendering is converged
    /// @return True if no more rendering is needed
    [[nodiscard]] bool IsConverged() const override;

    /// @brief Set convergence state
    /// @param _cv True to mark buffer as converged
    void SetConverged(bool _cv);

    /// @brief Resolve multi-samples to final pixels
    ///
    /// For multi-sampled buffers, combines samples into final pixel values
    void Resolve() override;

    /// @}

    //-------------------------------------------------------------------------
    /// @name Pixel Operations
    //-------------------------------------------------------------------------
    /// @{

    /// @brief Write float components to pixel
    /// @param _pixel Pixel coordinates (x,y,z)
    /// @param _numComponents Number of components (1-4)
    /// @param _value Component values
    ///
    /// @note Extra components are discarded, missing components default to 0
    /// @note Buffer must be mapped before calling
    void Write(GfVec3i const& _pixel,
               size_t _numComponents,
               float const* _value);

    /// @brief Write int components to pixel
    /// @param _pixel Pixel coordinates (x,y,z)
    /// @param _numComponents Number of components (1-4)
    /// @param _value Component values
    ///
    /// @note Extra components are discarded, missing components default to 0
    /// @note Buffer must be mapped before calling
    void Write(GfVec3i const& _pixel,
               size_t _numComponents,
               int const* _value);

    /// @brief Clear buffer with float values
    /// @param _numComponents Number of components (1-4)
    /// @param _value Component values
    ///
    /// @note Extra components are discarded, missing components default to 0
    /// @note Buffer must be mapped before calling
    void Clear(size_t _numComponents,
               float const* _value);

    /// @brief Clear buffer with int values
    /// @param _numComponents Number of components (1-4)
    /// @param _value Component values
    ///
    /// @note Extra components are discarded, missing components default to 0
    /// @note Buffer must be mapped before calling
    void Clear(size_t _numComponents,
               int const* _value);

    /// @}

private:
    //-------------------------------------------------------------------------
    /// @name Private Implementation
    //-------------------------------------------------------------------------
    /// @{

    /// @brief Calculate required buffer size
    /// @param _dims Dimensions (width, height)
    /// @param _format Pixel format
    /// @return Required size in bytes
    static size_t _GetBufferSize(GfVec2i const& _dims,
                                HdFormat _format);

    /// @brief Get sample buffer format
    /// @param _format Base pixel format
    /// @return Corresponding sample format (float32 or int32)
    static HdFormat _GetSampleFormat(HdFormat _format);

    /// @brief Deallocate all buffers
    void _Deallocate() override;

    /// @}

    //-------------------------------------------------------------------------
    /// @name Private Members
    //-------------------------------------------------------------------------
    /// @{

    int m_width = 0;            ///< Current width in pixels
    int m_height = 0;           ///< Current height in pixels
    HdFormat m_format;          ///< Pixel format
    bool m_multiSampled = false;///< Multi-sample enabled flag

    std::vector<uint8_t> m_buffer;          ///< Resolved pixel buffer
    std::vector<uint8_t> m_sampleBuffer;    ///< Multi-sample accumulation buffer
    std::vector<uint8_t> m_sampleCount;     ///< Sample count buffer (multi-sample)

    std::atomic<int> m_mappers{0};      ///< Active mapping count
    std::atomic<bool> m_converged{false};///< Convergence state

    /// @}
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif // HDGRAVI_HDGRAVIRENDERBUFFER_H_