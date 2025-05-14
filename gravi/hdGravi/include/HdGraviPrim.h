// Copyright 2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

/// @file HdGraviPrim.h
/// @brief Base class for all Gravi render primitives.
///
/// Provides shared functionality for:
/// - Primitive synchronization and updates
/// - Ray intersection testing
/// - BVH acceleration structure management
/// - Primitive attribute handling
/// - Visibility and material properties
///
/// @author Christopher Hosken
/// @version 1.0
/// @date 07/05/2025 Pre-release cleanup
/// @ingroup hdGravi

#ifndef HDGRAVI_HDGRAVIPRIM_H_
#define HDGRAVI_HDGRAVIPRIM_H_

#include <pxr/imaging/hd/extComputationUtils.h>

#include "Common.h"

PXR_NAMESPACE_OPEN_SCOPE

//-----------------------------------------------------------------------------
/// @class HdGraviPrim
/// @brief Abstract base class for Gravi render primitives
///
/// Implements common functionality shared by all Gravi primitives including:
/// - Core synchronization pipeline
/// - BVH acceleration structure management
/// - Ray intersection interface
/// - Primitive attribute handling
/// - Transform and visibility state
///
/// @note All concrete primitive types should derive from this base class
//-----------------------------------------------------------------------------
class HdGraviPrim
{
public:
    //-------------------------------------------------------------------------
    /// @name Construction / Destruction
    //-------------------------------------------------------------------------
    /// @{

    /// @brief Construct a new primitive
    /// @param _rprim The Hydra primitive this will represent
    explicit HdGraviPrim(HdRprim* _rprim);

    /// @brief Virtual destructor
    virtual ~HdGraviPrim() = default;

    /// @}

    //-------------------------------------------------------------------------
    /// @name Synchronization Interface
    //-------------------------------------------------------------------------
    /// @{

    /// @brief Full primitive synchronization
    /// @param _sceneDelegate Scene data provider
    /// @param _renderParam Global render parameters
    /// @param _dirtyBits Bitmask of changed states
    /// @param _reprToken Representation token
    ///
    /// Coordinates the complete synchronization pipeline including:
    /// 1. Attribute updates
    /// 2. Primvar synchronization
    /// 3. BVH updates (if needed)
    ///
    /// @note Thread-safe for parallel execution
    void SyncAll(HdSceneDelegate* _sceneDelegate,
                 HdRenderParam& _renderParam,
                 HdDirtyBits* _dirtyBits,
                 TfToken const& _reprToken);

    /// @}

    //-------------------------------------------------------------------------
    /// @name Rendering Interface
    //-------------------------------------------------------------------------
    /// @{

    /// @brief Intersect ray with primitive
    /// @param _ray Ray to test
    /// @return Intersection data (t=-1 if no hit)
    ///
    /// @note Derived classes must implement primitive-specific intersection logic
    [[nodiscard]] virtual IntersectData Intersect(const GfRay& _ray) const = 0;

    /// @brief Get the primitive's bounding volume hierarchy
    /// @return The primitive's BVH
    [[nodiscard]] GfBBox3d GetBVH() const;

    /// @brief Check if primitive is a volume
    /// @return False by default (override in volume primitives)
    [[nodiscard]] virtual bool IsVolume() const;

    /// @brief Get local-to-world transform
    /// @return The transform matrix
    [[nodiscard]] GfMatrix4d GetTransform() const;

    /// @brief Get instance transforms (if instanced)
    /// @return Array of instance transforms
    [[nodiscard]] VtMatrix4dArray GetInstanceTransforms() const;

    /// @brief Check if primitive has instances
    /// @return True if instanced
    [[nodiscard]] bool HasInstances() const;

    /// @brief Set the primitive's transform
    /// @param _transform New transform matrix
    void SetTransform(const GfMatrix4d& _transform);

    /// @}

protected:
    //-------------------------------------------------------------------------
    /// @name Protected Members
    //-------------------------------------------------------------------------
    /// @{

    HdRprim& m_rprim;                ///< Reference to Hydra primitive
    GfBBox3d m_bvh;                  ///< Bounding volume hierarchy
    GfMatrix4d m_transform{};        ///< Local-to-world transform
    VtVec3fArray m_points;           ///< Geometry points
    VtVec3fArray m_displayColors;    ///< Display colors
    VtVec3fArray m_velocities;       ///< Motion vectors
    std::set<TfToken> m_primvars;    ///< Active primvar names
    bool m_doubleSided = false;      ///< Double-sided rendering flag
    bool m_visible = true;           ///< Visibility state

    /// @}

    //-------------------------------------------------------------------------
    /// @name Protected Virtual Methods
    //-------------------------------------------------------------------------
    /// @{

    /// @brief Synchronize primitive attributes
    /// @param _sceneDelegate Scene data provider
    /// @param _renderParam Global render parameters
    /// @param _dirtyBits Bitmask of changed states
    /// @param _reprToken Representation token
    /// @param _updateBVH Whether to rebuild BVH
    ///
    /// @note Derived classes should extend with primitive-specific attributes
    virtual void SyncAttributes(HdSceneDelegate* _sceneDelegate,
                               HdRenderParam& _renderParam,
                               const HdDirtyBits* _dirtyBits,
                               TfToken const& _reprToken,
                               bool _updateBVH);

    /// @brief Synchronize a single primvar
    /// @param _sceneDelegate Scene data provider
    /// @param _renderParam Global render parameters
    /// @param _name Primvar name
    /// @param _value Primvar value
    /// @param _interp Interpolation mode
    /// @param _role Primvar role
    virtual void SyncPrimvar(HdSceneDelegate* _sceneDelegate,
                            HdRenderParam& _renderParam,
                            const TfToken& _name,
                            const VtValue& _value,
                            const HdInterpolation& _interp,
                            const TfToken& _role);

    /// @brief Create/update the BVH
    ///
    /// @note Derived classes must implement BVH generation specific to their geometry
    virtual void CreateBVH();

    /// @}

private:
    //-------------------------------------------------------------------------
    /// @name Private Implementation
    //-------------------------------------------------------------------------
    /// @{

    /// @brief Assign primitive properties from scene delegate
    /// @param _sceneDelegate Scene data provider
    /// @param _renderParam Global render parameters
    /// @param _dirtyBits Bitmask of changed states
    void Assign(HdSceneDelegate* _sceneDelegate,
                HdRenderParam& _renderParam,
                const HdDirtyBits* _dirtyBits) const;

    /// @brief Synchronize all primvars
    /// @param _sceneDelegate Scene data provider
    /// @param _renderParam Global render parameters
    /// @param _dirtyBits Bitmask of changed states
    void SyncPrimvars(HdSceneDelegate* _sceneDelegate,
                      HdRenderParam& _renderParam,
                      const HdDirtyBits* _dirtyBits);

    // Instancing
    mutable bool m_instanced = false;             ///< Instancing flag
    mutable VtMatrix4dArray m_instanceTransforms; ///< Instance transforms

    /// @}
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif // HDGRAVI_HDGRAVIPRIM_H_