//
// Copyright 2025 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//

/// @file RenderPass.h
/// @brief Render pass implementation for Gravi renderer
///
/// Implements core rendering loop handling:
/// - Scene rendering via raycasting
/// - AOV management and output
/// - Progressive rendering convergence
/// - Camera/viewport transformations
/// - Scene version tracking
/// - Render state synchronization
///
/// @author Christopher Hosken
/// @version 1.0
/// @date 07/05/2025 Pre-release cleanup
/// @ingroup hdGravi

#ifndef HDGRAVI_HDGRAVIRENDERPASS_H_
#define HDGRAVI_HDGRAVIRENDERPASS_H_

#include <atomic>
#include <pxr/pxr.h>
#include <pxr/imaging/hd/aov.h>
#include <pxr/imaging/hd/renderPass.h>
#include <pxr/imaging/hd/renderThread.h>
#include <pxr/base/gf/matrix4d.h>
#include <pxr/base/gf/rect2i.h>

#include "Renderer.h"
#include "RenderBuffer.h"

PXR_NAMESPACE_OPEN_SCOPE

//-----------------------------------------------------------------------------
/// @class HdGraviRenderPass
/// @brief Hydra render pass implementation for Gravi's raycasting renderer
///
/// Handles single rendering pass that:
/// 1. Transforms scene data using view/projection matrices
/// 2. Raycasts into scene using Gravi's backend
/// 3. Writes results to AOV buffers
/// 4. Tracks rendering convergence state
/// 5. Manages scene version synchronization
///
/// @note Maintains strict synchronization with scene state through version tracking
//-----------------------------------------------------------------------------
class HdGraviRenderPass final : public HdRenderPass
{
public:
    //-------------------------------------------------------------------------
    /// @name Construction / Destruction
    //-------------------------------------------------------------------------
    /// @{

    /// @brief Construct a new render pass
    /// @param _index Render index containing scene data
    /// @param _collection Collection of primitives to render
    /// @param _renderThread Render thread for async operations
    /// @param _renderer Gravi renderer instance
    /// @param _sceneVersion Atomic counter tracking scene changes
    HdGraviRenderPass(HdRenderIndex* _index,
                     HdRprimCollection const& _collection,
                     HdRenderThread* _renderThread,
                     HdGraviRenderer* _renderer,
                     std::atomic<int>* _sceneVersion);

    /// @brief Destructor
    ///
    /// Cleans up render pass resources including allocated buffers
    ~HdGraviRenderPass() override;

    /// @}

    //-------------------------------------------------------------------------
    /// @name Render State Queries
    //-------------------------------------------------------------------------
    /// @{

    /// @brief Check render convergence state
    /// @return True if rendering is considered converged
    ///
    /// @note Used for progressive rendering
    bool IsConverged() const override;

    /// @}

protected:
    //-------------------------------------------------------------------------
    /// @name Rendering Operations
    //-------------------------------------------------------------------------
    /// @{

    /// @brief Execute the render pass
    /// @param _renderPassState Current render state including camera params
    /// @param _renderTags Which render tags should be included
    ///
    /// Performs core rendering operations:
    /// 1. Updates camera/viewport transformations
    /// 2. Synchronizes with scene changes
    /// 3. Executes raycasting
    /// 4. Writes results to AOV buffers
    void _Execute(HdRenderPassStateSharedPtr const& _renderPassState,
                 TfTokenVector const& _renderTags) override;

    /// @brief Mark collection as dirty (no-op implementation)
    ///
    /// @note Collection changes are handled during _Execute
    void _MarkCollectionDirty() override {}

    /// @}

private:
    //-------------------------------------------------------------------------
    /// @name Private Members
    //-------------------------------------------------------------------------
    /// @{

    HdGraviRenderer* m_renderer;          ///< Gravi renderer instance
    HdRenderThread* m_renderThread;       ///< Render thread for async ops

    std::atomic<int>* m_sceneVersion;     ///< Atomic scene change counter
    int m_lastSceneVersion = 0;           ///< Last scene version rendered
    int m_lastSettingsVersion = 0;        ///< Last settings version rendered

    HdGraviScene m_scene;                ///< Gravi scene representation
    GfRect2i m_dataWindow;               ///< Render target region (y-down)
    GfMatrix4d m_viewMatrix;             ///< World to camera space
    GfMatrix4d m_projMatrix;             ///< Camera to NDC space

    HdRenderPassAovBindingVector m_aovBindings;  ///< Configured AOV bindings
    HdGraviRenderBuffer m_colorBuffer;          ///< Default color buffer
    HdGraviRenderBuffer m_depthBuffer;          ///< Default depth buffer
    bool m_converged = false;                   ///< Convergence state flag

    /// @}
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif // HDGRAVI_HDGRAVIRENDERPASS_H_