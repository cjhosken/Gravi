//
// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
#include "./gravityWell.h"
#include "pxr/usd/usd/schemaRegistry.h"
#include "pxr/usd/usd/typed.h"

#include "pxr/usd/sdf/types.h"
#include "pxr/usd/sdf/assetPath.h"

PXR_NAMESPACE_OPEN_SCOPE

// Register the schema with the TfType system.
TF_REGISTRY_FUNCTION(TfType)
{
    TfType::Define<GravityWell,
        TfType::Bases< UsdGeomGprim > >();
    
    // Skip registering an alias as it would be the same as the class name.
}

/* virtual */
GravityWell::~GravityWell()
{
}

/* static */
GravityWell
GravityWell::Get(const UsdStagePtr &stage, const SdfPath &path)
{
    if (!stage) {
        TF_CODING_ERROR("Invalid stage");
        return GravityWell();
    }
    return GravityWell(stage->GetPrimAtPath(path));
}

/* static */
GravityWell
GravityWell::Define(
    const UsdStagePtr &stage, const SdfPath &path)
{
    static TfToken usdPrimTypeName("GravityWell");
    if (!stage) {
        TF_CODING_ERROR("Invalid stage");
        return GravityWell();
    }
    return GravityWell(
        stage->DefinePrim(path, usdPrimTypeName));
}

/* virtual */
UsdSchemaKind GravityWell::_GetSchemaKind() const
{
    return GravityWell::schemaKind;
}

/* static */
const TfType &
GravityWell::_GetStaticTfType()
{
    static TfType tfType = TfType::Find<GravityWell>();
    return tfType;
}

/* static */
bool 
GravityWell::_IsTypedSchema()
{
    static bool isTyped = _GetStaticTfType().IsA<UsdTyped>();
    return isTyped;
}

/* virtual */
const TfType &
GravityWell::_GetTfType() const
{
    return _GetStaticTfType();
}

UsdAttribute
GravityWell::GetForceAttr() const
{
    return GetPrim().GetAttribute(Tokens->force);
}

UsdAttribute
GravityWell::CreateForceAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(Tokens->force,
                       SdfValueTypeNames->Float,
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
GravityWell::GetSchemaAttributeNames(bool includeInherited)
{
    static TfTokenVector localNames = {
        Tokens->force,
    };
    static TfTokenVector allNames =
        _ConcatenateAttributeNames(
            UsdGeomGprim::GetSchemaAttributeNames(true),
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
