//
// Copyright 2025 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//

/// @file RendererPlugin.h
/// @brief Hydra renderer plugin entry point for Gravi renderer.
///
/// Implements plugin interface for creating Gravi render delegate instances.
/// Handles:
/// - Render delegate instantiation
/// - System capability checks
/// - Plugin lifecycle management
///
/// @author Christopher Hosken
/// @version 1.0
/// @date 07/05/2025 Pre-release cleanup
/// @ingroup hdGravi

#ifndef HDGRAVI_RENDERERPLUGIN_H_
#define HDGRAVI_RENDERERPLUGIN_H_

#include <pxr/pxr.h>
#include <pxr/imaging/hd/rendererPlugin.h>

PXR_NAMESPACE_OPEN_SCOPE

//-----------------------------------------------------------------------------
/// @class HdGraviRendererPlugin
/// @brief Hydra renderer plugin implementation for Gravi.
///
/// Factory class for creating Gravi render delegate instances with:
/// - Render delegate lifecycle management
/// - System compatibility verification
/// - Plugin singleton pattern support
//-----------------------------------------------------------------------------
class HdGraviRendererPlugin final : public HdRendererPlugin
{
public:
    //-------------------------------------------------------------------------
    /// @name Construction/Destruction
    //-------------------------------------------------------------------------

    /// @brief Default constructor.
    ///
    /// Creates plugin instance with default configuration.
    HdGraviRendererPlugin() = default;

    /// @brief Destructor.
    ///
    /// @note Individual render delegates should be deleted via DeleteRenderDelegate() first.
    ~HdGraviRendererPlugin() override = default;

    //-------------------------------------------------------------------------
    /// @name Render Delegate Management
    //-------------------------------------------------------------------------

    /// @brief Create render delegate with default settings.
    /// @return Pointer to new render delegate instance.
    ///
    /// @note Each delegate maintains its own scene state - create new delegate per HdRenderIndex.
    HdRenderDelegate* CreateRenderDelegate() override;

    /// @brief Create render delegate with custom settings.
    /// @param _settingsMap Initial render settings.
    /// @return Pointer to new render delegate instance.
    HdRenderDelegate* CreateRenderDelegate(HdRenderSettingsMap const& _settingsMap) override;

    /// @brief Destroy a render delegate.
    /// @param _renderDelegate Delegate to destroy.
    void DeleteRenderDelegate(HdRenderDelegate* _renderDelegate) override;

    //-------------------------------------------------------------------------
    /// @name Capability Reporting
    //-------------------------------------------------------------------------

    /// @brief Check system support.
    /// @param _gpuEnabled Whether to consider GPU acceleration.
    /// @return True if Gravi is supported on this system.
    ///
    /// Verifies system requirements including:
    /// - CPU feature support
    /// - Memory requirements
    /// - GPU capabilities (if enabled)
    [[nodiscard]] bool IsSupported(bool _gpuEnabled) const override;
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif // HDGRAVI_RENDERERPLUGIN_H_