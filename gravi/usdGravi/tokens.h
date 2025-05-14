//
// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
#ifndef _TOKENS_H
#define _TOKENS_H

/// \file GRAVI/tokens.h

// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
// 
// This is an automatically generated file (by usdGenSchema.py).
// Do not hand-edit!
// 
// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

#include "pxr/pxr.h"
#include "./api.h"
#include "pxr/base/tf/staticData.h"
#include "pxr/base/tf/token.h"
#include <vector>

PXR_NAMESPACE_OPEN_SCOPE


/// \class TokensType
///
/// \link Tokens \endlink provides static, efficient
/// \link TfToken TfTokens\endlink for use in all public USD API.
///
/// These tokens are auto-generated from the module's schema, representing
/// property names, for when you need to fetch an attribute or relationship
/// directly by name, e.g. UsdPrim::GetAttribute(), in the most efficient
/// manner, and allow the compiler to verify that you spelled the name
/// correctly.
///
/// Tokens also contains all of the \em allowedTokens values
/// declared for schema builtin attributes of 'token' scene description type.
/// Use Tokens like so:
///
/// \code
///     gprim.GetMyTokenValuedAttr().Set(Tokens->bounces);
/// \endcode
struct TokensType {
    GRAVI_API TokensType();
    /// \brief "bounces"
    /// 
    /// GraviRenderSettings
    const TfToken bounces;
    /// \brief "convergedSamplesPerPixel"
    /// 
    /// GraviRenderSettings
    const TfToken convergedSamplesPerPixel;
    /// \brief "force"
    /// 
    /// GravityWell
    const TfToken force;
    /// \brief "frameRange"
    /// 
    /// GraviRenderSettings
    const TfToken frameRange;
    /// \brief "lightStepSize"
    /// 
    /// GraviRenderSettings
    const TfToken lightStepSize;
    /// \brief "maxLightDistance"
    /// 
    /// GraviRenderSettings
    const TfToken maxLightDistance;
    /// \brief "maxLightSteps"
    /// 
    /// GraviRenderSettings
    const TfToken maxLightSteps;
    /// \brief "maxLightStepSize"
    /// 
    /// GraviRenderSettings
    const TfToken maxLightStepSize;
    /// \brief "minLightStepSize"
    /// 
    /// GraviRenderSettings
    const TfToken minLightStepSize;
    /// \brief "outputPath"
    /// 
    /// GraviRenderSettings
    const TfToken outputPath;
    /// \brief "GraviRenderSettings"
    /// 
    /// Schema identifer and family for GraviRenderSettings
    const TfToken GraviRenderSettings;
    /// \brief "GravityWell"
    /// 
    /// Schema identifer and family for GravityWell
    const TfToken GravityWell;
    /// A vector of all of the tokens listed above.
    const std::vector<TfToken> allTokens;
};

/// \var Tokens
///
/// A global variable with static, efficient \link TfToken TfTokens\endlink
/// for use in all public USD API.  \sa TokensType
extern GRAVI_API TfStaticData<TokensType> Tokens;

PXR_NAMESPACE_CLOSE_SCOPE

#endif
