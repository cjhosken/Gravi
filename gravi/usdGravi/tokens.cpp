//
// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
#include "./tokens.h"

PXR_NAMESPACE_OPEN_SCOPE

TokensType::TokensType() :
    bounces("bounces", TfToken::Immortal),
    convergedSamplesPerPixel("convergedSamplesPerPixel", TfToken::Immortal),
    force("force", TfToken::Immortal),
    frameRange("frameRange", TfToken::Immortal),
    lightStepSize("lightStepSize", TfToken::Immortal),
    maxLightDistance("maxLightDistance", TfToken::Immortal),
    maxLightSteps("maxLightSteps", TfToken::Immortal),
    maxLightStepSize("maxLightStepSize", TfToken::Immortal),
    minLightStepSize("minLightStepSize", TfToken::Immortal),
    outputPath("outputPath", TfToken::Immortal),
    GraviRenderSettings("GraviRenderSettings", TfToken::Immortal),
    GravityWell("GravityWell", TfToken::Immortal),
    allTokens({
        bounces,
        convergedSamplesPerPixel,
        force,
        frameRange,
        lightStepSize,
        maxLightDistance,
        maxLightSteps,
        maxLightStepSize,
        minLightStepSize,
        outputPath,
        GraviRenderSettings,
        GravityWell
    })
{
}

TfStaticData<TokensType> Tokens;

PXR_NAMESPACE_CLOSE_SCOPE
