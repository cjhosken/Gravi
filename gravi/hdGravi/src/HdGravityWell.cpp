///
/// @file Points.cpp
/// @brief Point support for USD scene data.

#include <pxr/base/gf/ray.h>

#include "HdGraviPrim.h"

#include "HdGravityWell.h"


PXR_NAMESPACE_OPEN_SCOPE

//-------------------------------------------------------------------------
// Construction / Destruction
//-------------------------------------------------------------------------

HdGravityWell::HdGravityWell(SdfPath const &_id)
    : HdRprim(_id)
    , HdGraviPrim(this)
    , m_force(0)
    {}

//-------------------------------------------------------------------------
// Hydra Primitive Interface
//-------------------------------------------------------------------------

TfTokenVector const & HdGravityWell::GetBuiltinPrimvarNames() const
{
    static const TfTokenVector primvarNames = {
        TfToken("force")
    };
    return primvarNames;
}

HdDirtyBits HdGravityWell::GetInitialDirtyBitsMask() const { return HdChangeTracker::AllDirty; }

void HdGravityWell::Sync(HdSceneDelegate *_sceneDelegate,
             HdRenderParam   *_renderParam,
             HdDirtyBits     *_dirtyBits,
             TfToken const   &_reprToken)
{
    _UpdateVisibility(_sceneDelegate, _dirtyBits);
    _UpdateInstancer(_sceneDelegate, _dirtyBits);
    SyncAll(_sceneDelegate, *_renderParam, _dirtyBits, _reprToken);
}

//-------------------------------------------------------------------------
// Attribute Synchronization
//-------------------------------------------------------------------------

void HdGravityWell::SyncAttributes(HdSceneDelegate *_sceneDelegate, HdRenderParam &_renderParam, const HdDirtyBits* _dirtyBits, TfToken const &_reprToken, const bool _updateBVH)
{
    SdfPath const& id = GetId();

    if (*_dirtyBits) {
        VtValue forceValue = _sceneDelegate->Get(id, TfToken("force"));
        if (forceValue.IsHolding<float>()) {
            m_force = forceValue.Get<float>();
        }
    }

    HdGraviPrim::SyncAttributes(_sceneDelegate, _renderParam, _dirtyBits, _reprToken, _updateBVH);
}

void HdGravityWell::SyncPrimvar(HdSceneDelegate *_sceneDelegate, HdRenderParam &_renderParam, const TfToken &_name, const VtValue &_value, const HdInterpolation &_interp, const TfToken &_role)
{
    HdGraviPrim::SyncPrimvar(_sceneDelegate, _renderParam, _name, _value, _interp, _role);
}

//-------------------------------------------------------------------------
// Rendering Operations
//-------------------------------------------------------------------------

void HdGravityWell::CreateBVH() { m_bvh = GfBBox3d(); }

IntersectData HdGravityWell::Intersect(const GfRay &_ray) const
{
    double closestT = std::numeric_limits<double>::infinity();

    GfVec4f Cd(0.0f, 0.0f, 0.0f, 1.0f);
    GfVec3f N(0.0f);

    return {static_cast<float>(closestT), N, Cd, static_cast<int32_t>(GetId().GetHash()), this};
}

float HdGravityWell::GetForce() const { return m_force; }


//-------------------------------------------------------------------------
// Hydra Internal Interface
//-------------------------------------------------------------------------

HdDirtyBits HdGravityWell::_PropagateDirtyBits(const HdDirtyBits _bits) const { return _bits; }


PXR_NAMESPACE_CLOSE_SCOPE