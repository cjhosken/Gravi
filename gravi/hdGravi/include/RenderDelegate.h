//
// Copyright 2025 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//

/// @file RenderDelegate.h
/// @brief Primary Hydra render delegate interface for Gravi renderer
///
/// Implements the main Hydra interface for Gravi, handling:
/// - Scene primitive lifecycle management (Rprims, Sprims, Bprims)
/// - Render settings configuration and validation
/// - Resource allocation and management
/// - Render pass creation and scheduling
/// - Thread management and synchronization
/// - Render statistics collection
///
/// @author Christopher Hosken
/// @version 1.0
/// @date 07/05/2025 Pre-release cleanup
/// @ingroup hdGravi

#ifndef HDGRAVI_HDGRAVIRENDERDELEGATE_H_
#define HDGRAVI_HDGRAVIRENDERDELEGATE_H_

#include <pxr/pxr.h>
#include <pxr/imaging/hd/renderDelegate.h>
#include <pxr/imaging/hd/renderThread.h>

#include "Renderer.h"
#include "RenderParam.h"

PXR_NAMESPACE_OPEN_SCOPE

//-----------------------------------------------------------------------------
/// @class HdGraviRenderDelegate
/// @brief Hydra render delegate implementation for Gravi renderer
///
/// Serves as main interface between Hydra and Gravi, managing:
/// - Complete primitive lifecycle (creation, sync, destruction)
/// - Render state and settings management
/// - Resource allocation and sharing
/// - Render pass creation and execution
/// - Background rendering threads
/// - Scene version tracking
//-----------------------------------------------------------------------------
class HdGraviRenderDelegate final : public HdRenderDelegate
{
public:
    //-------------------------------------------------------------------------
    /// @name Construction / Destruction
    //-------------------------------------------------------------------------
    /// @{

    /// @brief Default constructor
    ///
    /// Initializes with default settings and creates:
    /// - Gravi renderer and scene state
    /// - Error handling systems
    /// - Basic render state configuration
    HdGraviRenderDelegate();

    /// @brief Constructor with initial settings
    /// @param _settingsMap Initial render settings map
    ///
    /// Initializes with specified settings while creating:
    /// - Gravi renderer and scene
    /// - Error handling linkage
    /// - Default render state
    explicit HdGraviRenderDelegate(HdRenderSettingsMap const& _settingsMap);

    /// @brief Destructor
    ///
    /// Cleans up all renderer resources including:
    /// - Gravi renderer state
    /// - Render thread
    /// - All allocated primitives
    ~HdGraviRenderDelegate() override;

    /// @}

    //-------------------------------------------------------------------------
    /// @name Primitive Support Queries
    //-------------------------------------------------------------------------
    /// @{

    /// @brief Get supported Rprim types
    /// @return Const reference to supported Rprim type tokens
    [[nodiscard]] const TfTokenVector& GetSupportedRprimTypes() const override;

    /// @brief Get supported Sprim types
    /// @return Const reference to supported Sprim type tokens
    [[nodiscard]] const TfTokenVector& GetSupportedSprimTypes() const override;

    /// @brief Get supported Bprim types
    /// @return Const reference to supported Bprim type tokens
    [[nodiscard]] const TfTokenVector& GetSupportedBprimTypes() const override;

    /// @}

    //-------------------------------------------------------------------------
    /// @name Resource Management
    //-------------------------------------------------------------------------
    /// @{

    /// @brief Get render parameter object
    /// @return Pointer to shared render param instance
    [[nodiscard]] HdRenderParam* GetRenderParam() const override;

    /// @brief Get resource registry
    /// @return Shared pointer to resource registry
    [[nodiscard]] HdResourceRegistrySharedPtr GetResourceRegistry() const override;

    /// @}

    //-------------------------------------------------------------------------
    /// @name Render Settings
    //-------------------------------------------------------------------------
    /// @{

    /// @brief Get render setting descriptors
    /// @return List of render setting descriptors
    [[nodiscard]] HdRenderSettingDescriptorList GetRenderSettingDescriptors() const override;

    /// @}

    //-------------------------------------------------------------------------
    /// @name Rendering Control
    //-------------------------------------------------------------------------
    /// @{

    /// @brief Check if pause/resume is supported
    /// @return Always true for Gravi
    [[nodiscard]] bool IsPauseSupported() const override;

    /// @brief Get the core renderer implementation
    /// @return Pointer to the renderer instance
    [[nodiscard]] HdGraviRenderer* GetRenderer() const;

    /// @brief Pause rendering threads
    /// @return True if successful
    bool Pause() override;

    /// @brief Resume rendering threads
    /// @return True if successful
    bool Resume() override;

    /// @}

    //-------------------------------------------------------------------------
    /// @name Primitive Management
    //-------------------------------------------------------------------------
    /// @{

    /// @brief Create render pass
    /// @param _index Render index to bind
    /// @param _collection Primitives collection to render
    /// @return Shared pointer to new render pass
    HdRenderPassSharedPtr CreateRenderPass(HdRenderIndex* _index,
                                         HdRprimCollection const& _collection) override;

    /// @brief Create instancer
    /// @param _delegate Scene delegate providing data
    /// @param _id Instancer's scene path
    /// @return Pointer to new instancer
    HdInstancer* CreateInstancer(HdSceneDelegate* _delegate,
                                SdfPath const& _id) override;

    /// @brief Destroy instancer
    /// @param _instancer Instancer to destroy
    void DestroyInstancer(HdInstancer* _instancer) override;

    /// @brief Create Rprim
    /// @param _typeId Type of Rprim to create
    /// @param _rprimId Scene path for new Rprim
    /// @return Pointer to new Rprim
    HdRprim* CreateRprim(TfToken const& _typeId,
                        SdfPath const& _rprimId) override;

    /// @brief Destroy Rprim
    /// @param _rPrim Rprim to destroy
    void DestroyRprim(HdRprim* _rPrim) override;

    /// @brief Create Sprim
    /// @param _typeId Type of Sprim to create
    /// @param _sprimId Scene path for new Sprim
    /// @return Pointer to new Sprim
    HdSprim* CreateSprim(TfToken const& _typeId,
                        SdfPath const& _sprimId) override;

    /// @brief Create fallback Sprim
    /// @param _typeId Type of Sprim to create
    /// @return Pointer to new Sprim
    HdSprim* CreateFallbackSprim(TfToken const& _typeId) override;

    /// @brief Destroy Sprim
    /// @param _sPrim Sprim to destroy
    void DestroySprim(HdSprim* _sPrim) override;

    /// @brief Create Bprim
    /// @param _typeId Type of Bprim to create
    /// @param _bprimId Scene path for new Bprim
    /// @return Pointer to new Bprim
    HdBprim* CreateBprim(TfToken const& _typeId,
                        SdfPath const& _bprimId) override;

    /// @brief Create fallback Bprim
    /// @param _typeId Type of Bprim to create
    /// @return Pointer to new Bprim
    HdBprim* CreateFallbackBprim(TfToken const& _typeId) override;

    /// @brief Destroy Bprim
    /// @param _bPrim Bprim to destroy
    void DestroyBprim(HdBprim* _bPrim) override;

    /// @}

    //-------------------------------------------------------------------------
    /// @name Rendering Operations
    //-------------------------------------------------------------------------
    /// @{

    /// @brief Commit resources to renderer
    /// @param _tracker Change tracker tracking dirty states
    void CommitResources(HdChangeTracker* _tracker) override {};

    /// @brief Get material binding purpose
    /// @return Always returns "full" material binding
    [[nodiscard]] TfToken GetMaterialBindingPurpose() const override;

    /// @brief Get default AOV descriptor
    /// @param _name Name of the AOV
    /// @return Descriptor for requested AOV
    [[nodiscard]] HdAovDescriptor GetDefaultAovDescriptor(TfToken const& _name) const override;

    /// @brief Get render statistics
    /// @return Dictionary of current render statistics
    [[nodiscard]] VtDictionary GetRenderStats() const override;

    /// @}

private:
    //-------------------------------------------------------------------------
    /// @name Private Implementation
    //-------------------------------------------------------------------------
    /// @{

    /// @brief Initialize render delegate
    void _Initialize();

    /// @}

    //-------------------------------------------------------------------------
    /// @name Static Members
    //-------------------------------------------------------------------------
    /// @{

    static const TfTokenVector s_SUPPORTED_RPRIM_TYPES;  ///< Supported Rprim types
    static const TfTokenVector s_SUPPORTED_SPRIM_TYPES;  ///< Supported Sprim types
    static const TfTokenVector s_SUPPORTED_BPRIM_TYPES;  ///< Supported Bprim types

    static std::mutex s_mutexResourceRegistry;           ///< Resource registry mutex
    static std::atomic_int s_counterResourceRegistry;    ///< Registry instance counter
    static HdResourceRegistrySharedPtr s_resourceRegistry; ///< Shared resource registry

    /// @}

    //-------------------------------------------------------------------------
    /// @name Instance Members
    //-------------------------------------------------------------------------
    /// @{

    std::shared_ptr<HdGraviRenderParam> m_renderParam;   ///< Render parameter object
    HdGraviRenderer* m_renderer = nullptr;              ///< Core renderer implementation
    HdRenderThread m_renderThread;                      ///< Background render thread
    std::atomic<int> m_sceneVersion{0};                 ///< Scene version counter
    HdRenderSettingDescriptorList m_settingDescriptors; ///< Render setting descriptors

    /// @}

    //-------------------------------------------------------------------------
    /// @name Deleted Methods
    //-------------------------------------------------------------------------
    /// @{

    HdGraviRenderDelegate(const HdGraviRenderDelegate&) = delete;
    HdGraviRenderDelegate& operator=(const HdGraviRenderDelegate&) = delete;

    /// @}
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif // HDGRAVI_HDGRAVIRENDERDELEGATE_H_