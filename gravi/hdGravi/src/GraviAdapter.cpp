///
/// @file GraviAdapter.cpp
/// @brief How the renderer recognizes GravityWells.

#include <pxr/usd/usdGeom/subset.h>
#include <pxr/usdImaging/usdImaging/indexProxy.h>
#include <pxr/usdImaging/usdImaging/tokens.h>

#include "gravityWell.h"

#include "GraviAdapter.h"

PXR_NAMESPACE_OPEN_SCOPE

static TfToken graviToken("GravityWell");

TF_REGISTRY_FUNCTION(TfType)
{
    typedef HdGraviAdapter Adapter;
    TfType t = TfType::Define<Adapter, TfType::Bases<Adapter::BaseAdapter> >();
    t.SetFactory< UsdImagingPrimAdapterFactory<Adapter> >();
}

//-------------------------------------------------------------------------
// Primitive Interface
//-------------------------------------------------------------------------

VtValue HdGraviAdapter::Get(UsdPrim const& _prim, SdfPath const& _cachePath, TfToken const& _key, const UsdTimeCode _time, VtIntArray *_outIndices) const
{
    TRACE_FUNCTION();
    HF_MALLOC_TAG_FUNCTION();
    const GravityWell well(_prim);

    if (_key == TfToken("force")) {
        VtValue force;
        well.GetForceAttr().Get(&force);
        return force;
    }

    return UsdImagingGprimAdapter::Get(_prim, _cachePath, _key, _time, _outIndices);
}

bool HdGraviAdapter::IsSupported(UsdImagingIndexProxy const* _index) const { return _index->IsRprimTypeSupported(graviToken); }

SdfPath HdGraviAdapter::Populate(UsdPrim const& _prim, UsdImagingIndexProxy* _index, UsdImagingInstancerContext const* _instancerContext)
{
    return _AddRprim(graviToken,
        _prim,
        _index,
        GetMaterialUsdPath(_prim),
        _instancerContext
    );
}

PXR_NAMESPACE_CLOSE_SCOPE
