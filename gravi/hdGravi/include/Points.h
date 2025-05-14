// Copyright 2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

/// @file Points.h
/// @brief Point cloud support for Gravi's Hydra render delegate
///
/// Provides USD point cloud rendering with:
/// - BVH acceleration structure
/// - Efficient ray intersection
/// - Per-point width/radius attributes
/// - Primvar synchronization
/// - Thread-safe updates
///
/// @author Christopher Hosken
/// @version 1.0
/// @date 18/04/2025 Pre-release cleanup
/// @ingroup hdGravi

#ifndef HDGRAVI_HDGRAVIPOINTS_H_
#define HDGRAVI_HDGRAVIPOINTS_H_

#include <pxr/pxr.h>
#include <pxr/imaging/hd/points.h>

#include "HdGraviPrim.h"

PXR_NAMESPACE_OPEN_SCOPE

//-----------------------------------------------------------------------------
/// @class HdGraviPoints
/// @brief Hydra render delegate implementation for point clouds in Gravi
///
/// Handles point cloud rendering including:
/// - Geometry synchronization from USD
/// - BVH acceleration structure construction
/// - Ray intersection testing
/// - Primvar management and interpolation
/// - Per-point width/radius attributes
//-----------------------------------------------------------------------------
class HdGraviPoints final : public HdPoints, public HdGraviPrim
{
public:
    //-------------------------------------------------------------------------
    /// @name Construction / Destruction
    //-------------------------------------------------------------------------
    /// @{

    /// @brief Constructor
    /// @param _id Scene path identifier for this point cloud
    explicit HdGraviPoints(SdfPath const& _id);

    /// @}

    //-------------------------------------------------------------------------
    /// @name Hydra Points Interface
    //-------------------------------------------------------------------------
    /// @{

    /// @brief Get initial dirty state mask
    /// @return Bitmask of initial dirty states
    HdDirtyBits GetInitialDirtyBitsMask() const override;

    /// @brief Synchronize point cloud with scene data
    /// @param _sceneDelegate Scene data provider
    /// @param _renderParam Global render parameters
    /// @param _dirtyBits Bitmask of changed states
    /// @param _reprToken Representation token
    ///
    /// @note Thread-safe for parallel execution
    void Sync(HdSceneDelegate* _sceneDelegate,
              HdRenderParam* _renderParam,
              HdDirtyBits* _dirtyBits,
              TfToken const& _reprToken) override;

    /// @brief Finalize the point cloud (no-op implementation)
    /// @param _renderParam Global render parameters
    void Finalize(HdRenderParam* _renderParam) override {}

    /// @}

    //-------------------------------------------------------------------------
    /// @name Primvar Management
    //-------------------------------------------------------------------------
    /// @{

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

    /// @}

    //-------------------------------------------------------------------------
    /// @name Rendering Operations
    //-------------------------------------------------------------------------
    /// @{

    /// @brief Build BVH acceleration structure
    ///
    /// Constructs an optimized BVH for efficient point cloud intersection
    void CreateBVH() override;

    /// @brief Intersect ray with point cloud
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
    /// @name Private Members
    //-------------------------------------------------------------------------
    /// @{

    VtFloatArray m_widths;  ///< Per-point width/radius values

    /// @}
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif // HDGRAVI_HDGRAVIPOINTS_H_