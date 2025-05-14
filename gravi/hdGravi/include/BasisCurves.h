// Copyright 2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

/// @file BasisCurves.h
/// @brief Basis curves implementation for the Gravi renderer
///
/// This class implements USD basis curves support for Gravi, handling:
/// - Curve geometry synchronization
/// - Width interpolation
/// - BVH acceleration structure generation
/// - Ray intersection testing
///
/// @author Christopher Hosken
/// @version 1.0
/// @date 07/05/2025 Pre-release cleanup
/// @ingroup hdGravi

#ifndef HDGRAVI_BASISCURVES_H_
#define HDGRAVI_BASISCURVES_H_

#include <pxr/pxr.h>
#include <pxr/imaging/hd/basisCurves.h>

#include "HdGraviPrim.h"

PXR_NAMESPACE_OPEN_SCOPE

//-----------------------------------------------------------------------------
/// @class HdGraviBasisCurves
/// @brief Hydra basis curves implementation for the Gravi renderer
///
/// Supports:
/// - Multiple curve basis types (e.g., Bézier, B-spline)
/// - Width interpolation (constant, uniform, vertex-varying)
/// - BVH acceleration for efficient ray intersection
/// - Thread-safe primvar synchronization
//-----------------------------------------------------------------------------
class HdGraviBasisCurves : public HdBasisCurves, public HdGraviPrim
{
public:
    //-------------------------------------------------------------------------
    /// @name Construction / Destruction
    //-------------------------------------------------------------------------
    /// @{

    /// @brief Constructor
    /// @param _id Scene path identifier for these curves
    explicit HdGraviBasisCurves(SdfPath const& _id);

    /// @}

    //-------------------------------------------------------------------------
    /// @name Hydra Primitive Interface
    //-------------------------------------------------------------------------
    /// @{

    /// @brief Get initial dirty state mask
    /// @return Bitmask of states needed for first sync
    HdDirtyBits GetInitialDirtyBitsMask() const override;

    /// @brief Synchronize curve data from the scene delegate
    /// @param _sceneDelegate Scene data provider
    /// @param _renderParam Global render parameters
    /// @param _dirtyBits Bitmask of changed states
    /// @param _reprToken Representation token
    ///
    /// @note Thread-safe for parallel execution
    void Sync(
        HdSceneDelegate* _sceneDelegate,
        HdRenderParam* _renderParam,
        HdDirtyBits* _dirtyBits,
        TfToken const& _reprToken
    ) override;

    /// @brief Finalize the curves (no-op in this implementation)
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
    void SyncPrimvar(
        HdSceneDelegate* _sceneDelegate,
        HdRenderParam& _renderParam,
        const TfToken& _name,
        const VtValue& _value,
        const HdInterpolation& _interp,
        const TfToken& _role
    ) override;

    /// @}

    //-------------------------------------------------------------------------
    /// @name Rendering Operations
    //-------------------------------------------------------------------------
    /// @{

    /// @brief Build BVH acceleration structure for ray intersection
    void CreateBVH() override;

    /// @brief Intersect a ray with the curves
    /// @param _ray Ray to test
    /// @return Intersection data (t=-1 if no hit)
    IntersectData Intersect(const GfRay& _ray) const override;

    /// @}

protected:
    //-------------------------------------------------------------------------
    /// @name Hydra Internal Interface
    //-------------------------------------------------------------------------
    /// @{

    /// @brief Initialize representation (no-op in this implementation)
    /// @param _reprToken Representation token
    /// @param _dirtyBits Dirty state bits
    void _InitRepr(
        TfToken const& _reprToken,
        HdDirtyBits* _dirtyBits
    ) override {}

    /// @brief Propagate dirty bits (no propagation in this implementation)
    /// @param _bits Input dirty bits
    /// @return Same dirty bits (passthrough)
    HdDirtyBits _PropagateDirtyBits(HdDirtyBits _bits) const override;

    /// @}

private:
    //-------------------------------------------------------------------------
    // Private Members
    //-------------------------------------------------------------------------
    VtFloatArray m_widths;  ///< Per-curve or per-point width values
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif // HDGRAVI_BASISCURVES_H_