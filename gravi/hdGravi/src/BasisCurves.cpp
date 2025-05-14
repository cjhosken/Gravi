///
/// @file BasisCurves.cpp
/// @brief Curve support for USD scene data.

#include <pxr/base/gf/ray.h>

#include "HdGraviPrim.h"

#include "BasisCurves.h"

PXR_NAMESPACE_OPEN_SCOPE

//-------------------------------------------------------------------------
// Construction / Destruction
//-------------------------------------------------------------------------

HdGraviBasisCurves::HdGraviBasisCurves(SdfPath const &_id) : HdBasisCurves(_id), HdGraviPrim(this) {}

//-------------------------------------------------------------------------
// Hydra Primitive Interface
//-------------------------------------------------------------------------

HdDirtyBits HdGraviBasisCurves::GetInitialDirtyBitsMask() const
{
    int mask = HdChangeTracker::Clean
        | HdChangeTracker::DirtyPrimID
        | HdChangeTracker::DirtyDisplayStyle
        | HdChangeTracker::DirtyPrimvar
        | HdChangeTracker::DirtyMaterialId
        | HdChangeTracker::DirtyTopology
        | HdChangeTracker::DirtyTransform
        | HdChangeTracker::DirtyVisibility
        | HdChangeTracker::DirtyDoubleSided
        | HdChangeTracker::DirtySubdivTags
        | HdChangeTracker::DirtyWidths
        | HdChangeTracker::DirtyInstancer
        | HdChangeTracker::DirtyInstanceIndex
        | HdChangeTracker::DirtyCategories;
    return static_cast<HdDirtyBits>(mask);
}

void HdGraviBasisCurves::Sync(HdSceneDelegate *_sceneDelegate, HdRenderParam *_renderParam, HdDirtyBits *_dirtyBits, TfToken const &_reprToken)
{
    _UpdateVisibility(_sceneDelegate, _dirtyBits);
    _UpdateInstancer(_sceneDelegate, _dirtyBits);
    SyncAll(_sceneDelegate, *_renderParam, _dirtyBits, _reprToken);
}

//-------------------------------------------------------------------------
// Primvar Management
//-------------------------------------------------------------------------

void HdGraviBasisCurves::SyncPrimvar(HdSceneDelegate *_sceneDelegate, HdRenderParam &_renderParam, const TfToken &_name, const VtValue &_value, const HdInterpolation &_interp, const TfToken &_role)
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

void HdGraviBasisCurves::CreateBVH()
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

        for (auto m_point : m_points)
        {
            GfVec3d transformedPoint = m_transform.Transform(GfVec3d(m_point));

            GfVec3d radiusVec(maxRadius, maxRadius, maxRadius);
            range.UnionWith(GfRange3d(
                transformedPoint - radiusVec,
                transformedPoint + radiusVec
                ));
        }

        m_bvh.SetRange(range);
    }
}

IntersectData HdGraviBasisCurves::Intersect(const GfRay &_ray) const
{
    // Early exit if no geometry
    if (m_points.empty()) {
        return { -1.0f, GfVec3f(0.0f), GfVec4f(0.0f), 0, this };
    }

    // Validate data arrays
    const bool useGlobalWidth = m_widths.empty() || m_widths.size() != m_points.size();
    const bool useGlobalColors = m_displayColors.empty() || m_displayColors.size() != m_points.size();
    const float defaultWidth = m_widths.empty() ? 1.0f : m_widths[0];
    const GfVec3f defaultColor = m_displayColors.empty() ? GfVec3f(1.0f) : m_displayColors[0];

    double closestT = std::numeric_limits<double>::infinity();
    GfVec3f N(0.0f);
    GfVec3f Cd = defaultColor;

        for (size_t i = 0; i < m_points.size(); ++i) {
            const float radius = useGlobalWidth ? defaultWidth :
                (i < m_widths.size() ? m_widths[i] : defaultWidth) * 0.05f;

            double t;
            if (_ray.Intersect(m_points[i], radius, &t)) {
                if (t > 0.0001f && t < closestT) {
                    Cd = useGlobalColors ? defaultColor :
                        (i < m_displayColors.size() ? m_displayColors[i] : defaultColor);
                    closestT = t;
                    N = GfVec3f((_ray.GetPoint(t) - m_points[i]).GetNormalized());
                }
            }
        }


    closestT = (closestT == std::numeric_limits<double>::infinity()) ? -1.0 : closestT;
    return {static_cast<float>(closestT), N, GfVec4f(Cd[0], Cd[1], Cd[2], 1.0f),
            static_cast<int32_t>(GetId().GetHash()), this};
}

//-------------------------------------------------------------------------
// Hydra Internal Interface
//-------------------------------------------------------------------------

HdDirtyBits HdGraviBasisCurves::_PropagateDirtyBits(const HdDirtyBits _bits) const { return _bits; }

PXR_NAMESPACE_CLOSE_SCOPE