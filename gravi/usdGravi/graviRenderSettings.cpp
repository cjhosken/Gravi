//
// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
#include "./graviRenderSettings.h"
#include "pxr/usd/usd/schemaRegistry.h"
#include "pxr/usd/usd/typed.h"

#include "pxr/usd/sdf/types.h"
#include "pxr/usd/sdf/assetPath.h"

PXR_NAMESPACE_OPEN_SCOPE

// Register the schema with the TfType system.
TF_REGISTRY_FUNCTION(TfType)
{
    TfType::Define<GraviRenderSettings,
        TfType::Bases< UsdRenderSettings > >();
    
    // Skip registering an alias as it would be the same as the class name.
}

/* virtual */
GraviRenderSettings::~GraviRenderSettings()
{
}

/* static */
GraviRenderSettings
GraviRenderSettings::Get(const UsdStagePtr &stage, const SdfPath &path)
{
    if (!stage) {
        TF_CODING_ERROR("Invalid stage");
        return GraviRenderSettings();
    }
    return GraviRenderSettings(stage->GetPrimAtPath(path));
}

/* static */
GraviRenderSettings
GraviRenderSettings::Define(
    const UsdStagePtr &stage, const SdfPath &path)
{
    static TfToken usdPrimTypeName("GraviRenderSettings");
    if (!stage) {
        TF_CODING_ERROR("Invalid stage");
        return GraviRenderSettings();
    }
    return GraviRenderSettings(
        stage->DefinePrim(path, usdPrimTypeName));
}

/* virtual */
UsdSchemaKind GraviRenderSettings::_GetSchemaKind() const
{
    return GraviRenderSettings::schemaKind;
}

/* static */
const TfType &
GraviRenderSettings::_GetStaticTfType()
{
    static TfType tfType = TfType::Find<GraviRenderSettings>();
    return tfType;
}

/* static */
bool 
GraviRenderSettings::_IsTypedSchema()
{
    static bool isTyped = _GetStaticTfType().IsA<UsdTyped>();
    return isTyped;
}

/* virtual */
const TfType &
GraviRenderSettings::_GetTfType() const
{
    return _GetStaticTfType();
}

UsdAttribute
GraviRenderSettings::GetConvergedSamplesPerPixelAttr() const
{
    return GetPrim().GetAttribute(Tokens->convergedSamplesPerPixel);
}

UsdAttribute
GraviRenderSettings::CreateConvergedSamplesPerPixelAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(Tokens->convergedSamplesPerPixel,
                       SdfValueTypeNames->Int,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
GraviRenderSettings::GetBouncesAttr() const
{
    return GetPrim().GetAttribute(Tokens->bounces);
}

UsdAttribute
GraviRenderSettings::CreateBouncesAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(Tokens->bounces,
                       SdfValueTypeNames->Int,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
GraviRenderSettings::GetFrameRangeAttr() const
{
    return GetPrim().GetAttribute(Tokens->frameRange);
}

UsdAttribute
GraviRenderSettings::CreateFrameRangeAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(Tokens->frameRange,
                       SdfValueTypeNames->Int2,
                       /* custom = */ false,
                       SdfVariabilityUniform,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
GraviRenderSettings::GetLightStepSizeAttr() const
{
    return GetPrim().GetAttribute(Tokens->lightStepSize);
}

UsdAttribute
GraviRenderSettings::CreateLightStepSizeAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(Tokens->lightStepSize,
                       SdfValueTypeNames->Double,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
GraviRenderSettings::GetMinLightStepSizeAttr() const
{
    return GetPrim().GetAttribute(Tokens->minLightStepSize);
}

UsdAttribute
GraviRenderSettings::CreateMinLightStepSizeAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(Tokens->minLightStepSize,
                       SdfValueTypeNames->Double,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
GraviRenderSettings::GetMaxLightStepSizeAttr() const
{
    return GetPrim().GetAttribute(Tokens->maxLightStepSize);
}

UsdAttribute
GraviRenderSettings::CreateMaxLightStepSizeAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(Tokens->maxLightStepSize,
                       SdfValueTypeNames->Double,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
GraviRenderSettings::GetMaxLightDistanceAttr() const
{
    return GetPrim().GetAttribute(Tokens->maxLightDistance);
}

UsdAttribute
GraviRenderSettings::CreateMaxLightDistanceAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(Tokens->maxLightDistance,
                       SdfValueTypeNames->Double,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
GraviRenderSettings::GetMaxLightStepsAttr() const
{
    return GetPrim().GetAttribute(Tokens->maxLightSteps);
}

UsdAttribute
GraviRenderSettings::CreateMaxLightStepsAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(Tokens->maxLightSteps,
                       SdfValueTypeNames->Int,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
GraviRenderSettings::GetOutputPathAttr() const
{
    return GetPrim().GetAttribute(Tokens->outputPath);
}

UsdAttribute
GraviRenderSettings::CreateOutputPathAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(Tokens->outputPath,
                       SdfValueTypeNames->String,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

namespace {
static inline TfTokenVector
_ConcatenateAttributeNames(const TfTokenVector& left,const TfTokenVector& right)
{
    TfTokenVector result;
    result.reserve(left.size() + right.size());
    result.insert(result.end(), left.begin(), left.end());
    result.insert(result.end(), right.begin(), right.end());
    return result;
}
}

/*static*/
const TfTokenVector&
GraviRenderSettings::GetSchemaAttributeNames(bool includeInherited)
{
    static TfTokenVector localNames = {
        Tokens->convergedSamplesPerPixel,
        Tokens->bounces,
        Tokens->frameRange,
        Tokens->lightStepSize,
        Tokens->minLightStepSize,
        Tokens->maxLightStepSize,
        Tokens->maxLightDistance,
        Tokens->maxLightSteps,
        Tokens->outputPath,
    };
    static TfTokenVector allNames =
        _ConcatenateAttributeNames(
            UsdRenderSettings::GetSchemaAttributeNames(true),
            localNames);

    if (includeInherited)
        return allNames;
    else
        return localNames;
}

PXR_NAMESPACE_CLOSE_SCOPE

// ===================================================================== //
// Feel free to add custom code below this line. It will be preserved by
// the code generator.
//
// Just remember to wrap code in the appropriate delimiters:
// 'PXR_NAMESPACE_OPEN_SCOPE', 'PXR_NAMESPACE_CLOSE_SCOPE'.
// ===================================================================== //
// --(BEGIN CUSTOM CODE)--
