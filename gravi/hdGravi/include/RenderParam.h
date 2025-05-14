//
// Copyright 2025 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//

/// @file RenderParam.h
/// @brief Render parameter management for Gravi renderer
///
/// Maintains global render state shared across all scene primitives,
/// handling:
/// - Scene representation access
/// - Scene version tracking
/// - Render thread control
/// - Quality parameter management
///
/// @author Christopher Hosken
/// @version 1.0
/// @date 07/05/2025 Pre-release cleanup
/// @ingroup hdGravi

#ifndef HDGRAVI_HDGRAVIRENDERPARAM_H_
#define HDGRAVI_HDGRAVIRENDERPARAM_H_

#include <pxr/pxr.h>
#include <pxr/imaging/hd/renderDelegate.h>
#include <pxr/imaging/hd/renderThread.h>

#include "Scene.h"

PXR_NAMESPACE_OPEN_SCOPE

//-----------------------------------------------------------------------------
/// @class HdGraviRenderParam
/// @brief Container for Gravi's global render state
///
/// Manages access to renderer-wide state including:
/// - Main scene representation
/// - Scene version tracking
/// - Render thread control
///
/// @note One instance is created by render delegate and shared across primitives
//-----------------------------------------------------------------------------
class HdGraviRenderParam final : public HdRenderParam
{
public:
    //-------------------------------------------------------------------------
    /// @name Construction / Initialization
    //-------------------------------------------------------------------------
    /// @{

    /// @brief Constructs a render parameter container
    /// @param _renderThread Render thread to control
    /// @param _scene Gravi scene representation
    /// @param _sceneVersion Atomic counter for scene changes
    ///
    /// @note Does not take ownership of any parameters
    HdGraviRenderParam(HdRenderThread* _renderThread,
                      HdGraviScene* _scene,
                      std::atomic<int>* _sceneVersion)
        : m_renderThread(_renderThread)
        , m_scene(_scene)
        , m_sceneVersion(_sceneVersion)
    {}

    /// @}

    //-------------------------------------------------------------------------
    /// @name Scene Access
    //-------------------------------------------------------------------------
    /// @{

    /// @brief Get editable scene reference
    /// @return Pointer to scene representation
    ///
    /// Provides mutable access with proper synchronization:
    /// 1. Stops active rendering
    /// 2. Increments scene version
    /// 3. Returns scene pointer
    ///
    /// @warning Callers must ensure proper locking when modifying scene
    [[nodiscard]] HdGraviScene* Edit() const {
        m_renderThread->StopRender();
        ++(*m_sceneVersion);
        return m_scene;
    }

    /// @}

private:
    //-------------------------------------------------------------------------
    /// @name Private Members
    //-------------------------------------------------------------------------
    /// @{

    HdRenderThread* m_renderThread;      ///< Background rendering thread
    HdGraviScene* m_scene;              ///< Complete scene representation
    std::atomic<int>* m_sceneVersion;    ///< Scene modification counter

    /// @}
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif // HDGRAVI_HDGRAVIRENDERPARAM_H_