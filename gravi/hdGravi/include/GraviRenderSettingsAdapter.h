//
// Copyright 2025 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//

/// @file GraviRenderSettingsAdapter.h
/// @brief USD Imaging Adapter for Gravi render settings
///
/// This adapter provides USD imaging support for Gravi-specific render settings,
/// handling:
/// - Render settings population
/// - Property value resolution
/// - Scene index management
///
/// @author Christopher Hosken
/// @version 1.0
/// @date 07/05/2025 Pre-release cleanup
/// @ingroup hdGravi

#ifndef HDGRAVI_GRAVIRENDERSETTINGSADAPTER_H_
#define HDGRAVI_GRAVIRENDERSETTINGSADAPTER_H_

#include <pxr/pxr.h>
#include <pxr/usdImaging/usdImaging/primAdapter.h>
#include <pxr/usdImaging/usdImaging/renderSettingsAdapter.h>

#include "RenderSettings.h"

PXR_NAMESPACE_OPEN_SCOPE

//-----------------------------------------------------------------------------
/// @class HdGraviRenderSettingsAdapter
/// @brief USD Imaging Adapter for Gravi render settings
///
/// Inherits from UsdImagingRenderSettingsAdapter to provide Gravi-specific:
/// - Render settings population
/// - Thread-safe property value resolution
/// - Integration with Gravi's render settings system
//-----------------------------------------------------------------------------
class HdGraviRenderSettingsAdapter : public UsdImagingRenderSettingsAdapter
{
public:
    using BaseClass = UsdImagingRenderSettingsAdapter;

    //-------------------------------------------------------------------------
    /// @name Construction / Destruction
    //-------------------------------------------------------------------------
    /// @{

    /// @brief Default constructor
    HdGraviRenderSettingsAdapter() = default;

    /// @brief Destructor
    ~HdGraviRenderSettingsAdapter() override = default;

    /// @}

    //-------------------------------------------------------------------------
    /// @name Primitive Interface
    //-------------------------------------------------------------------------
    /// @{

    /// @brief Get property value for the specified prim
    /// @param _prim The USD prim to query
    /// @param _cachePath Path to the prim in the render index
    /// @param _key Name of the property to fetch
    /// @param _time Time code for evaluation
    /// @param _outIndices Optional output for indices (for indexed attributes)
    /// @return The requested property value
    VtValue Get(UsdPrim const& _prim,
                SdfPath const& _cachePath,
                TfToken const& _key,
                UsdTimeCode _time,
                VtIntArray* _outIndices) const override;

    /// @brief Check if this adapter supports the given prim type
    /// @param _index The index proxy to check against
    /// @return True if the prim type is supported
    bool IsSupported(UsdImagingIndexProxy const* _index) const override;

    /// @brief Populate the prim in the render index
    /// @param _prim The USD prim to populate
    /// @param _index The render index proxy
    /// @param _instancerContext Optional instancer context
    /// @return The populated path in the render index
    SdfPath Populate(UsdPrim const& _prim,
                     UsdImagingIndexProxy* _index,
                     UsdImagingInstancerContext const* _instancerContext) override;

    /// @}
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif // HDGRAVI_GRAVIRENDERSETTINGSADAPTER_H_