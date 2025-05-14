///
/// @file Mesh.cpp
/// @brief Mesh support for USD scene data.

#include <pxr/base/gf/ray.h>
#include <pxr/base/gf/matrix4d.h>
#include <pxr/imaging/hd/extComputationUtils.h>
#include <pxr/imaging/hd/meshUtil.h>

#include "RenderParam.h"
#include "RenderPass.h"
#include "Scene.h"

#include "Mesh.h"

#include <iostream>

PXR_NAMESPACE_OPEN_SCOPE

//-------------------------------------------------------------------------
// Construction / Destruction
//-------------------------------------------------------------------------

HdGraviMesh::HdGraviMesh(SdfPath const &_id) : HdMesh(_id), HdGraviPrim(this) {}

//-------------------------------------------------------------------------
// Hydra Mesh Interface
//-------------------------------------------------------------------------

HdDirtyBits HdGraviMesh::GetInitialDirtyBitsMask() const
{
    // The initial dirty bits control what data is available on the first run
    constexpr int mask = HdChangeTracker::Clean
        | HdChangeTracker::InitRepr
        | HdChangeTracker::DirtyPoints
        | HdChangeTracker::DirtyTopology
        | HdChangeTracker::DirtyTransform
        | HdChangeTracker::DirtyVisibility
        | HdChangeTracker::DirtyCullStyle
        | HdChangeTracker::DirtyDoubleSided
        | HdChangeTracker::DirtyDisplayStyle
        | HdChangeTracker::DirtySubdivTags
        | HdChangeTracker::DirtyInstancer
        | HdChangeTracker::DirtyPrimvar
        | HdChangeTracker::DirtyNormals
        | HdChangeTracker::DirtyInstancer;

    return mask;
}

void HdGraviMesh::Sync(HdSceneDelegate *_sceneDelegate, HdRenderParam *_renderParam, HdDirtyBits *_dirtyBits, TfToken const &_reprToken)
{
    _UpdateVisibility(_sceneDelegate, _dirtyBits);
    _UpdateInstancer(_sceneDelegate, _dirtyBits);
    SyncAll(_sceneDelegate, *_renderParam, _dirtyBits, _reprToken);
}

void HdGraviMesh::SyncPrimvar(HdSceneDelegate *_sceneDelegate, HdRenderParam &_renderParam, const TfToken &_name, const VtValue &_value, const HdInterpolation &_interp, const TfToken &_role)
{

    // UVs
    if (_name == TfToken("uv") || _name == TfToken("st")) {
        if (_value.IsEmpty()) {
            m_uvs = {GfVec2f(0.0f, 0.0f)}; // set default value
        } else if (_value.IsHolding<VtVec2fArray>()) {
            m_uvs = _value.UncheckedGet<VtVec2fArray>();
        }
        return;
    }

    HdGraviPrim::SyncPrimvar(_sceneDelegate, _renderParam, _name, _value, _interp, _role);
}

void HdGraviMesh::SyncAttributes(HdSceneDelegate *_sceneDelegate, HdRenderParam &_renderParam, const HdDirtyBits* _dirtyBits, TfToken const &_reprToken, bool _updateBVH)
{
    SdfPath const& id = GetId();

    if (HdChangeTracker::IsDisplayStyleDirty(*_dirtyBits, id) ||
        HdChangeTracker::IsReprDirty(*_dirtyBits, id) ||
        HdChangeTracker::IsTopologyDirty(*_dirtyBits, id) ||
        HdChangeTracker::IsSubdivTagsDirty(*_dirtyBits, id)) {
        m_topology = GetMeshTopology(_sceneDelegate);

        HdMeshUtil meshUtil(&m_topology, id);
        meshUtil.ComputeTriangleIndices(&m_triangulatedIndices,
                                        &m_trianglePrimitiveParams);

        _updateBVH = true;
    }

    HdGraviPrim::SyncAttributes(_sceneDelegate, _renderParam, _dirtyBits, _reprToken, _updateBVH);

    if (_updateBVH) {
        // Clear previous triangles
        m_triangles.clear();
        m_triangles.reserve(m_triangulatedIndices.size());

        // Iterate over all triangles in the mesh
        for (size_t i = 0; i < m_triangulatedIndices.size(); i += 1) {
            // Extract and transform triangle vertices
            GfVec3d p0 = m_points[m_triangulatedIndices[i][0]];
            GfVec3d p1 = m_points[m_triangulatedIndices[i][1]];
            GfVec3d p2 = m_points[m_triangulatedIndices[i][2]];

            // Store the triangle
            m_triangles.emplace_back(p0, p1, p2);
        }

        BuildLocalBVH();
    }
}

//-------------------------------------------------------------------------
// Rendering Operations
//-------------------------------------------------------------------------

void HdGraviMesh::CreateBVH()
{
    m_bvh = GfBBox3d();

    if (!m_points.empty())
    {
        GfRange3d range;

        for (const GfVec3f &point : m_points)
        {
            range.UnionWith(m_transform.Transform(GfVec3d(point)));
        }

        m_bvh.SetRange(range);
    }
}

IntersectData HdGraviMesh::Intersect(const GfRay& _ray) const
{
    if (m_bvhRoot == -1) {
        return IntersectData{-1.0f, GfVec3f(0.0f), GfVec4f(0.0f, 0.0f, 0.0f, 1.0f)};
    }

    double closestT = std::numeric_limits<double>::infinity();
    GfVec3f normal(0.0f);
    GfVec3f Cd(1.0f);


    // Start BVH traversal
    IntersectBVHNode(_ray, m_bvhRoot, closestT, normal);

    // Return the closest intersection t-value (or -1.0 if no intersection)
    closestT = closestT == std::numeric_limits<double>::infinity() ? -1.0f : closestT;

    if (!m_displayColors.empty()) {
        Cd = m_displayColors[0];
    }

    GfVec4f CdOut(Cd[0], Cd[1], Cd[2], 1.0f);

    return IntersectData{
        static_cast<float>(closestT),
        normal,
        CdOut,
        static_cast<int32_t>(GetId().GetHash()),
        this
    };
}

//-------------------------------------------------------------------------
// Hydra Internal Interface
//-------------------------------------------------------------------------

HdDirtyBits HdGraviMesh::_PropagateDirtyBits(const HdDirtyBits _bits) const { return _bits; }

//-------------------------------------------------------------------------
// Private Methods
//-------------------------------------------------------------------------

int HdGraviMesh::BuildBVHNode(size_t start, size_t end)
{
    // Create new node
    BVHNode node;
    int nodeIndex = m_bvhNodes.size();
    m_bvhNodes.push_back(node);

    // Calculate bounds for all triangles in this node
    GfRange3d bounds;
    for (size_t i = start; i < end; ++i) {
        const GfTriangle& tri = m_triangles[m_bvhTriIndices[i]];
        bounds.UnionWith(GfVec3d(tri.p0));
        bounds.UnionWith(GfVec3d(tri.p1));
        bounds.UnionWith(GfVec3d(tri.p2));
    }
    m_bvhNodes[nodeIndex].bounds = bounds;

    // If few triangles, make a leaf node
    if (constexpr size_t maxLeafSize = 4; end - start <= maxLeafSize) {
        m_bvhNodes[nodeIndex].firstTri = start;
        m_bvhNodes[nodeIndex].numTris = end - start;
        m_bvhNodes[nodeIndex].leftChild = -1;
        m_bvhNodes[nodeIndex].rightChild = -1;
        return nodeIndex;
    }

    // Choose split axis (longest axis)
    GfVec3d diag = bounds.GetMax() - bounds.GetMin();
    int axis = 0;
    if (diag[1] > diag[axis]) axis = 1;
    if (diag[2] > diag[axis]) axis = 2;
    float splitPos = 0.5f * (bounds.GetMin()[axis] + bounds.GetMax()[axis]);

    // Partition triangles based on centroids
    size_t mid = start;
    for (size_t i = start; i < end; ++i) {
        const GfTriangle& tri = m_triangles[m_bvhTriIndices[i]];
        GfVec3d centroid = (tri.p0 + tri.p1 + tri.p2) / 3.0;
        if (centroid[axis] < splitPos) {
            std::swap(m_bvhTriIndices[i], m_bvhTriIndices[mid]);
            ++mid;
        }
    }

    // If partition failed, split in the middle
    if (mid == start || mid == end) {
        mid = start + (end - start) / 2;
    }

    // Recursively build child nodes
    m_bvhNodes[nodeIndex].leftChild = BuildBVHNode(start, mid);
    m_bvhNodes[nodeIndex].rightChild = BuildBVHNode(mid, end);

    return nodeIndex;
}

void HdGraviMesh::BuildLocalBVH()
{
    // Clear previous BVH data
    m_bvhNodes.clear();
    m_bvhTriIndices.clear();

    // If no triangles, return empty BVH
    if (m_triangles.empty()) {
        m_bvhRoot = -1;
        return;
    }

    // Create triangle indices
    m_bvhTriIndices.resize(m_triangles.size());
    for (size_t i = 0; i < m_triangles.size(); ++i) {
        m_bvhTriIndices[i] = i;
    }

    // Build BVH recursively
    m_bvhRoot = BuildBVHNode(0, m_triangles.size());
}

void HdGraviMesh::IntersectBVHNode(const GfRay& ray, int nodeIndex, double& closestT, GfVec3f& normal) const
{
    const BVHNode& node = m_bvhNodes[nodeIndex];

    // Check ray against node bounds
    double tMin, tMax;
    if (!ray.Intersect(node.bounds, &tMin, &tMax) || tMin > closestT) {
        return;
    }

    // If leaf node, check all triangles
    if (node.leftChild == -1) {
        for (size_t i = 0; i < node.numTris; ++i) {
            size_t triIndex = m_bvhTriIndices[node.firstTri + i];
            const GfTriangle& tri = m_triangles[triIndex];

            double t;
            GfVec3d barycentricCoords;
            bool frontFacing;
            bool hit = ray.Intersect(tri.p0, tri.p1, tri.p2, &t, &barycentricCoords, &frontFacing, closestT);

            if (hit && t > 0.0001 && t < closestT) {
                GfVec3f computedNormal = GfVec3f(GfCross(tri.p1 - tri.p0, tri.p2 - tri.p0));
                if (computedNormal.Normalize()) { // Only use if normalization succeeds
                    // Flip normal if back-facing (optional)
                    GfVec3f rayDirection = GfVec3f(ray.GetDirection());
                    if (GfDot(computedNormal, rayDirection) > 0) {
                        computedNormal = -computedNormal;
                    }
                    normal = computedNormal;
                    closestT = t;
                }
            }
        }
        return;
    }

    // Recursively check child nodes
    IntersectBVHNode(ray, node.leftChild, closestT, normal);
    IntersectBVHNode(ray, node.rightChild, closestT, normal);
}


PXR_NAMESPACE_CLOSE_SCOPE