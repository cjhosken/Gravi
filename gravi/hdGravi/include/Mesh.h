// Copyright 2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
//
// Copyright 2025 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.

/// @file Mesh.h
/// @brief Mesh primitive implementation for Gravi renderer
///
/// Provides USD mesh support including:
/// - Geometry synchronization and triangulation
/// - Primvar interpolation and management
/// - Ray intersection acceleration
/// - BVH construction and traversal
/// - Topology management
///
/// @author Christopher Hosken
/// @version 1.0
/// @date 07/05/2025 Pre-release cleanup
/// @ingroup hdGravi

#ifndef HDGRAVI_HDGRAVIMESH_H_
#define HDGRAVI_HDGRAVIMESH_H_

#include <pxr/pxr.h>
#include <pxr/imaging/hd/mesh.h>
#include <pxr/imaging/hd/enums.h>
#include <pxr/base/gf/matrix4f.h>

#include "HdGraviPrim.h"

PXR_NAMESPACE_OPEN_SCOPE

//-----------------------------------------------------------------------------
/// @class HdGraviMesh
/// @brief Gravi render delegate implementation for USD mesh primitives
///
/// Handles mesh geometry processing including:
/// - Complete topology management
/// - Optimized BVH acceleration structure
/// - Efficient ray intersection
/// - Primvar interpolation and storage
/// - Thread-safe synchronization
//-----------------------------------------------------------------------------
class HdGraviMesh final : public HdMesh, public HdGraviPrim
{
public:
    //-------------------------------------------------------------------------
    /// @name Construction / Destruction
    //-------------------------------------------------------------------------
    /// @{

    /// @brief Constructor
    /// @param _id Scene path identifier for this mesh
    explicit HdGraviMesh(SdfPath const& _id);

    /// @}

    //-------------------------------------------------------------------------
    /// @name Hydra Mesh Interface
    //-------------------------------------------------------------------------
    /// @{

    /// @brief Get initial dirty state mask
    /// @return Bitmask of initial dirty states
    HdDirtyBits GetInitialDirtyBitsMask() const override;

    /// @brief Synchronize mesh with USD scene data
    /// @param _sceneDelegate Scene data provider
    /// @param _renderParam Global render parameters
    /// @param _dirtyBits Bitmask of changed states
    /// @param _reprToken Representation token
    ///
    /// @note Thread-safe for parallel execution
    void Sync(HdSceneDelegate* _sceneDelegate,
              HdRenderParam* _renderParam,
              HdDirtyBits* _dirtyBits,
              const TfToken& _reprToken) override;

    /// @brief Synchronize a primvar attribute
    /// @param _sceneDelegate Scene data provider
    /// @param _renderParam Global render parameters
    /// @param _name Primvar name
    /// @param _value Primvar value
    /// @param _interp Interpolation mode
    /// @param _role Primvar role
    void SyncPrimvar(HdSceneDelegate* _sceneDelegate,
                     HdRenderParam& _renderParam,
                     const TfToken& _name,
                     const VtValue& _value,
                     const HdInterpolation& _interp,
                     const TfToken& _role) override;

    /// @brief Synchronize mesh attributes
    /// @param _sceneDelegate Scene data provider
    /// @param _renderParam Global render parameters
    /// @param _dirtyBits Bitmask of changed states
    /// @param _reprToken Representation token
    /// @param _updateBVH Whether to rebuild BVH
    void SyncAttributes(HdSceneDelegate* _sceneDelegate,
                       HdRenderParam& _renderParam,
                       const HdDirtyBits* _dirtyBits,
                       TfToken const& _reprToken,
                       bool _updateBVH) override;

    /// @brief Finalize the mesh (no-op implementation)
    /// @param _renderParam Global render parameters
    void Finalize(HdRenderParam* _renderParam) override {}

    /// @}

    //-------------------------------------------------------------------------
    /// @name Rendering Operations
    //-------------------------------------------------------------------------
    /// @{

    /// @brief Build BVH acceleration structure
    ///
    /// Constructs an optimized BVH for efficient ray intersection
    void CreateBVH() override;

    /// @brief Intersect ray with mesh
    /// @param _ray Ray to test
    /// @return Intersection data (t=-1 if no hit)
    [[nodiscard]] IntersectData Intersect(const GfRay& _ray) const override;

    /// @}

protected:
    //-------------------------------------------------------------------------
    /// @name Hydra Internal Interface
    //-------------------------------------------------------------------------
    /// @{

    /// @brief Initialize representation (no-op implementation)
    /// @param _reprToken Representation token
    /// @param _dirtyBits Dirty state bits
    void _InitRepr(TfToken const& _reprToken, HdDirtyBits* _dirtyBits) override {}

    /// @brief Propagate dirty bits (passthrough implementation)
    /// @param _bits Input dirty bits
    /// @return Same dirty bits (no propagation)
    HdDirtyBits _PropagateDirtyBits(HdDirtyBits _bits) const override;

    /// @}

private:
    //-------------------------------------------------------------------------
    /// @name BVH Structures
    //-------------------------------------------------------------------------
    /// @{

    /// @struct GfTriangle
    /// @brief Triangle storage for BVH construction
    struct GfTriangle {
        GfVec3d p0, p1, p2;  ///< Triangle vertices
        GfTriangle(const GfVec3d& _p0, const GfVec3d& _p1, const GfVec3d& _p2) : p0(_p0), p1(_p1), p2(_p2) {}
    };

    /// @struct BVHNode
    /// @brief Node in BVH acceleration structure
    struct BVHNode {
        GfRange3d bounds;      ///< Bounding box of node
        int leftChild = -1;     ///< Index of left child (-1 for leaf)
        int rightChild = -1;    ///< Index of right child (-1 for leaf)
        size_t firstTri = 0;    ///< First triangle index (for leaf nodes)
        size_t numTris = 0;     ///< Triangle count (for leaf nodes)
    };

    /// @}

    //-------------------------------------------------------------------------
    /// @name Private Members
    //-------------------------------------------------------------------------
    /// @{

    VtVec2fArray m_uvs;                        ///< UV coordinates
    HdMeshTopology m_topology;                 ///< Mesh topology
    VtVec3iArray m_triangulatedIndices;        ///< Triangulated indices
    VtIntArray m_trianglePrimitiveParams;      ///< Triangle primitive params
    std::vector<GfTriangle> m_triangles;       ///< Triangle geometry
    std::vector<BVHNode> m_bvhNodes;           ///< BVH node storage
    std::vector<size_t> m_bvhTriIndices;       ///< BVH triangle indices
    int m_bvhRoot = -1;                        ///< Root node index

    /// @struct PrimvarSource
    /// @brief Primvar data storage
    struct PrimvarSource {
        VtValue m_data;                 ///< Primvar data
        HdInterpolation m_interpolation; ///< Interpolation mode
    };

    TfHashMap<TfToken, PrimvarSource, TfToken::HashFunctor> m_primvarSourceMap; ///< Primvar storage

    /// @}

    //-------------------------------------------------------------------------
    /// @name Private Methods
    //-------------------------------------------------------------------------
    /// @{

    /// @brief Build BVH node recursively
    /// @param _start Start triangle index
    /// @param _end End triangle index
    /// @return Node index
    int BuildBVHNode(size_t _start, size_t _end);

    /// @brief Build local BVH structure
    void BuildLocalBVH();

    /// @brief Intersect ray with BVH node
    /// @param _ray Ray to test
    /// @param _nodeIndex Node index to test
    /// @param _closestT [in/out] Closest intersection distance
    /// @param _normal [out] Intersection normal
    void IntersectBVHNode(const GfRay& _ray,
                         int _nodeIndex,
                         double& _closestT,
                         GfVec3f& _normal) const;

    /// @}
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif // HDGRAVI_HDGRAVIMESH_H_