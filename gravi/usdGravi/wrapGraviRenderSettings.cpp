//
// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
#include "./graviRenderSettings.h"
#include "pxr/usd/usd/schemaBase.h"

#include "pxr/usd/sdf/primSpec.h"

#include "pxr/usd/usd/pyConversions.h"
#include "pxr/base/tf/pyContainerConversions.h"
#include "pxr/base/tf/pyResultConversions.h"
#include "pxr/base/tf/pyUtils.h"
#include "pxr/base/tf/wrapTypeHelpers.h"

#include "pxr/external/boost/python.hpp"

#include <string>

PXR_NAMESPACE_USING_DIRECTIVE

using namespace pxr_boost::python;

namespace {

#define WRAP_CUSTOM                                                     \
    template <class Cls> static void _CustomWrapCode(Cls &_class)

// fwd decl.
WRAP_CUSTOM;

        
static UsdAttribute
_CreateConvergedSamplesPerPixelAttr(GraviRenderSettings &self,
                                      object defaultVal, bool writeSparsely) {
    return self.CreateConvergedSamplesPerPixelAttr(
        UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Int), writeSparsely);
}
        
static UsdAttribute
_CreateBouncesAttr(GraviRenderSettings &self,
                                      object defaultVal, bool writeSparsely) {
    return self.CreateBouncesAttr(
        UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Int), writeSparsely);
}
        
static UsdAttribute
_CreateFrameRangeAttr(GraviRenderSettings &self,
                                      object defaultVal, bool writeSparsely) {
    return self.CreateFrameRangeAttr(
        UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Int2), writeSparsely);
}
        
static UsdAttribute
_CreateLightStepSizeAttr(GraviRenderSettings &self,
                                      object defaultVal, bool writeSparsely) {
    return self.CreateLightStepSizeAttr(
        UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Double), writeSparsely);
}
        
static UsdAttribute
_CreateMinLightStepSizeAttr(GraviRenderSettings &self,
                                      object defaultVal, bool writeSparsely) {
    return self.CreateMinLightStepSizeAttr(
        UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Double), writeSparsely);
}
        
static UsdAttribute
_CreateMaxLightStepSizeAttr(GraviRenderSettings &self,
                                      object defaultVal, bool writeSparsely) {
    return self.CreateMaxLightStepSizeAttr(
        UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Double), writeSparsely);
}
        
static UsdAttribute
_CreateMaxLightDistanceAttr(GraviRenderSettings &self,
                                      object defaultVal, bool writeSparsely) {
    return self.CreateMaxLightDistanceAttr(
        UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Double), writeSparsely);
}
        
static UsdAttribute
_CreateMaxLightStepsAttr(GraviRenderSettings &self,
                                      object defaultVal, bool writeSparsely) {
    return self.CreateMaxLightStepsAttr(
        UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Int), writeSparsely);
}
        
static UsdAttribute
_CreateOutputPathAttr(GraviRenderSettings &self,
                                      object defaultVal, bool writeSparsely) {
    return self.CreateOutputPathAttr(
        UsdPythonToSdfType(defaultVal, SdfValueTypeNames->String), writeSparsely);
}

static std::string
_Repr(const GraviRenderSettings &self)
{
    std::string primRepr = TfPyRepr(self.GetPrim());
    return TfStringPrintf(
        "GRAVI.GraviRenderSettings(%s)",
        primRepr.c_str());
}

} // anonymous namespace

void wrapGraviRenderSettings()
{
    typedef GraviRenderSettings This;

    class_<This, bases<UsdRenderSettings> >
        cls("GraviRenderSettings");

    cls
        .def(init<UsdPrim>(arg("prim")))
        .def(init<UsdSchemaBase const&>(arg("schemaObj")))
        .def(TfTypePythonClass())

        .def("Get", &This::Get, (arg("stage"), arg("path")))
        .staticmethod("Get")

        .def("Define", &This::Define, (arg("stage"), arg("path")))
        .staticmethod("Define")

        .def("GetSchemaAttributeNames",
             &This::GetSchemaAttributeNames,
             arg("includeInherited")=true,
             return_value_policy<TfPySequenceToList>())
        .staticmethod("GetSchemaAttributeNames")

        .def("_GetStaticTfType", (TfType const &(*)()) TfType::Find<This>,
             return_value_policy<return_by_value>())
        .staticmethod("_GetStaticTfType")

        .def(!self)

        
        .def("GetConvergedSamplesPerPixelAttr",
             &This::GetConvergedSamplesPerPixelAttr)
        .def("CreateConvergedSamplesPerPixelAttr",
             &_CreateConvergedSamplesPerPixelAttr,
             (arg("defaultValue")=object(),
              arg("writeSparsely")=false))
        
        .def("GetBouncesAttr",
             &This::GetBouncesAttr)
        .def("CreateBouncesAttr",
             &_CreateBouncesAttr,
             (arg("defaultValue")=object(),
              arg("writeSparsely")=false))
        
        .def("GetFrameRangeAttr",
             &This::GetFrameRangeAttr)
        .def("CreateFrameRangeAttr",
             &_CreateFrameRangeAttr,
             (arg("defaultValue")=object(),
              arg("writeSparsely")=false))
        
        .def("GetLightStepSizeAttr",
             &This::GetLightStepSizeAttr)
        .def("CreateLightStepSizeAttr",
             &_CreateLightStepSizeAttr,
             (arg("defaultValue")=object(),
              arg("writeSparsely")=false))
        
        .def("GetMinLightStepSizeAttr",
             &This::GetMinLightStepSizeAttr)
        .def("CreateMinLightStepSizeAttr",
             &_CreateMinLightStepSizeAttr,
             (arg("defaultValue")=object(),
              arg("writeSparsely")=false))
        
        .def("GetMaxLightStepSizeAttr",
             &This::GetMaxLightStepSizeAttr)
        .def("CreateMaxLightStepSizeAttr",
             &_CreateMaxLightStepSizeAttr,
             (arg("defaultValue")=object(),
              arg("writeSparsely")=false))
        
        .def("GetMaxLightDistanceAttr",
             &This::GetMaxLightDistanceAttr)
        .def("CreateMaxLightDistanceAttr",
             &_CreateMaxLightDistanceAttr,
             (arg("defaultValue")=object(),
              arg("writeSparsely")=false))
        
        .def("GetMaxLightStepsAttr",
             &This::GetMaxLightStepsAttr)
        .def("CreateMaxLightStepsAttr",
             &_CreateMaxLightStepsAttr,
             (arg("defaultValue")=object(),
              arg("writeSparsely")=false))
        
        .def("GetOutputPathAttr",
             &This::GetOutputPathAttr)
        .def("CreateOutputPathAttr",
             &_CreateOutputPathAttr,
             (arg("defaultValue")=object(),
              arg("writeSparsely")=false))

        .def("__repr__", ::_Repr)
    ;

    _CustomWrapCode(cls);
}

// ===================================================================== //
// Feel free to add custom code below this line, it will be preserved by 
// the code generator.  The entry point for your custom code should look
// minimally like the following:
//
// WRAP_CUSTOM {
//     _class
//         .def("MyCustomMethod", ...)
//     ;
// }
//
// Of course any other ancillary or support code may be provided.
// 
// Just remember to wrap code in the appropriate delimiters:
// 'namespace {', '}'.
//
// ===================================================================== //
// --(BEGIN CUSTOM CODE)--

namespace {

WRAP_CUSTOM {
}

}
