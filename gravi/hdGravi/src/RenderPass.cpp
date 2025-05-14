///
/// @file RenderPass.cpp
/// @brief Render passes that Gravi writes to

#include <pxr/imaging/hd/renderPassState.h>

#include "RenderDelegate.h"
#include "RenderSettings.h"
#include "Scene.h"

#include "RenderPass.h"

PXR_NAMESPACE_OPEN_SCOPE

//-------------------------------------------------------------------------
// Construction / Destruction
//-------------------------------------------------------------------------

HdGraviRenderPass::HdGraviRenderPass(HdRenderIndex *_index, HdRprimCollection const &_collection, HdRenderThread* _renderThread, HdGraviRenderer *_renderer, std::atomic<int> *_sceneVersion)
    : HdRenderPass(_index, _collection)
      , m_renderer(_renderer)
      , m_renderThread(_renderThread)
      , m_sceneVersion(_sceneVersion)
      , m_lastSceneVersion(0)
      , m_lastSettingsVersion(0)
      , m_viewMatrix()
      , m_projMatrix()
      , m_colorBuffer(SdfPath::EmptyPath())
      , m_depthBuffer(SdfPath::EmptyPath())
      , m_converged(false) {
    m_renderer->SetScene(HdGraviScene(_index));
}

// Make sure the render thread's not running, in case it's writing
// to _colorBuffer/_depthBuffer.
HdGraviRenderPass::~HdGraviRenderPass() { m_renderThread->StopRender(); }

//-------------------------------------------------------------------------
// Render State Queries
//-------------------------------------------------------------------------

bool HdGraviRenderPass::IsConverged() const
{
    // If the aov binding array is empty, the render thread is rendering into
    // _colorBuffer and _depthBuffer.  _converged is set to their convergence
    // state just before blit, so use that as our answer.
    if (m_aovBindings.empty()) {
        return m_converged;
    }

    // Otherwise, check the convergence of all attachments.
    for (const auto & m_aovBinding : m_aovBindings) {
        if (m_aovBinding.renderBuffer &&
            !m_aovBinding.renderBuffer->IsConverged()) {
            return false;
        }
    }
    return true;
}

//-------------------------------------------------------------------------
// Rendering Operations
//-------------------------------------------------------------------------

static GfRect2i _GetDataWindow(HdRenderPassStateSharedPtr const& _renderPassState)
{
    if (const CameraUtilFraming &framing = _renderPassState->GetFraming(); framing.IsValid()) {
        return framing.dataWindow;
    }
    // For applications that use the old viewport API instead of
    // the new camera framing API.
    const GfVec4f vp = _renderPassState->GetViewport();
    return {GfRect2i(GfVec2i(0), static_cast<int>(vp[2]), static_cast<int>(vp[3]))};
}

void HdGraviRenderPass::_Execute(HdRenderPassStateSharedPtr const& _renderPassState, TfTokenVector const &_renderTags)
{
    // Determine whether the scene has changed since the last time we rendered.
    bool needStartRender = false;
    bool needUpdateLights = false;
    if (const int currentSceneVersion = m_sceneVersion->load(); m_lastSceneVersion != currentSceneVersion) {
        m_renderer->BuildBVH(); // We also setup the lights in here.
        needStartRender = true;
        needUpdateLights = true;
        m_lastSceneVersion = currentSceneVersion;
    }
    // Process lights
    if (needUpdateLights) {
        m_renderer->UpdateLights();
        needUpdateLights = false;
    }

    // Determine whether we need to update the renderer camera.
    const GfMatrix4d view = _renderPassState->GetWorldToViewMatrix();
    if (const GfMatrix4d proj = _renderPassState->GetProjectionMatrix(); m_viewMatrix != view || m_projMatrix != proj) {
        m_viewMatrix = view;
        m_projMatrix = proj;

        m_renderThread->StopRender();
        m_renderer->SetCamera(m_viewMatrix, m_projMatrix);
        needStartRender = true;
    }

    // Check if the window has been resized
    if (const GfRect2i dataWindow = _GetDataWindow(_renderPassState); m_dataWindow != dataWindow) {
        m_dataWindow = dataWindow;

        m_renderThread->StopRender();
        m_renderer->SetDataWindow(dataWindow);

        if (!_renderPassState->GetFraming().IsValid()) {
            // Support clients that do not use the new framing API
            // and do not use AOVs.
            //
            // Note that we do not support the case of using the
            // new camera framing API without using AOVs.
            //
            const GfVec3i dimensions(m_dataWindow.GetWidth(),
                                     m_dataWindow.GetHeight(),
                                     1);

            m_colorBuffer.Allocate(
                dimensions,
                HdFormatUNorm8Vec4,
                /*multiSampled=*/false);

            m_depthBuffer.Allocate(
                dimensions,
                HdFormatFloat32,
                /*multiSampled=*/false);
        }

        needStartRender = true;
    }

    // Determine whether we need to update the renderer AOV bindings.
    //
    // It's possible for the passed in bindings to be empty, but that's
    // never a legal state for the renderer, so if that's the case we add
    // a color and depth aov.
    //
    // If the renderer AOV bindings are empty, force a bindings update so that
    // we always get a chance to add color/depth on the first time through.
    HdRenderPassAovBindingVector aovBindings =
        _renderPassState->GetAovBindings();
    if (m_aovBindings != aovBindings || m_renderer->GetAovBindings().empty()) {
        m_aovBindings = aovBindings;

        m_renderThread->StopRender();
        if (aovBindings.empty()) {
            HdRenderPassAovBinding colorAov;
            colorAov.aovName = HdAovTokens->color;
            colorAov.renderBuffer = &m_colorBuffer;
            colorAov.clearValue = VtValue(GfVec4d(0, 0, 0, 1));
            aovBindings.push_back(colorAov);

            HdRenderPassAovBinding depthAov;
            depthAov.aovName = HdAovTokens->depth;
            depthAov.renderBuffer = &m_depthBuffer;
            depthAov.clearValue = VtValue(1.0f);
            aovBindings.push_back(depthAov);
        }
        m_renderer->SetAovBindings(aovBindings);
        // In general, the render thread clears aov bindings, but make sure
        // they are cleared initially on this thread.
        m_renderer->Clear();
        needStartRender = true;
    }

    TF_VERIFY(!m_aovBindings.empty(), "No aov bindings to render into");

    // Only start a new render if something in the scene has changed.
    if (needStartRender) {
        m_converged = false;
        m_renderer->MarkAovBuffersUnconverged();
        m_renderThread->StartRender();
        auto lock = m_renderThread->LockFramebuffer();
    }
}

PXR_NAMESPACE_CLOSE_SCOPE