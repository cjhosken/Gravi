///
/// @file RendererPlugin.cpp
/// @brief This is the entry point to the hdGravi rendering plugin.

#include <pxr/imaging/hd/rendererPluginRegistry.h>
#include <pxr/base/tf/registryManager.h>
#include <pxr/imaging/hd/types.h>
#include <pxr/usd/usd/schemaRegistry.h>
#include <pxr/usd/usd/primDefinition.h>
#include <pxr/usdImaging/usdImaging/delegate.h>

#include "RenderDelegate.h"

#include "RendererPlugin.h"

PXR_NAMESPACE_OPEN_SCOPE

TF_REGISTRY_FUNCTION(TfType) { HdRendererPluginRegistry::Define<HdGraviRendererPlugin>(); }

//-------------------------------------------------------------------------
// Render Delegate Management
//-------------------------------------------------------------------------

HdRenderDelegate* HdGraviRendererPlugin::CreateRenderDelegate() { return new HdGraviRenderDelegate(); }

HdRenderDelegate* HdGraviRendererPlugin::CreateRenderDelegate(HdRenderSettingsMap const& _settingsMap) { return new HdGraviRenderDelegate(_settingsMap); }

void HdGraviRendererPlugin::DeleteRenderDelegate(HdRenderDelegate* _renderDelegate) { delete _renderDelegate; }

//-------------------------------------------------------------------------
// Capability Reporting
//-------------------------------------------------------------------------

bool HdGraviRendererPlugin::IsSupported(bool) const { return true; }

PXR_NAMESPACE_CLOSE_SCOPE