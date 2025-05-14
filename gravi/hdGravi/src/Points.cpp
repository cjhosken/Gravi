///
/// @file Points.cpp
/// @brief Point support for USD scene data.

#include <pxr/base/gf/ray.h>

#include "HdGraviPrim.h"

#include "Points.h"


PXR_NAMESPACE_OPEN_SCOPE

//-------------------------------------------------------------------------
// Construction / Destruction
//-------------------------------------------------------------------------

HdGraviPoints::HdGraviPoints(SdfPath const &_id) : HdPoints(_id), HdGraviPrim(this) {}

//-------------------------------------------------------------------------
// Hydra Points Interface
//-------------------------------------------------------------------------

HdDirtyBits HdGraviPoints::GetInitialDirtyBitsMask() const
{
    int mask = HdChangeTracker::Clean
        | HdChangeTracker::DirtyPoints
        | HdChangeTracker::DirtyPrimID
        | HdChangeTracker::DirtyPrimvar
        | HdChangeTracker::DirtyMaterialId
        | HdChangeTracker::DirtyTransform
        | HdChangeTracker::DirtyVisibility
        | HdChangeTracker::DirtyWidths
        | HdChangeTracker::DirtyDoubleSided
        | HdChangeTracker::DirtyInstancer
        | HdChangeTracker::DirtyInstanceIndex
        | HdChangeTracker::DirtyCategories;
    return (HdDirtyBits)mask;
}

void HdGraviPoints::Sync(HdSceneDelegate *_sceneDelegate,
             HdRenderParam   *_renderParam,
             HdDirtyBits     *_dirtyBits,
             TfToken const   &_reprToken)
{
    _UpdateVisibility(_sceneDelegate, _dirtyBits);
    _UpdateInstancer(_sceneDelegate, _dirtyBits);
    SyncAll(_sceneDelegate, *_renderParam, _dirtyBits, _reprToken);
}

//-------------------------------------------------------------------------
// Primvar Management
//-------------------------------------------------------------------------

void HdGraviPoints::SyncPrimvar(HdSceneDelegate *_sceneDelegate, HdRenderParam &_renderParam, const TfToken &_name, const VtValue &_value, const HdInterpolation &_interp, const TfToken &_role)
{
    if (_name == HdTokens->widths) {
        if (_value.IsEmpty()) {
            m_widths = {0.1}; // set default value
        } else if (_value.IsHolding<VtFloatArray>()) {
            m_widths = _value.UncheckedGet<VtFloatArray>();
        }
        return;
    }

    HdGraviPrim::SyncPrimvar(_sceneDelegate, _renderParam, _name, _value, _interp, _role);
}

//-------------------------------------------------------------------------
// Rendering Operations
//-------------------------------------------------------------------------

void HdGraviPoints::CreateBVH()
{
    m_bvh = GfBBox3d();
    float maxRadius = 0.0f;

    if (!m_widths.empty()) {
        maxRadius = (m_widths.size() == m_points.size())
                ? *std::max_element(m_widths.begin(), m_widths.end()) * 0.05f
                : m_widths[0] * 0.05f;
    }

    if (!m_points.empty())
    {
        GfRange3d range;

        for (size_t i = 0; i < m_points.size(); ++i)
        {
            GfVec3d transformedPoint = m_transform.Transform(GfVec3d(m_points[i]));

            GfVec3d radiusVec(maxRadius, maxRadius, maxRadius);
            range.UnionWith(GfRange3d(
                transformedPoint - radiusVec,
                transformedPoint + radiusVec
                ));
        }

        m_bvh.SetRange(range);
    }
}

IntersectData HdGraviPoints::Intersect(const GfRay &_ray) const
{
    double closestT = std::numeric_limits<double>::infinity();

    GfVec3f Cd(1.0f);
    GfVec3f N(0.0f);

    bool useGlobalWidth = true;
    bool useGlobalColors = true;

    if (m_widths.size() == m_points.size()) {
        useGlobalWidth = false;
    }

    if (m_displayColors.size() > 1) {
        useGlobalColors = false;
    }

    // Start BVH traversal
    for (size_t i = 0; i < m_points.size(); ++i) {
            GfVec3d pnt = m_points[i];
            double t;
            const float radius = (useGlobalWidth ? m_widths[0] : m_widths[i]) * 0.05f;

            if (const bool hit = _ray.Intersect(pnt,radius, &t); hit && t > 0.0001f && t < closestT) {
                Cd =(useGlobalColors ? m_displayColors[0] : m_displayColors[i]);
                closestT = t;
                N = GfVec3f((_ray.GetPoint(closestT) - pnt).GetNormalized());
            }
        }

    closestT = closestT == std::numeric_limits<double>::infinity() ? -1.0 : closestT;

    GfVec4f CdOut(Cd[0], Cd[1], Cd[2], 1.0f);


    return {static_cast<float>(closestT), N, CdOut, static_cast<int32_t>(GetId().GetHash()), this};
}

//-------------------------------------------------------------------------
// Hydra Internal Interface
//-------------------------------------------------------------------------

HdDirtyBits HdGraviPoints::_PropagateDirtyBits(const HdDirtyBits _bits) const { return _bits; }

PXR_NAMESPACE_CLOSE_SCOPE