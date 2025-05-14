//
// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
// GENERATED FILE.  DO NOT EDIT.
#include "pxr/external/boost/python/class.hpp"
#include "./tokens.h"

PXR_NAMESPACE_USING_DIRECTIVE

#define _ADD_TOKEN(cls, name) \
    cls.add_static_property(#name, +[]() { return Tokens->name.GetString(); });

void wrapTokens()
{
    pxr_boost::python::class_<TokensType, pxr_boost::python::noncopyable>
        cls("Tokens", pxr_boost::python::no_init);
    _ADD_TOKEN(cls, bounces);
    _ADD_TOKEN(cls, convergedSamplesPerPixel);
    _ADD_TOKEN(cls, force);
    _ADD_TOKEN(cls, frameRange);
    _ADD_TOKEN(cls, lightStepSize);
    _ADD_TOKEN(cls, maxLightDistance);
    _ADD_TOKEN(cls, maxLightSteps);
    _ADD_TOKEN(cls, maxLightStepSize);
    _ADD_TOKEN(cls, minLightStepSize);
    _ADD_TOKEN(cls, outputPath);
    _ADD_TOKEN(cls, GraviRenderSettings);
    _ADD_TOKEN(cls, GravityWell);
}
