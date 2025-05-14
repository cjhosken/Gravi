//
// Copyright 2025 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//

/// @file RenderSettings.h
/// @brief Render settings implementation for Gravi renderer
///
/// Implements Hydra render settings support for Gravi, handling:
/// - Render setting synchronization
/// - Dirty state management
/// - Default value initialization
/// - Quality parameter control
/// - Output configuration
///
/// @author Christopher Hosken
/// @version 1.0
/// @date 07/05/2025 Pre-release cleanup
/// @ingroup hdGravi

#ifndef HDGRAVI_HDGRAVIRENDERSETTINGS_H_
#define HDGRAVI_HDGRAVIRENDERSETTINGS_H_

#include <pxr/imaging/hd/renderSettings.h>

PXR_NAMESPACE_OPEN_SCOPE

//-----------------------------------------------------------------------------
/// @def HDGRAVI_RENDER_SETTINGS_TOKENS
/// @brief Tokens for Gravi-specific render settings
///
/// Extends Hydra render settings with Gravi-specific controls:
/// - bounces: Maximum ray bounce depth
/// - endFrame: Final frame for animation rendering
/// - outputPath: Render output file path pattern
/// - lightStepSize: Initial light sampling step size
/// - minLightStepSize: Minimum light sampling step size
/// - maxLightStepSize: Maximum light sampling step size
/// - maxLightDistance: Maximum light sampling distance
/// - maxLightSteps: Maximum number of light sampling steps
//-----------------------------------------------------------------------------
#define HDGRAVI_RENDER_SETTINGS_TOKENS \
    (bounces) \
    (endFrame) \
    (outputPath) \
    (lightStepSize) \
    (minLightStepSize) \
    (maxLightStepSize) \
    (maxLightDistance) \
    (maxLightSteps)

/// Default render quality values
static const int defaultConvergedSamplesPerPixel = 16;  ///< Default samples per pixel
static const int defaultBounces = 4;                    ///< Default ray bounce count
static const int defaultStartFrame = 1;                 ///< Default start frame
static const int defaultEndFrame = 240;                 ///< Default end frame

/// Default light sampling parameters
static const double defaultLightStepSize = 0.01;        ///< Default light step size
static const double defaultMinLightStepSize = 0.001;    ///< Default min light step size
static const double defaultMaxLightStepSize = 1.0;      ///< Default max light step size
static const double defaultMaxLightDistance = 10000;    ///< Default max light distance
static const int defaultMaxLightSteps = 1000;           ///< Default max light steps
static const std::string defaultOutputPath = "./render_####.png"; ///< Default output path

TF_DEFINE_PRIVATE_TOKENS(HdGraviRenderSettingsTokens, HDGRAVI_RENDER_SETTINGS_TOKENS);

//-----------------------------------------------------------------------------
/// @class HdGraviRenderSettings
/// @brief Hydra render settings implementation for Gravi
///
/// Manages render settings synchronization between Hydra and Gravi renderer,
/// including:
/// - Quality parameters (bounces, samples)
/// - Light sampling configuration
/// - Output path specification
/// - Frame range settings
/// - Dirty state tracking
//-----------------------------------------------------------------------------
class HdGraviRenderSettings final : public HdRenderSettings
{
public:
    //-------------------------------------------------------------------------
    /// @name Construction / Destruction
    //-------------------------------------------------------------------------
    /// @{

    /// @brief Constructor
    /// @param _id Path identifier for these settings
    explicit HdGraviRenderSettings(SdfPath const& _id) : HdRenderSettings(_id) {}

    /// @}

    //-------------------------------------------------------------------------
    /// @name Hydra Interface
    //-------------------------------------------------------------------------
    /// @{

    /// @brief Get initial dirty state mask
    /// @return Bitmask of all possible dirty states
    HdDirtyBits GetInitialDirtyBitsMask() const override;

    /// @}

protected:
    //-------------------------------------------------------------------------
    /// @name Synchronization
    //-------------------------------------------------------------------------
    /// @{

    /// @brief Synchronize settings from scene delegate
    /// @param _sceneDelegate Scene data provider
    /// @param _renderParam Global render parameters
    /// @param _dirtyBits Bitmask of changed states
    void _Sync(HdSceneDelegate* _sceneDelegate,
              HdRenderParam* _renderParam,
              const HdDirtyBits* _dirtyBits) override;

    /// @}
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif // HDGRAVI_HDGRAVIRENDERSETTINGS_H_