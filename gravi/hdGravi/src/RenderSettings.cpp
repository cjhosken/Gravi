///
/// @file RenderSettings.cpp
/// @brief Render setting support for USD scene data.

#include <pxr/imaging/hd/sceneDelegate.h>

#include "RenderDelegate.h"

#include "RenderSettings.h"

PXR_NAMESPACE_OPEN_SCOPE

//-------------------------------------------------------------------------
/// @name Hydra Interface
//-------------------------------------------------------------------------

HdDirtyBits HdGraviRenderSettings::GetInitialDirtyBitsMask() const { return AllDirty; }

//-------------------------------------------------------------------------
/// @name Synchronization
//-------------------------------------------------------------------------

void HdGraviRenderSettings::_Sync(HdSceneDelegate* _sceneDelegate,
                                HdRenderParam* _renderParam,
                                const HdDirtyBits* _dirtyBits)
{
    SdfPath const& id = GetId();
    HdRenderDelegate* renderDelegate = _sceneDelegate->GetRenderIndex().GetRenderDelegate();
    auto* graviDelegate = static_cast<HdGraviRenderDelegate*>(renderDelegate);

    if (*_dirtyBits && AllDirty) {
        // Handle bounces
        VtValue bounces = _sceneDelegate->Get(id, HdGraviRenderSettingsTokens->bounces);
        if (!bounces.IsEmpty()) {
            graviDelegate->GetRenderer()->SetNumberBounces(bounces.UncheckedGet<int>());
        } else {
            graviDelegate->GetRenderer()->SetNumberBounces(graviDelegate->GetRenderSetting(HdGraviRenderSettingsTokens->bounces).Get<int>());
        }

        // Handle samples
        VtValue samples = _sceneDelegate->Get(id, HdRenderSettingsTokens->convergedSamplesPerPixel);
        if (!samples.IsEmpty()) {
            graviDelegate->GetRenderer()->SetSamplesToConvergence(samples.UncheckedGet<int>());
        } else {
            graviDelegate->GetRenderer()->SetSamplesToConvergence(graviDelegate->GetRenderSetting(HdRenderSettingsTokens->convergedSamplesPerPixel).Get<int>());
        }

        // Handle light stepping parameters
        VtValue lightStepSize = _sceneDelegate->Get(id, HdGraviRenderSettingsTokens->lightStepSize);
        if (!lightStepSize.IsEmpty()) {
            graviDelegate->GetRenderer()->SetLightStepSize(lightStepSize.UncheckedGet<double>());
        } else {
            graviDelegate->GetRenderer()->SetLightStepSize(graviDelegate->GetRenderSetting(HdGraviRenderSettingsTokens->lightStepSize).Get<double>());
        }

        VtValue minLightStepSize = _sceneDelegate->Get(id, HdGraviRenderSettingsTokens->minLightStepSize);
        if (!minLightStepSize.IsEmpty()) {
            graviDelegate->GetRenderer()->SetMinLightStepSize(minLightStepSize.UncheckedGet<double>());
        } else {
            graviDelegate->GetRenderer()->SetMinLightStepSize(graviDelegate->GetRenderSetting(HdGraviRenderSettingsTokens->minLightStepSize).Get<double>());
        }

        VtValue maxLightStepSize = _sceneDelegate->Get(id, HdGraviRenderSettingsTokens->maxLightStepSize);
        if (!maxLightStepSize.IsEmpty()) {
            graviDelegate->GetRenderer()->SetMaxLightStepSize(maxLightStepSize.UncheckedGet<double>());
        } else {
            graviDelegate->GetRenderer()->SetMaxLightStepSize(graviDelegate->GetRenderSetting(HdGraviRenderSettingsTokens->maxLightStepSize).Get<double>());
        }

        VtValue maxLightDistance = _sceneDelegate->Get(id, HdGraviRenderSettingsTokens->maxLightDistance);
        if (!maxLightDistance.IsEmpty()) {
            graviDelegate->GetRenderer()->SetMaxLightDistance(maxLightDistance.UncheckedGet<double>());
        } else {
            graviDelegate->GetRenderer()->SetMaxLightDistance(graviDelegate->GetRenderSetting(HdGraviRenderSettingsTokens->maxLightDistance).Get<double>());
        }

        VtValue maxLightSteps = _sceneDelegate->Get(id, HdGraviRenderSettingsTokens->maxLightSteps);
        if (!maxLightSteps.IsEmpty()) {
            graviDelegate->GetRenderer()->SetMaxLightSteps(maxLightSteps.UncheckedGet<int>());
        } else {
            graviDelegate->GetRenderer()->SetMaxLightSteps(graviDelegate->GetRenderSetting(HdGraviRenderSettingsTokens->maxLightSteps).Get<int>());
        }
    }
}

PXR_NAMESPACE_CLOSE_SCOPE