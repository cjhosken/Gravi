///
/// @file HdGraviPrim.cpp
/// @brief Curve support for USD scene data.

#include <pxr/imaging/hd/rprim.h>

#include "HdGraviPrim.h"


PXR_NAMESPACE_OPEN_SCOPE

//-------------------------------------------------------------------------
// Construction / Destruction
//-------------------------------------------------------------------------

HdGraviPrim::HdGraviPrim(HdRprim* _rprim) : m_rprim(*_rprim) {}

//-------------------------------------------------------------------------
// Synchronization Interface
//-------------------------------------------------------------------------

void HdGraviPrim::SyncAll(HdSceneDelegate *_sceneDelegate,
                       HdRenderParam& _renderParam,
                       HdDirtyBits *_dirtyBits,
                       TfToken const &_reprToken)
{
    // primvars may override other means of setting attributes
    // we handle this by syncing primvars first...
    SyncPrimvars(_sceneDelegate, _renderParam, _dirtyBits);

    // and then checking in the following function if a primvar
    // is set (function isPrimvarUsed)
    SyncAttributes(_sceneDelegate, _renderParam, _dirtyBits, _reprToken, false);

    // perform material and light assignment, and instancing
    Assign(_sceneDelegate, _renderParam, _dirtyBits);

    // clear dirty bits to indicate everything is synced
    *_dirtyBits &= ~HdChangeTracker::AllSceneDirtyBits;
}

//-------------------------------------------------------------------------
// Rendering Interface
//-------------------------------------------------------------------------

IntersectData HdGraviPrim::Intersect(const GfRay &_ray) const {return {};}

GfBBox3d HdGraviPrim::GetBVH() const { return m_bvh; }

bool HdGraviPrim::IsVolume() const { return false; }

GfMatrix4d HdGraviPrim::GetTransform() const { return m_transform; }

VtMatrix4dArray HdGraviPrim::GetInstanceTransforms() const { return m_instanceTransforms; }

bool HdGraviPrim::HasInstances() const { return m_instanced; }

void HdGraviPrim::SetTransform(const GfMatrix4d &_transform) { m_transform = _transform; }

//-------------------------------------------------------------------------
// Protected Virtual Methods
//-------------------------------------------------------------------------

void HdGraviPrim::SyncAttributes(HdSceneDelegate *_sceneDelegate, HdRenderParam &_renderParam, const HdDirtyBits *_dirtyBits, TfToken const &_reprToken, bool _updateBVH=false)
{
    // sync plain attributes. Should be called after syncPrimvars, since
    // it uses the list mAppliedPrimvars to avoid overwriting any primvar overrides
    // mGeometry must be non-null and have an active UpdateGuard.
    // This function should be overridden to handle additional attributes in a
    // subclass.

    const SdfPath& id = m_rprim.GetId();

    // Get the transform of the primitive
    if (HdChangeTracker::IsTransformDirty(*_dirtyBits, id)) {
        m_transform = _sceneDelegate->GetTransform(id);
        _updateBVH = true;
    }

    // Check if the primitive is visible.
    if (HdChangeTracker::IsTopologyDirty(*_dirtyBits, id)) {
        m_visible = _sceneDelegate->GetVisible(id);
    }

    // Check if the primitive is double sided.
    if (HdChangeTracker::IsDoubleSidedDirty(*_dirtyBits, id)) {
        m_doubleSided = false;
        if (_sceneDelegate->GetDoubleSided(id)) {
            m_doubleSided = true;
        }
    }

    // Since a BVH structure is being used, the bounding box needs to be obtained or created.
    if (_updateBVH) {
        VtValue value = _sceneDelegate->Get(id, HdTokens->bbox);
        if (value.IsHolding<GfBBox3d>())
        {
            m_bvh = value.Get<GfBBox3d>();
        }
        else
        {
            CreateBVH();
        }
    }
}

void HdGraviPrim::SyncPrimvar(HdSceneDelegate *_sceneDelegate, HdRenderParam &_renderParam, const TfToken &_name, const VtValue &_value, const HdInterpolation &_interp, const TfToken &_role)
{
    if (_name == HdTokens->points) {
        if (!_value.IsEmpty()) {
            m_points = _value.Get<VtVec3fArray>();
        }
        return;
    }
    if (_name == HdTokens->velocities) {
        if (!_value.IsEmpty()) {
            m_velocities = _value.Get<VtVec3fArray>();
        }
        return;
    }
    if (_name == HdTokens->displayColor) {
        if (!_value.IsEmpty()) {
            m_displayColors = _value.Get<VtVec3fArray>();
        }
        return;
    }
}

void HdGraviPrim::CreateBVH() { m_bvh = GfBBox3d(); }

//-------------------------------------------------------------------------
// Private Implementation
//-------------------------------------------------------------------------

void HdGraviPrim::Assign(HdSceneDelegate *_sceneDelegate, HdRenderParam &_renderParam, const HdDirtyBits *_dirtyBits) const
{

    // perform material and light assignment and instance creation

}

void HdGraviPrim::SyncPrimvars(HdSceneDelegate *_sceneDelegate, HdRenderParam &_renderParam, const HdDirtyBits *_dirtyBits)
{
    const SdfPath& id = m_rprim.GetId();

    if (HdChangeTracker::IsPrimIdDirty(*_dirtyBits, id)) {
        static TfToken primId("primId");
        SyncPrimvar(_sceneDelegate, _renderParam, primId, VtValue(static_cast<float>(m_rprim.GetPrimId())),
        HdInterpolation::HdInterpolationConstant,
        TfToken());
    }

    // We will iterate through every primvar, calling primvarChanged if the primvar
    // is dirty. To detect removed primvars, we maintain the list m_appliedPrimvars of
    // all currently applied primvars, and build a list 'removedPrimvars' by comparing
    // the new and old lists.

    std::set<TfToken> removedPrimvars;
    std::swap(m_primvars, removedPrimvars);
    m_primvars.clear();

    // initially, removedPrimvars has the old list of applied primvars : as we
    // see current primvars, we will erase them from the removed list

    // process external computations first, so that they have priority.
    // Values for these are fetched in a single call, so we need two loops
    // process external computations first, so that they have priority.
    // Values for these are fetched in a single call, so we need two loops
    HdExtComputationPrimvarDescriptorVector dirtyCompPrimvars;
    for (size_t i = 0; i < HdInterpolationCount; ++i) {
        HdInterpolation interp = static_cast<HdInterpolation>(i);
        HdExtComputationPrimvarDescriptorVector compPrimvars =
                _sceneDelegate->GetExtComputationPrimvarDescriptors(id, interp);
        for (auto const& pv: compPrimvars) {
            m_primvars.insert(pv.name);
            removedPrimvars.erase(pv.name);
            if (HdChangeTracker::IsPrimvarDirty(*_dirtyBits, id, pv.name)) {
                dirtyCompPrimvars.emplace_back(pv);
            }
        }
    }

    // actually compute and update the dirty primvars we discovered
    if (not dirtyCompPrimvars.empty()) {
        HdExtComputationUtils::ValueStore valueStore =
            HdExtComputationUtils::GetComputedPrimvarValues(dirtyCompPrimvars, _sceneDelegate);
        for (auto const& compPrimvar : dirtyCompPrimvars) {
            auto const computedValue = valueStore.find(compPrimvar.name);
            SyncPrimvar(_sceneDelegate,
                           _renderParam,
                           compPrimvar.name,
                           computedValue->second,
                           compPrimvar.interpolation,
                           compPrimvar.role);
        }
    }

    // now process regular primvars, skipping any already seen as computed primvars
    for (size_t i = 0; i < HdInterpolationCount; ++i) {
        HdInterpolation interp = static_cast<HdInterpolation>(i);
        for (HdPrimvarDescriptor const& pv : m_rprim.GetPrimvarDescriptors(_sceneDelegate, interp)) {
                m_primvars.insert(pv.name);
                removedPrimvars.erase(pv.name);
                if (HdChangeTracker::IsPrimvarDirty(*_dirtyBits, id, pv.name)) {
                    SyncPrimvar(_sceneDelegate,
                                   _renderParam,
                                   pv.name,
                                   m_rprim.GetPrimvar(_sceneDelegate, pv.name),
                                   interp, pv.role);
                }
        }
    }

    // finally, process the removed primvars left behind. We will call primvarChanged
    // with an empty VtValue
    for (auto&& name : removedPrimvars) {
        SyncPrimvar(_sceneDelegate, _renderParam, name, VtValue(),
                       HdInterpolation::HdInterpolationConstant, TfToken());
    }
}


PXR_NAMESPACE_CLOSE_SCOPE