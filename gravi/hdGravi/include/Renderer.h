//
// Copyright 2025 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//

/// @file Renderer.h
/// @brief Core rendering implementation for Gravi renderer
///
/// Implements main rendering capabilities including:
/// - Ray generation and tracing
/// - AOV management and validation
/// - Progressive rendering with convergence control
/// - Camera/viewport transformations
/// - Tile-based rendering optimization
/// - Light transport simulation
///
/// @author Christopher Hosken
/// @version 1.0
/// @date 07/05/2025 Pre-release cleanup
/// @ingroup hdGravi

#ifndef HDGRAVI_HDGRAVIRENDERER_H_
#define HDGRAVI_HDGRAVIRENDERER_H_

#include <pxr/pxr.h>
#include <pxr/imaging/hd/renderThread.h>
#include <pxr/imaging/hd/renderPassState.h>
#include <pxr/base/gf/matrix4d.h>
#include <pxr/base/gf/rect2i.h>

#include "Scene.h"

PXR_NAMESPACE_OPEN_SCOPE

//-----------------------------------------------------------------------------
/// @class HdGraviRenderer
/// @brief Core raycasting renderer implementation for Gravi
///
/// Features include:
/// - Sequential ray generation and tracing
/// - Configurable path tracing with bounce control
/// - Progressive rendering with convergence control
/// - Comprehensive AOV output management
/// - Tile-based rendering optimization
/// - Light transport simulation
///
/// Maintains all rendering state including:
/// - Scene geometry (via HdGraviScene)
/// - Camera/viewport configuration
/// - Rendering parameters and quality settings
/// - Output buffer bindings and validation
/// - Light configuration and sampling
//-----------------------------------------------------------------------------
class HdGraviRenderer final
{
public:
    //-------------------------------------------------------------------------
    /// @name Construction / Destruction
    //-------------------------------------------------------------------------
    /// @{

    /// @brief Default constructor
    ///
    /// Initializes with default parameters:
    /// - 64 samples to convergence
    /// - 3 ray bounces
    /// - 32x32 tile size
    HdGraviRenderer();

    /// @brief Destructor
    ///
    /// Cleans up all renderer resources
    ~HdGraviRenderer() = default;

    /// @}

    //-------------------------------------------------------------------------
    /// @name Scene Configuration
    //-------------------------------------------------------------------------
    /// @{

    /// @brief Set the scene to render
    /// @param _scene Scene containing geometry and acceleration structures
    ///
    /// @note Scene must contain valid geometry and BVH before rendering
    void SetScene(const HdGraviScene& _scene);

    /// @brief Build the scene BVH
    ///
    /// Constructs bounding volume hierarchy for accelerated ray tracing
    /// Should be called after scene changes and before rendering
    void BuildBVH();

    /// @}

    //-------------------------------------------------------------------------
    /// @name Rendering Configuration
    //-------------------------------------------------------------------------
    /// @{

    /// @brief Set the data window (render region)
    /// @param _dataWindow Region to render in y-Down coordinates
    ///
    /// Defines renderable area in pixel coordinates following CameraUtilFraming
    void SetDataWindow(const GfRect2i& _dataWindow);

    /// @brief Set the camera transformation
    /// @param _viewMatrix World-to-camera transformation
    /// @param _projMatrix Camera-to-NDC projection
    ///
    /// Configures camera used for ray generation
    void SetCamera(const GfMatrix4d& _viewMatrix, const GfMatrix4d& _projMatrix);

    /// @brief Set AOV bindings
    /// @param _aovBindings List of output buffers to render into
    ///
    /// Configures rendering results storage. Supported AOVs:
    /// - Color
    /// - Depth
    /// - Normal
    /// Uses default color/depth buffers if none bound
    void SetAovBindings(HdRenderPassAovBindingVector const& _aovBindings);

    /// @brief Get current AOV bindings
    /// @return Const reference to current AOV bindings
    [[nodiscard]] HdRenderPassAovBindingVector const& GetAovBindings() const;

    /// @brief Set samples required for convergence
    /// @param _samplesToConvergence Samples per pixel needed
    ///
    /// Controls quality/performance tradeoff for progressive rendering
    void SetSamplesToConvergence(int _samplesToConvergence);

    /// @brief Set number of ray bounces
    /// @param _numberBounces Maximum ray bounce depth
    void SetNumberBounces(int _numberBounces);

    /// @brief Set light stepping parameters
    /// @param _size Initial light step size
    void SetLightStepSize(double _size);

    /// @brief Set minimum light step size
    /// @param _size Minimum step size for light sampling
    void SetMinLightStepSize(double _size);

    /// @brief Set maximum light step size
    /// @param _size Maximum step size for light sampling
    void SetMaxLightStepSize(double _size);

    /// @brief Set maximum light distance
    /// @param _dist Maximum distance for light sampling
    void SetMaxLightDistance(double _dist);

    /// @brief Set maximum light steps
    /// @param _steps Maximum number of light sampling steps
    void SetMaxLightSteps(int _steps);

    /// @}

    //-------------------------------------------------------------------------
    /// @name Rendering Operations
    //-------------------------------------------------------------------------
    /// @{

    /// @brief Execute the rendering process
    /// @param _renderThread Render thread for synchronization
    ///
    /// Performs progressive rendering by:
    /// 1. Generating camera rays
    /// 2. Tracing rays through scene
    /// 3. Accumulating results
    /// 4. Repeating until convergence/cancellation
    ///
    /// @note Checks for thread cancellation between passes
    void Render(HdRenderThread* _renderThread);

    /// @brief Clear output buffers
    ///
    /// Resets all bound AOV buffers to their clear values
    void Clear();

    /// @brief Mark AOV buffers as unconverged
    ///
    /// Forces additional rendering passes when scene/camera changes
    void MarkAovBuffersUnconverged();

    /// @brief Get completed sample count
    /// @return Number of samples rendered so far
    [[nodiscard]] int GetCompletedSamples() const;

    /// @brief Add light to scene
    /// @param _light Light to add
    void AddLight(const HdGraviLight* _light);

    /// @brief Update light configuration in scene
    void UpdateLights();

    /// @}

private:
    //-------------------------------------------------------------------------
    /// @name Private Implementation
    //-------------------------------------------------------------------------
    /// @{

    /// @brief Validate AOV binding configuration
    /// @return True if bindings are valid for rendering
    ///
    /// Checks that bound AOVs have:
    /// - Supported formats
    /// - Valid render buffers
    /// - Consistent dimensions
    bool _ValidateAovBindings();

    /// @brief Get clear color from VtValue
    /// @param _clearValue Input clear value
    /// @return Resolved clear color
    static GfVec4f _GetClearColor(VtValue const& _clearValue);

    /// @}

    //-------------------------------------------------------------------------
    /// @name Private Members
    //-------------------------------------------------------------------------
    /// @{

    HdRenderPassAovBindingVector m_aovBindings;      ///< Current AOV bindings
    HdParsedAovTokenVector m_aovNames;               ///< Parsed AOV name tokens
    HdGraviScene m_scene;                           ///< Scene being rendered

    GfRect2i m_dataWindow;                          ///< Render region
    int m_width = 0;                                ///< Render buffer width
    int m_height = 0;                               ///< Render buffer height

    GfMatrix4d m_viewMatrix;                        ///< World-to-camera
    GfMatrix4d m_projMatrix;                        ///< Camera-to-NDC
    GfMatrix4d m_inverseViewMatrix;                 ///< Camera-to-world
    GfMatrix4d m_inverseProjMatrix;                 ///< NDC-to-camera

    int m_samplesToConvergence = 64;                ///< Target samples/pixel
    int m_tileSize = 32;                            ///< Rendering tile size
    int m_numBounces = 3;                           ///< Max ray bounces
    double m_lightStepSize = 0.1;                   ///< Light sampling step size
    double m_maxLightStepSize = 1.0;                ///< Max light step size
    double m_minLightStepSize = 0.01;               ///< Min light step size
    double m_maxLightDistance = 100.0;              ///< Max light distance
    int m_maxLightSteps = 100;                      ///< Max light steps

    std::atomic<int> m_completedSamples{0};         ///< Progressive render state
    std::vector<const HdGraviLight*> m_lights;      ///< Scene lights

    bool m_aovBindingsNeedValidation = false;       ///< AOV validation flag
    bool m_aovBindingsValid = false;                ///< Last validation result

    /// @}
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif // HDGRAVI_HDGRAVIRENDERER_H_