///
/// @file RenderDelegate.cpp
/// @brief The Gravi render delegate, this is fed into the plugin and is the main access point to the renderer.

#include <memory>
#include <pxr/imaging/hd/extComputation.h>
#include <pxr/imaging/hd/resourceRegistry.h>
#include <pxr/imaging/hd/tokens.h>
#include <pxr/imaging/hd/camera.h>
#include <pxr/imaging/hd/bprim.h>
#include <pxr/imaging/hd/renderSettings.h>

#include "RenderParam.h"
#include "RenderPass.h"
#include "Mesh.h"
#include "RenderSettings.h"
#include "BasisCurves.h"
#include "HdGravityWell.h"
#include "Points.h"
#include "Light.h"

#include "RenderDelegate.h"


PXR_NAMESPACE_OPEN_SCOPE

//-------------------------------------------------------------------------
// Construction / Destruction
//-------------------------------------------------------------------------

const TfTokenVector HdGraviRenderDelegate::s_SUPPORTED_RPRIM_TYPES = {
        HdPrimTypeTokens->mesh,
        HdPrimTypeTokens->basisCurves,
        HdPrimTypeTokens->points,
        //HdPrimTypeTokens->volume,
        //HdPrimTypeTokens->cone,
        //HdPrimTypeTokens->cylinder,
        //HdPrimTypeTokens->sphere,
        //HdPrimTypeTokens->cube,
        TfToken("GravityWell")
};

const TfTokenVector HdGraviRenderDelegate::s_SUPPORTED_SPRIM_TYPES = {
        HdPrimTypeTokens->camera,
        HdPrimTypeTokens->light,
        HdPrimTypeTokens->cylinderLight,
        HdPrimTypeTokens->diskLight,
        HdPrimTypeTokens->distantLight,
        HdPrimTypeTokens->domeLight,
        HdPrimTypeTokens->rectLight,
        HdPrimTypeTokens->sphereLight,
        //HdPrimTypeTokens->material,
        HdPrimTypeTokens->extComputation,
};

const TfTokenVector HdGraviRenderDelegate::s_SUPPORTED_BPRIM_TYPES = {
        HdPrimTypeTokens->renderBuffer,
        TfToken("GraviRenderSettings"),
        TfToken("openvdbAsset")
};

std::mutex HdGraviRenderDelegate::s_mutexResourceRegistry;
std::atomic_int HdGraviRenderDelegate::s_counterResourceRegistry;
HdResourceRegistrySharedPtr HdGraviRenderDelegate::s_resourceRegistry;

static void RenderCallback(HdGraviRenderer *_renderer, HdRenderThread *_renderThread)
{
    _renderer->Clear();
    _renderer->Render(_renderThread);
}

HdGraviRenderDelegate::HdGraviRenderDelegate() { _Initialize(); }

HdGraviRenderDelegate::HdGraviRenderDelegate(HdRenderSettingsMap const &_settingsMap)
    : HdRenderDelegate(_settingsMap) { _Initialize(); }

HdGraviRenderDelegate::~HdGraviRenderDelegate()
{
    // Clean the resource registry only when it is the last Gravi delegate
    std::lock_guard guard(s_mutexResourceRegistry);
    if (s_counterResourceRegistry.fetch_sub(1) == 1) {
        s_resourceRegistry.reset();
    }

    m_renderThread.StopThread();

    // Destroy Gravi library and scene state.
    m_renderParam.reset();
}

//-------------------------------------------------------------------------
// Primitive Support Queries
//-------------------------------------------------------------------------

TfTokenVector const & HdGraviRenderDelegate::GetSupportedRprimTypes() const { return s_SUPPORTED_RPRIM_TYPES; }

TfTokenVector const & HdGraviRenderDelegate::GetSupportedSprimTypes() const { return s_SUPPORTED_SPRIM_TYPES; }

TfTokenVector const & HdGraviRenderDelegate::GetSupportedBprimTypes() const { return s_SUPPORTED_BPRIM_TYPES; }

//-------------------------------------------------------------------------
// Resource Management
//-------------------------------------------------------------------------

HdRenderParam* HdGraviRenderDelegate::GetRenderParam() const { return m_renderParam.get(); }

HdResourceRegistrySharedPtr HdGraviRenderDelegate::GetResourceRegistry() const { return s_resourceRegistry; }

//-------------------------------------------------------------------------
// Render Settings
//-------------------------------------------------------------------------

HdRenderSettingDescriptorList HdGraviRenderDelegate::GetRenderSettingDescriptors() const { return m_settingDescriptors; }

//-------------------------------------------------------------------------
// Rendering Control
//-------------------------------------------------------------------------

bool HdGraviRenderDelegate::IsPauseSupported() const { return true; }

HdGraviRenderer* HdGraviRenderDelegate::GetRenderer() const { return m_renderer; }

bool HdGraviRenderDelegate::Pause()
{
    m_renderThread.PauseRender();
    return true;
}

bool HdGraviRenderDelegate::Resume()
{
    m_renderThread.ResumeRender();
    return true;
}

//-------------------------------------------------------------------------
// Primitive Management
//-------------------------------------------------------------------------

HdRenderPassSharedPtr HdGraviRenderDelegate::CreateRenderPass(HdRenderIndex *_index, HdRprimCollection const &_collection) { return std::make_shared<HdGraviRenderPass>(_index, _collection, &m_renderThread, m_renderer, &m_sceneVersion); }

HdInstancer* HdGraviRenderDelegate::CreateInstancer(HdSceneDelegate *_delegate, SdfPath const &_id) { return nullptr; }

void HdGraviRenderDelegate::DestroyInstancer(HdInstancer *_instancer) { }

HdRprim* HdGraviRenderDelegate::CreateRprim(TfToken const &_typeId, SdfPath const &_rprimId)
{
    if (_typeId == HdPrimTypeTokens->mesh)
    {
        return new HdGraviMesh(_rprimId);
    }
    if (_typeId == HdPrimTypeTokens->points) {
        return new HdGraviPoints(_rprimId);
    }
    if (_typeId == HdPrimTypeTokens->basisCurves)
    {
        return new HdGraviBasisCurves(_rprimId);
    }
    if (_typeId == TfToken("GravityWell")) {
        return new HdGravityWell(_rprimId);
    }

    TF_CODING_ERROR("Unknown Rprim Type %s", _typeId.GetText());
    return nullptr;
}

void HdGraviRenderDelegate::DestroyRprim(HdRprim *_rPrim) { delete _rPrim; }

HdSprim* HdGraviRenderDelegate::CreateSprim(TfToken const &_typeId, SdfPath const &_sprimId)
{
    if (_typeId == HdPrimTypeTokens->camera)
    {
        return new HdCamera(_sprimId);
    }
    if (_typeId == HdPrimTypeTokens->light ||
        _typeId == HdPrimTypeTokens->distantLight ||
        _typeId == HdPrimTypeTokens->domeLight ||
        _typeId == HdPrimTypeTokens->rectLight ||
        _typeId == HdPrimTypeTokens->diskLight ||
        _typeId == HdPrimTypeTokens->cylinderLight ||
        _typeId == HdPrimTypeTokens->sphereLight) {
        auto light = new HdGraviLight(_sprimId, _typeId);
        if (_sprimId.GetString().find("/_UsdImaging") != 0 && !_sprimId.GetString().empty()) {
            m_renderer->AddLight(light);
        }
        return light;
    }
    if (_typeId == HdPrimTypeTokens->extComputation)
    {
        return new HdExtComputation(_sprimId);
    }

    TF_CODING_ERROR("Unknown Sprim Type %s", _typeId.GetText());
    return nullptr;
}

HdSprim* HdGraviRenderDelegate::CreateFallbackSprim(TfToken const &_typeId) { return CreateSprim(_typeId, SdfPath::EmptyPath()); }

void HdGraviRenderDelegate::DestroySprim(HdSprim *_sPrim) { delete _sPrim; }

HdBprim* HdGraviRenderDelegate::CreateBprim(TfToken const &_typeId, SdfPath const &_bprimId)
{
    if (_typeId == HdPrimTypeTokens->renderBuffer)
    {
        return new HdGraviRenderBuffer(_bprimId);
    }
    if (_typeId == TfToken("GraviRenderSettings")) {
        return new HdGraviRenderSettings(_bprimId);
    }

    TF_CODING_ERROR("Unknown Bprim Type %s", _typeId.GetText());
    return nullptr;
}

HdBprim* HdGraviRenderDelegate::CreateFallbackBprim(TfToken const &_typeId) { return CreateBprim(_typeId, SdfPath::EmptyPath()); }

void HdGraviRenderDelegate::DestroyBprim(HdBprim *_bPrim) { delete _bPrim; }

//-------------------------------------------------------------------------
// Rendering Operations
//-------------------------------------------------------------------------

TfToken HdGraviRenderDelegate::GetMaterialBindingPurpose() const { return HdTokens->full; }

HdAovDescriptor HdGraviRenderDelegate::GetDefaultAovDescriptor(TfToken const &_name) const
{
    // Here we create a bunch of AOV Descriptors based on what HdAovTokens we want in our renderer. (This will make them selectable in the usdview)
    if (_name == HdAovTokens->color)
    {
        return {HdFormatFloat32Vec4, true,VtValue(GfVec4f(0.0f, 0.0f, 0.0f, 1.0f))};
    }
    if (_name == HdAovTokens->normal || _name == HdAovTokens->Neye)
    {
        return {HdFormatFloat32Vec3, false,VtValue(GfVec3f(0.0f))};
    }
    if (_name == HdAovTokens->depth)
    {
        return {HdFormatFloat32, false,VtValue(1.0f)};
    }
    if (_name == HdAovTokens->cameraDepth)
    {
        return {HdFormatFloat32, false,VtValue(0.0f)};
    }
    if (_name == HdAovTokens->Peye)
    {
        return {HdFormatFloat32Vec3, false,VtValue(GfVec3f(0.0f))};
    }
    if (_name == HdAovTokens->primId || _name == HdAovTokens->instanceId || _name == HdAovTokens->elementId)
    {
        return {HdFormatInt32, false,VtValue(-1)};
    }
    if (const HdParsedAovToken aovId(_name); aovId.isPrimvar) {
        return {HdFormatFloat32Vec3, false, VtValue(GfVec3f(0.0f))};
    }
    return {};
}

VtDictionary HdGraviRenderDelegate::GetRenderStats() const
{
    VtDictionary stats;
    stats[HdPerfTokens->numCompletedSamples.GetString()] = VtValue(m_renderer->GetCompletedSamples());
    return stats;
}

//-------------------------------------------------------------------------
// Private Implementation
//-------------------------------------------------------------------------

void HdGraviRenderDelegate::_Initialize()
{
    m_settingDescriptors.clear();

    // Bounces
    m_settingDescriptors.push_back(HdRenderSettingDescriptor{
        TfToken("Samples per pixel"),
        HdRenderSettingsTokens->convergedSamplesPerPixel,
        VtValue(defaultConvergedSamplesPerPixel),
    });

    // Samples
    m_settingDescriptors.push_back(HdRenderSettingDescriptor{
        TfToken("Bounces"),
        HdGraviRenderSettingsTokens->bounces,
        VtValue(pxr::defaultBounces),

    });

    // Output
    m_settingDescriptors.push_back(HdRenderSettingDescriptor{
        HdGraviRenderSettingsTokens->outputPath,
        TfToken("Output file path"),
        VtValue(defaultOutputPath)
    });

    _PopulateDefaultSettings(m_settingDescriptors);

    // Store top-level Gravi objects inside a render param that can be
    // passed to prims during Sync(). Also pass a handle to the render thread.
    m_renderParam = std::make_shared<HdGraviRenderParam>(
        &m_renderThread, nullptr, &m_sceneVersion);

    // Initialize the Gravi Renderer
    m_renderer = new HdGraviRenderer();

    // Set the background render thread's rendering entrypoint to
    // HdGraviRenderer::Render.
    m_renderThread.SetRenderCallback(
        [capture0 = m_renderer, capture1 = &m_renderThread] { RenderCallback(capture0, capture1); });
    m_renderThread.StartThread();

    // Initialize one resource registry for all Gravi plugins
    std::lock_guard guard(s_mutexResourceRegistry);

    if (s_counterResourceRegistry.fetch_add(1) == 0) {
        s_resourceRegistry = std::make_shared<HdResourceRegistry>();
    }
}

PXR_NAMESPACE_CLOSE_SCOPE