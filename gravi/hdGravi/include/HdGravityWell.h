// Copyright 2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

/// @file HdGravityWell.h
/// @brief Gravity well support for Gravi's Hydra render delegate
///
/// Implements gravity well rendering within the Gravi renderer, providing:
/// - USD support through Hydra's render delegate interface
/// - Data synchronization from USD
/// - BVH acceleration structure construction
/// - Ray intersection testing
/// - Force and radius attribute management
///
/// @author Christopher Hosken
/// @version 1.0
/// @date 07/05/2025 Pre-release cleanup
/// @ingroup hdGravi

#ifndef HDGRAVI_HDGRAVITYWELL_H_
#define HDGRAVI_HDGRAVITYWELL_H_

#include <pxr/pxr.h>
#include <pxr/imaging/hd/points.h>

#include "HdGraviPrim.h"

PXR_NAMESPACE_OPEN_SCOPE

//-----------------------------------------------------------------------------
/// @class HdGravityWell
/// @brief Hydra render delegate implementation for gravity wells in Gravi
///
/// Combines HdRprim (standard Hydra functionality) and HdGraviPrim (Gravi
/// features) to provide:
/// - Complete USD data synchronization
/// - Optimized BVH construction
/// - Force and radius attribute management
/// - Efficient ray intersection testing
//-----------------------------------------------------------------------------
class HdGravityWell : public HdRprim, public HdGraviPrim
{
public:
    //-------------------------------------------------------------------------
    /// @name Construction / Destruction
    //-------------------------------------------------------------------------
    /// @{

    /// @brief Constructor
    /// @param _id The scene path identifier for this gravity well
    explicit HdGravityWell(SdfPath const& _id);

    /// @}

    //-------------------------------------------------------------------------
    /// @name Hydra Primitive Interface
    //-------------------------------------------------------------------------
    /// @{

    /// @brief Get built-in primvar names
    /// @return Const reference to token vector of primvar names
    TfTokenVector const& GetBuiltinPrimvarNames() const override;

    /// @brief Get the initial dirty state mask
    /// @return Bitmask of initial dirty states needed for first sync
    HdDirtyBits GetInitialDirtyBitsMask() const override;

    /// @brief Synchronize this gravity well with USD scene data
    /// @param _sceneDelegate The scene delegate providing the data
    /// @param _renderParam Renderer-specific parameters
    /// @param _dirtyBits Bitmask indicating which attributes need sync
    /// @param _reprToken The representation token to sync
    ///
    /// @note Thread-safe for parallel execution
    void Sync(HdSceneDelegate* _sceneDelegate,
              HdRenderParam* _renderParam,
              HdDirtyBits* _dirtyBits,
              TfToken const& _reprToken) override;

    /// @brief Finalize the gravity well (no-op implementation)
    /// @param _renderParam Renderer-specific parameters
    void Finalize(HdRenderParam* _renderParam) override {}

    /// @}

    //-------------------------------------------------------------------------
    /// @name Attribute Synchronization
    //-------------------------------------------------------------------------
    /// @{

    /// @brief Synchronize gravity well attributes
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

    /// @brief Synchronize a primvar attribute
    /// @param _sceneDelegate The scene delegate providing the data
    /// @param _renderParam Renderer-specific parameters
    /// @param _name The name of the primvar to sync
    /// @param _value The value of the primvar
    /// @param _interp The interpolation mode
    /// @param _role The role of the primvar data
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

    /// @brief Build the BVH acceleration structure
    ///
    /// @note Optimized for gravity well spatial characteristics
    void CreateBVH() override;

    /// @brief Intersect a ray with the gravity well
    /// @param _ray The ray to test for intersection
    /// @return IntersectData containing hit information (t=-1 if no hit)
    [[nodiscard]] IntersectData Intersect(const GfRay& _ray) const override;

    /// @}

    //-------------------------------------------------------------------------
    /// @name Gravity Well Properties
    //-------------------------------------------------------------------------
    /// @{

    /// @brief Get the force strength
    /// @return Current force value
    [[nodiscard]] float GetForce() const;

    /// @}

protected:
    //-------------------------------------------------------------------------
    /// @name Hydra Internal Interface
    //-------------------------------------------------------------------------
    /// @{

    /// @brief Initialize a representation (no-op implementation)
    /// @param _reprToken The representation token to initialize
    /// @param _dirtyBits Bitmask of dirty states
    void _InitRepr(TfToken const& _reprToken, HdDirtyBits* _dirtyBits) override {}

    /// @brief Propagate dirty bits (passthrough implementation)
    /// @param _bits Input dirty bits
    /// @return The same dirty bits (no propagation)
    HdDirtyBits _PropagateDirtyBits(HdDirtyBits _bits) const override;

    /// @}

private:
    //-------------------------------------------------------------------------
    /// @name Private Members
    //-------------------------------------------------------------------------
    /// @{

    float m_force = 1.0f;    ///< Force strength (default 1.0)

    /// @}
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif // HDGRAVI_HDGRAVITYWELL_H_