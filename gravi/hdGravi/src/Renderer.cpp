///
/// @file Renderer.cpp
/// @brief The Gravi renderer

#include <random>

#include <pxr/imaging/hd/perfLog.h>
#include <pxr/base/gf/matrix3f.h>
#include <pxr/base/gf/ray.h>
#include <pxr/base/gf/vec2f.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/base/tf/debug.h>
#include <tbb/parallel_for.h>
#include <tbb/blocked_range2d.h>

#include "RenderBuffer.h"
#include "Mesh.h"
#include "RenderSettings.h"

#include "Renderer.h"

PXR_NAMESPACE_OPEN_SCOPE

//-------------------------------------------------------------------------
// Construction / Destruction
//-------------------------------------------------------------------------

HdGraviRenderer::HdGraviRenderer()
    : m_width(0)
    , m_height(0)
    , m_viewMatrix(1.0f)
    , m_projMatrix(1.0f)
    , m_inverseViewMatrix(1.0f)
    , m_inverseProjMatrix(1.0f)
    , m_completedSamples(0)
    , m_numBounces(pxr::defaultBounces)
    , m_lightStepSize(pxr::defaultLightStepSize)
    , m_minLightStepSize(pxr::defaultMinLightStepSize)
    , m_maxLightStepSize(pxr::defaultMaxLightStepSize)
    , m_maxLightDistance(pxr::defaultMaxLightDistance)
    , m_maxLightSteps(pxr::defaultMaxLightSteps)
    , m_samplesToConvergence(pxr::defaultConvergedSamplesPerPixel)
    {}

//-------------------------------------------------------------------------
// Scene Configuration
//-------------------------------------------------------------------------

void HdGraviRenderer::SetScene(const HdGraviScene& _scene) { m_scene = _scene; };

void HdGraviRenderer::BuildBVH() { m_scene.BuildBVH(); }

//-------------------------------------------------------------------------
// Rendering Configuration
//-------------------------------------------------------------------------

void HdGraviRenderer::SetDataWindow(const GfRect2i &_dataWindow)
{
    m_dataWindow = _dataWindow;

    // Here for clients that do not use camera framing but the
    // viewport.
    //
    // Re-validate the attachments, since attachment viewport and
    // render viewport need to match.
   m_aovBindingsNeedValidation = true;
}

void HdGraviRenderer::SetCamera(const GfMatrix4d &_viewMatrix, const GfMatrix4d &_projMatrix)
{
   m_viewMatrix = _viewMatrix;
   m_projMatrix = _projMatrix;
   m_inverseViewMatrix = _viewMatrix.GetInverse();
   m_inverseProjMatrix = _projMatrix.GetInverse();
}

void HdGraviRenderer::SetAovBindings(HdRenderPassAovBindingVector const &_aovBindings)
{
   m_aovBindings = _aovBindings;
   m_aovNames.resize(m_aovBindings.size());
    for (size_t i = 0; i <m_aovBindings.size(); ++i)
    {
       m_aovNames[i] = HdParsedAovToken(m_aovBindings[i].aovName);
    }

    // Re-validate the attachments.
   m_aovBindingsNeedValidation = true;
}

HdRenderPassAovBindingVector const& HdGraviRenderer::GetAovBindings() const { return m_aovBindings;}

void HdGraviRenderer::SetSamplesToConvergence(int _samplesToConvergence) { m_samplesToConvergence = _samplesToConvergence; }

void HdGraviRenderer::SetNumberBounces(int _numberBounces) { m_numBounces = _numberBounces; }

void HdGraviRenderer::SetLightStepSize(double _size) { m_lightStepSize = _size; }

void HdGraviRenderer::SetMinLightStepSize(double _size) { m_minLightStepSize = _size; }

void HdGraviRenderer::SetMaxLightStepSize(double _size) { m_maxLightStepSize = _size; }

void HdGraviRenderer::SetMaxLightDistance(double _dist) { m_maxLightDistance = _dist; }

void HdGraviRenderer::SetMaxLightSteps(int _steps) { m_maxLightSteps = _steps; }

//-------------------------------------------------------------------------
// Rendering Operations
//-------------------------------------------------------------------------

static bool _IsContained(const GfRect2i &_rect, const int _width, const int _height)
{
    return _rect.GetMinX() >= 0 && _rect.GetMaxX() < _width &&
           _rect.GetMinY() >= 0 && _rect.GetMaxY() < _height;
}

void HdGraviRenderer::Render(HdRenderThread *_renderThread)
{
    m_completedSamples.store(0);

    if (!_ValidateAovBindings()) {
        // We aren't going to render anything. Just mark all AOVs as converged
        // so that we will stop rendering.
        for (size_t i = 0; i < m_aovBindings.size(); ++i) {
            HdGraviRenderBuffer *rb = static_cast<HdGraviRenderBuffer*>(
                m_aovBindings[i].renderBuffer);
            rb->SetConverged(true);
        }
        // XXX:validation
        TF_WARN("Could not validate Aovs. Render will not complete");
        return;
    }

    m_width = 0;
    m_height = 0;

    // Map all of the attachments.
    for (size_t i = 0; i < m_aovBindings.size(); ++i)
    {
        //
        // XXX
        //
        // A scene delegate might specify the path to a
        // render buffer instead of a pointer to the
        // render buffer.
        //
        dynamic_cast<HdGraviRenderBuffer *>(
           m_aovBindings[i].renderBuffer)
            ->Map();

        if (i == 0)
        {
            m_width = static_cast<int>(m_aovBindings[i].renderBuffer->GetWidth());
            m_height = static_cast<int>(m_aovBindings[i].renderBuffer->GetHeight());
        }
        else
        {
            if (m_width !=m_aovBindings[i].renderBuffer->GetWidth() ||
               m_height !=m_aovBindings[i].renderBuffer->GetHeight())
            {
                TF_CODING_ERROR(
                    "Template render buffers have inconsistent sizes");
            }
        }
    }

    if (m_width > 0 || m_height > 0) {
        if (!_IsContained(m_dataWindow, m_width, m_height)) {
            TF_CODING_ERROR(
                "dataWindow is larger than render buffer");

        }
    }

    auto origin = GfVec3f(m_inverseViewMatrix.Transform(GfVec3f(0,0,0)));

    std::default_random_engine random(0);

    // Create a uniform distribution for jitter calculations.
    std::uniform_real_distribution<float> uniform_dist(0.0f, 1.0f);
    auto uniform_float = [&random, &uniform_dist]() { return uniform_dist(random); };

    //_scene->SortByDepth(origin);

    for (int s = 0; s < m_samplesToConvergence; ++s) {
        tbb::parallel_for(tbb::blocked_range2d<int>(0, m_dataWindow.GetHeight(),
            0, m_dataWindow.GetWidth()),
            [&](const tbb::blocked_range2d<int>& r) {
                for (int y = r.rows().begin(); y != r.rows().end(); ++y)
                {
                    for (int x = r.cols().begin(); x != r.cols().end(); ++x) {
                        if (_renderThread->IsStopRequested()) { break; }
                            GfVec4f Cd(0.0f, 0.0f, 0.0f, 1.0f);
                            GfVec3f N(0.0f);
                            GfVec3f P(0.0f);
                            int32_t id;
                            float z = 0;
                            auto jitter = GfVec2f(uniform_float(), uniform_float());

                            if (m_samplesToConvergence <= 1) {
                                jitter *= 0;
                            }

                            const auto w(static_cast<float>(m_dataWindow.GetWidth()));
                            const auto h(static_cast<float>(m_dataWindow.GetHeight()));

                            const GfVec3f ndc(
                            2 * ((static_cast<float>(x) + jitter[0] -static_cast<float>(m_dataWindow.GetMinX())) / w) - 1,
                            2 * ((static_cast<float>(y) + jitter[1] -static_cast<float>(m_dataWindow.GetMinY())) / h) - 1,
                            -1);

                            const GfVec3f nearPlaneTrace(m_inverseProjMatrix.Transform(ndc));

                            GfVec3f dir = GfVec3f(m_inverseViewMatrix.TransformDir(nearPlaneTrace)).GetNormalized();

                            GfRay ray = GfRay(GfVec3d(origin), GfVec3d(dir));

                            HitData hit = m_scene.Intersect(ray, m_numBounces, m_lightStepSize, m_minLightStepSize, m_maxLightStepSize, m_maxLightDistance, m_maxLightSteps);

                            if (hit.m_hit) {
                                Cd += hit.m_Cd;
                                N += hit.m_N;
                                P += hit.m_P;
                                z += hit.m_z;
                                id = hit.m_id;
                            } else {
                                continue;
                            }
                                    // Set pixels for each AOV
                            for (size_t i = 0; i <m_aovBindings.size(); ++i)
                            {
                                        HdGraviRenderBuffer *renderBuffer = static_cast<HdGraviRenderBuffer *>(m_aovBindings[i].renderBuffer);

                                        if (m_aovNames[i].name == HdAovTokens->color)
                                        {
                                            renderBuffer->Write(GfVec3i(x, y, 1), 4, Cd.data());
                                        }
                                        if ((m_aovNames[i].name == HdAovTokens->cameraDepth || m_aovNames[i].name == HdAovTokens->depth) && renderBuffer->GetFormat() == HdFormatFloat32)
                                        {
                                            renderBuffer->Write(GfVec3i(x, y, 1), 1, &z);
                                        }
                                        if (m_aovNames[i].name == HdAovTokens->Peye && renderBuffer->GetFormat() == HdFormatFloat32Vec3)
                                        {
                                            renderBuffer->Write(GfVec3i(x, y, 1), 3, P.data());
                                        }
                                        if ((m_aovNames[i].name == HdAovTokens->Neye ||
                                        m_aovNames[i].name == HdAovTokens->normal) &&
                                        renderBuffer->GetFormat() == HdFormatFloat32Vec3)
                                        {
                                            renderBuffer->Write(GfVec3i(x, y, 1), 3, N.data());
                                        }
                                        if ((m_aovNames[i].name == HdAovTokens->primId ||
                                        m_aovNames[i].name == HdAovTokens->elementId ||
                                        m_aovNames[i].name == HdAovTokens->instanceId) &&
                                        renderBuffer->GetFormat() == HdFormatInt32)
                                        {
                                            renderBuffer->Write(GfVec3i(x,y,1), 1, &id);
                                        }
                                        if (m_aovNames[i].isPrimvar &&
                                        renderBuffer->GetFormat() == HdFormatFloat32Vec3)
                                        {
                                            GfVec3f value;
                                            renderBuffer->Write(GfVec3i(x, y, 1), 3, value.data());
                                        }
                            }
                        }
                    }
                    m_completedSamples.store(s+1);
                });
    }

    // Check for convergence
    bool converged = (m_completedSamples >= m_samplesToConvergence);
    for (const auto& aovBinding : m_aovBindings) {
        auto* rb = dynamic_cast<HdGraviRenderBuffer*>(aovBinding.renderBuffer);
        rb->Unmap();
        rb->SetConverged(converged);
    }
}

void HdGraviRenderer::Clear()
{

    for (size_t i = 0; i < m_aovBindings.size(); ++i)
    {
        if (m_aovBindings[i].clearValue.IsEmpty())
        {
            continue;
        }

        HdGraviRenderBuffer *rb = static_cast<HdGraviRenderBuffer *>(m_aovBindings[i].renderBuffer);

        rb->Map();
        if (m_aovNames[i].name == HdAovTokens->color)
        {
            GfVec4f clearColor = _GetClearColor(m_aovBindings[i].clearValue);
            rb->Clear(4, clearColor.data());
        }
        else if (rb->GetFormat() == HdFormatInt32)
        {
            int32_t clearValue = m_aovBindings[i].clearValue.Get<int32_t>();
            rb->Clear(1, &clearValue);
        }
        else if (rb->GetFormat() == HdFormatFloat32)
        {
            float clearValue = m_aovBindings[i].clearValue.Get<float>();
            rb->Clear(1, &clearValue);
        }
        else if (rb->GetFormat() == HdFormatFloat32Vec3)
        {
            GfVec3f clearValue = m_aovBindings[i].clearValue.Get<GfVec3f>();
            rb->Clear(3, clearValue.data());
        }

        rb->Unmap();
        rb->SetConverged(false);
    }
}

void HdGraviRenderer::MarkAovBuffersUnconverged()
{
    for (const auto & m_aovBinding : m_aovBindings)
    {
        auto *rb =
            dynamic_cast<HdGraviRenderBuffer *>(m_aovBinding.renderBuffer);
        rb->SetConverged(false);
    }
}

int HdGraviRenderer::GetCompletedSamples() const { return m_completedSamples.load(); }

void HdGraviRenderer::AddLight(const HdGraviLight* _light)
{
    m_lights.push_back(_light);
}

void HdGraviRenderer::UpdateLights()
{
    m_scene.ClearLights();
    m_scene.SetLights(m_lights);
}

//-------------------------------------------------------------------------
// Private Implementation
//-------------------------------------------------------------------------

bool HdGraviRenderer::_ValidateAovBindings()
{
    if (!m_aovBindingsNeedValidation)
    {
        return m_aovBindingsValid;
    }

   m_aovBindingsNeedValidation = false;
   m_aovBindingsValid = true;

    for (size_t i = 0; i <m_aovBindings.size(); ++i)
    {

        // By the time the attachment gets here, there should be a bound
        // output buffer.
        if (m_aovBindings[i].renderBuffer == nullptr)
        {
            TF_WARN("Aov '%s' doesn't have any renderbuffer bound",
                   m_aovNames[i].name.GetText());
           m_aovBindingsValid = false;
            continue;
        }

        if (m_aovNames[i].name != HdAovTokens->color &&
            m_aovNames[i].name != HdAovTokens->cameraDepth &&
            m_aovNames[i].name != HdAovTokens->depth &&
            m_aovNames[i].name != HdAovTokens->primId &&
            m_aovNames[i].name != HdAovTokens->instanceId &&
            m_aovNames[i].name != HdAovTokens->elementId &&
            m_aovNames[i].name != HdAovTokens->Neye &&
            m_aovNames[i].name != HdAovTokens->normal &&

            !m_aovNames[i].isPrimvar)
        {
            TF_WARN("Unsupported attachment with Aov '%s' won't be rendered to", m_aovNames[i].name.GetText());
        }

        const HdFormat format = m_aovBindings[i].renderBuffer->GetFormat();

        // depth is only supported for float32 attachments
        if ((m_aovNames[i].name == HdAovTokens->cameraDepth ||
             m_aovNames[i].name == HdAovTokens->depth) &&
            format != HdFormatFloat32)
        {
            TF_WARN("Aov '%s' has unsupported format '%s'",
                    m_aovNames[i].name.GetText(),
                    TfEnum::GetName(format).c_str());
            m_aovBindingsValid = false;
        }

        // ids are only supported for int32 attachments
        if ((m_aovNames[i].name == HdAovTokens->primId ||
             m_aovNames[i].name == HdAovTokens->instanceId ||
             m_aovNames[i].name == HdAovTokens->elementId) &&
            format != HdFormatInt32) {
            TF_WARN("Aov '%s' has unsupported format '%s'",
                    m_aovNames[i].name.GetText(),
                    TfEnum::GetName(format).c_str());
            m_aovBindingsValid = false;
            }

        if ((m_aovNames[i].name == HdAovTokens->Neye ||
             m_aovNames[i].name == HdAovTokens->normal) &&
            format != HdFormatFloat32Vec3)
        {
            TF_WARN("Aov '%s' has unsupported format '%s'",
                    m_aovNames[i].name.GetText(),
                    TfEnum::GetName(format).c_str());
            m_aovBindingsValid = false;
        }

        if (m_aovNames[i].name == HdAovTokens->Peye &&
            format != HdFormatFloat32Vec3)
        {
            TF_WARN("Aov '%s' has unsupported format '%s'",
                    m_aovNames[i].name.GetText(),
                    TfEnum::GetName(format).c_str());
            m_aovBindingsValid = false;
        }

        if (m_aovNames[i].isPrimvar && format != HdFormatFloat32Vec3)
        {
            TF_WARN("Aov 'primvars:%s' has unsupported format '%s'",
                    m_aovNames[i].name.GetText(),
                    TfEnum::GetName(format).c_str());
            m_aovBindingsValid = false;
        }

        // color is only supported for vec3/vec4 attachments of float,
        // unorm, or snorm.
        if (m_aovNames[i].name == HdAovTokens->color)
        {
            switch (format)
            {
            case HdFormatUNorm8Vec4:
            case HdFormatUNorm8Vec3:
            case HdFormatSNorm8Vec4:
            case HdFormatSNorm8Vec3:
            case HdFormatFloat32Vec4:
            case HdFormatFloat32Vec3:
                break;
            default:
                TF_WARN("Aov '%s' has unsupported format '%s'",
                       m_aovNames[i].name.GetText(),
                        TfEnum::GetName(format).c_str());
               m_aovBindingsValid = false;
                break;
            }
        }

        // make sure the clear value is reasonable for the format of the
        // attached buffer.
        if (!m_aovBindings[i].clearValue.IsEmpty())
        {
            auto [type, count] =
                HdGetValueTupleType(m_aovBindings[i].clearValue);

            // array-valued clear types aren't supported.
            if (count != 1)
            {
                TF_WARN("Aov '%s' clear value type '%s' is an array",
                       m_aovNames[i].name.GetText(),
                       m_aovBindings[i].clearValue.GetTypeName().c_str());
               m_aovBindingsValid = false;
            }

            // color only supports float/double vec3/4
            if (m_aovNames[i].name == HdAovTokens->color &&
                type != HdTypeFloatVec3 &&
                type != HdTypeFloatVec4 &&
                type != HdTypeDoubleVec3 &&
                type != HdTypeDoubleVec4)
            {
                TF_WARN("Aov '%s' clear value type '%s' isn't compatible",
                       m_aovNames[i].name.GetText(),
                       m_aovBindings[i].clearValue.GetTypeName().c_str());
               m_aovBindingsValid = false;
            }

            // only clear float formats with float, int with int, float3 with
            // float3.
            if ((format == HdFormatFloat32 && type != HdTypeFloat) ||
                (format == HdFormatInt32 && type != HdTypeInt32) ||
                (format == HdFormatFloat32Vec3 &&
                 type != HdTypeFloatVec3))
            {
                TF_WARN("Aov '%s' clear value type '%s' isn't compatible with"
                        " format %s",
                       m_aovNames[i].name.GetText(),
                       m_aovBindings[i].clearValue.GetTypeName().c_str(),
                        TfEnum::GetName(format).c_str());
               m_aovBindingsValid = false;
            }
        }
    }

    return m_aovBindingsValid;
}


GfVec4f HdGraviRenderer::_GetClearColor(VtValue const &_clearValue)
{
    auto [type, count] = HdGetValueTupleType(_clearValue);
    if (count != 1)
    {
        return {0.0f, 0.0f, 0.0f, 1.0f};
    }

    switch (type)
    {
    case HdTypeFloatVec3:
    {
        GfVec3f f =
            *(static_cast<const GfVec3f *>(HdGetValueData(_clearValue)));
        return {f[0], f[1], f[2], 1.0f};
    }
    case HdTypeFloatVec4:
    {
        const GfVec4f f =
            *(static_cast<const GfVec4f *>(HdGetValueData(_clearValue)));
        return f;
    }
    case HdTypeDoubleVec3:
    {
        GfVec3d f =
            *(static_cast<const GfVec3d *>(HdGetValueData(_clearValue)));
        return {static_cast<float>(f[0]), static_cast<float>(f[1]), static_cast<float>(f[2]), 1.0f};
    }
    case HdTypeDoubleVec4:
    {
        const GfVec4d f =
            *(static_cast<const GfVec4d *>(HdGetValueData(_clearValue)));
        return GfVec4f(f);
    }
    default:
        return {0.0f, 0.0f, 0.0f, 1.0f};
    }
}

PXR_NAMESPACE_CLOSE_SCOPE