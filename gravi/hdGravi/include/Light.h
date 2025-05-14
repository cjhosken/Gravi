//
// Copyright 2025 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//

/// @file Light.h
/// @brief Light implementation for Gravi renderer
///
/// Implements Hydra light primitives for the Gravi renderer, supporting:
/// - Multiple light types (dome, rect, distant, spot, etc.)
/// - Physical light properties (intensity, exposure, color)
/// - Geometric light properties (size, angle, radius)
/// - Transform hierarchy support
/// - Thread-safe property updates
///
/// @author Christopher Hosken
/// @version 1.0
/// @date 07/05/2025 Pre-release cleanup
/// @ingroup hdGravi

#ifndef HDGRAVI_HDGRAVILIGHT_H_
#define HDGRAVI_HDGRAVILIGHT_H_

#include <pxr/pxr.h>
#include <pxr/imaging/hd/light.h>
#include <pxr/imaging/hd/sprim.h>
#include <pxr/imaging/hd/types.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/matrix4d.h>
#include <pxr/usd/sdf/path.h>
#include <pxr/base/tf/token.h>

PXR_NAMESPACE_OPEN_SCOPE

//-----------------------------------------------------------------------------
/// @class HdGraviLight
/// @brief Hydra light implementation for Gravi renderer
///
/// Provides comprehensive light support including:
/// - Physical light properties (intensity, exposure, color)
/// - Geometric light properties (size, angle, radius)
/// - Transform hierarchy support
/// - Thread-safe property updates
/// - Multiple light type specializations
//-----------------------------------------------------------------------------
class HdGraviLight final : public HdLight
{
public:
    //-------------------------------------------------------------------------
    /// @name Construction / Destruction
    //-------------------------------------------------------------------------
    /// @{

    /// @brief Constructor
    /// @param _id Path identifier for the light
    /// @param _lightType Type of light (e.g., "dome", "rect", "distant")
    HdGraviLight(SdfPath const& _id, TfToken const& _lightType);

    /// @brief Destructor
    ~HdGraviLight() override = default;

    /// @}

    //-------------------------------------------------------------------------
    /// @name Hydra Light Interface
    //-------------------------------------------------------------------------
    /// @{

    /// @brief Synchronize light properties from scene delegate
    /// @param _sceneDelegate Scene data provider
    /// @param _renderParam Renderer parameters
    /// @param _dirtyBits Bitmask of changed properties
    ///
    /// @note Thread-safe for parallel execution
    void Sync(HdSceneDelegate* _sceneDelegate,
              HdRenderParam* _renderParam,
              HdDirtyBits* _dirtyBits) override;

    /// @brief Get initial dirty state mask
    /// @return Bitmask of initial dirty states
    HdDirtyBits GetInitialDirtyBitsMask() const override;

    /// @brief Finalize the light (cleanup resources)
    /// @param _renderParam Renderer parameters
    void Finalize(HdRenderParam* _renderParam) override {}

    /// @}

    //-------------------------------------------------------------------------
    /// @name Light Properties
    //-------------------------------------------------------------------------
    /// @{

    /// @brief Get the light type
    /// @return Light type token
    [[nodiscard]] TfToken GetLightType() const;

    /// @brief Get light intensity
    /// @return Current intensity value
    [[nodiscard]] float GetIntensity() const;

    /// @brief Get light exposure
    /// @return Current exposure value
    [[nodiscard]] float GetExposure() const;

    /// @brief Get light color
    /// @return RGB color vector
    [[nodiscard]] GfVec3f GetColor() const;

    /// @brief Get light transform
    /// @return Transformation matrix
    [[nodiscard]] GfMatrix4d GetTransform() const;

    /// @brief Get light angle (for spot lights)
    /// @return Angle in degrees
    [[nodiscard]] float GetAngle() const;

    /// @brief Get light width (for area lights)
    /// @return Width in world units
    [[nodiscard]] float GetWidth() const;

    /// @brief Get light height (for area lights)
    /// @return Height in world units
    [[nodiscard]] float GetHeight() const;

    /// @brief Get light radius (for sphere lights)
    /// @return Radius in world units
    [[nodiscard]] float GetRadius() const;

    /// @brief Get light length (for cylinder lights)
    /// @return Length in world units
    [[nodiscard]] float GetLength() const;

    /// @}

private:
    //-------------------------------------------------------------------------
    /// @name Private Members
    //-------------------------------------------------------------------------
    /// @{

    const TfToken m_lightType;   ///< Type identifier for the light

    // Basic light properties
    float m_intensity = 1.0f;    ///< Light intensity multiplier
    float m_exposure = 0.0f;     ///< Exposure compensation
    GfVec3f m_color = GfVec3f(1);///< Light color (linear RGB)
    GfMatrix4d m_transform;      ///< World transform

    // Geometric properties
    float m_height = 1.0f;       ///< Height for rectangular lights
    float m_width = 1.0f;        ///< Width for rectangular lights
    float m_angle = 45.0f;       ///< Angle for spot lights

    // Sphere/cylinder properties
    float m_radius = 0.5f;       ///< Radius for spherical lights
    float m_length = 1.0f;       ///< Length for cylindrical lights

    /// @}
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif // HDGRAVI_HDGRAVILIGHT_H_