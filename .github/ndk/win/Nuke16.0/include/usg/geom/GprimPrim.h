// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

// This is automatically-generated code.
// You may add custom code between the --(BEGIN CUSTOM CODE)-- and --(END CUSTOM CODE)-- markers.
// Code added anywhere else will be destroyed when the code is generated.

#ifndef USG_GEOM_GPRIMPRIM_H
#define USG_GEOM_GPRIMPRIM_H

#include "usg/api.h"

#include "usg/geom/GeomTokens.h"

#include "usg/geom/Prim.h"
#include "usg/base/ArrayTypes.h"
#include "usg/geom/BoundablePrim.h"

namespace usg {

  // class GprimPrim
  //   inherits from Boundable, Xformable, Imageable
  //
  // Attributes:
  //     uniform bool doubleSided
  //     uniform token orientation = "rightHanded" (allowed values: rightHanded, leftHanded)
  //     color3f[] primvars:displayColor
  //     float[] primvars:displayOpacity
  //
  // Inherited attributes:
  //   From Boundable:
  //     float3[] extent
  //   From Xformable:
  //     uniform token[] xformOpOrder
  //   From Imageable:
  //     rel proxyPrim
  //     uniform token purpose = "default" (allowed values: default, render, proxy, guide)
  //     token visibility = "inherited" (allowed values: inherited, invisible)

  class USG_API GprimPrim : public BoundablePrim {
  public:
    GprimPrim();
    GprimPrim(const Prim& prim);
    ~GprimPrim();

    bool validate();
    static bool isValidPrim(const Prim& prim);
    static GprimPrim overrideInLayer(const LayerRef& layer, const Prim& prim);
    static GprimPrim getInStage(const StageRef& stage, const Path& path);

    Attribute createDoubleSidedAttr(const Value& defaultValue = Value());
    Attribute getDoubleSidedAttr() const;
    void setDoubleSided(bool value, fdk::TimeValue time = fdk::defaultTimeValue());
    bool getDoubleSided(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createOrientationAttr(const Value& defaultValue = Value());
    Attribute getOrientationAttr() const;
    void setOrientation(const Token& value, fdk::TimeValue time = fdk::defaultTimeValue());
    Token getOrientation(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createDisplayColorAttr(const Value& defaultValue = Value());
    Attribute getDisplayColorAttr() const;
    void setDisplayColor(const Vec3fArray& value, fdk::TimeValue time = fdk::defaultTimeValue());
    Vec3fArray getDisplayColor(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createDisplayOpacityAttr(const Value& defaultValue = Value());
    Attribute getDisplayOpacityAttr() const;
    void setDisplayOpacity(const FloatArray& value, fdk::TimeValue time = fdk::defaultTimeValue());
    FloatArray getDisplayOpacity(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    // --(BEGIN CUSTOM CODE)--
    // --(END CUSTOM CODE)--
  };

  using GeometryPrim = GprimPrim;
}

#endif
