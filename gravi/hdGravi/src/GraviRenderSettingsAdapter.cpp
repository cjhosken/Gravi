///
/// @file GraviAdapter.cpp
/// @brief How the renderer recognizes GravityWells.


#include <pxr/usd/usdGeom/subset.h>
#include <pxr/usdImaging/usdImaging/indexProxy.h>
#include <pxr/usdImaging/usdImaging/tokens.h>

#include "RenderSettings.h"
#include "graviRenderSettings.h"

#include "GraviRenderSettingsAdapter.h"

PXR_NAMESPACE_OPEN_SCOPE

static TfToken graviToken("GraviRenderSettings");

TF_REGISTRY_FUNCTION(TfType)
{
    typedef HdGraviRenderSettingsAdapter Adapter;
    TfType t = TfType::Define<Adapter, TfType::Bases<Adapter::BaseAdapter> >();
    t.SetFactory< UsdImagingPrimAdapterFactory<Adapter> >();
}

bool HdGraviRenderSettingsAdapter::IsSupported(UsdImagingIndexProxy const* _index) const { return _index->IsBprimTypeSupported(graviToken); }

VtValue HdGraviRenderSettingsAdapter::Get(UsdPrim const& _prim, SdfPath const& _cachePath, TfToken const& _key, const UsdTimeCode _time, VtIntArray *_outIndices) const
{
    TRACE_FUNCTION();
    HF_MALLOC_TAG_FUNCTION();
    const GraviRenderSettings settings(_prim);

    if (_key == HdGraviRenderSettingsTokens->bounces) {
        VtValue bounces;
        settings.GetBouncesAttr().Get(&bounces);
        return bounces;
    }

    if (_key == HdRenderSettingsTokens->convergedSamplesPerPixel) {
        VtValue samples;
        settings.GetConvergedSamplesPerPixelAttr().Get(&samples);
        return samples;
    }

    if (_key == HdGraviRenderSettingsTokens->lightStepSize) {
        VtValue size;
        settings.GetLightStepSizeAttr().Get(&size);
        return size;
    }

    if (_key == HdGraviRenderSettingsTokens->minLightStepSize) {
        VtValue size;
        settings.GetMinLightStepSizeAttr().Get(&size);
        return size;
    }

    if (_key == HdGraviRenderSettingsTokens->maxLightStepSize) {
        VtValue size;
        settings.GetMaxLightStepSizeAttr().Get(&size);
        return size;
    }

    if (_key == HdGraviRenderSettingsTokens->maxLightDistance) {
        VtValue dist;
        settings.GetMaxLightDistanceAttr().Get(&dist);
        return dist;
    }

    if (_key == HdGraviRenderSettingsTokens->maxLightSteps) {
        VtValue steps;
        settings.GetMaxLightStepsAttr().Get(&steps);
        return steps;
    }

    return UsdImagingRenderSettingsAdapter::Get(_prim, _cachePath, _key, _time, _outIndices);
}

SdfPath HdGraviRenderSettingsAdapter::Populate(UsdPrim const& _prim, UsdImagingIndexProxy* _index, UsdImagingInstancerContext const* _instancerContext)
{
    SdfPath const rsPrimPath = _prim.GetPath();
    _index->InsertBprim(graviToken, rsPrimPath, _prim);
    return _prim.GetPath();
}

PXR_NAMESPACE_CLOSE_SCOPE
