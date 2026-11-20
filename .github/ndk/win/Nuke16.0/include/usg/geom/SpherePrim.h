// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

// This is automatically-generated code.
// You may add custom code between the --(BEGIN CUSTOM CODE)-- and --(END CUSTOM CODE)-- markers.
// Code added anywhere else will be destroyed when the code is generated.

#ifndef USG_GEOM_SPHEREPRIM_H
#define USG_GEOM_SPHEREPRIM_H

#include "usg/api.h"

#include "usg/geom/GeomTokens.h"

#include "usg/geom/Prim.h"
#include "usg/geom/GprimPrim.h"

namespace usg {

  // class SpherePrim
  //   inherits from Gprim, Boundable, Xformable, Imageable
  //
  // Attributes:
  //     double radius = 1.0
  //
  // Inherited attributes:
  //   From Gprim:
  //     uniform bool doubleSided
  //     uniform token orientation = "rightHanded" (allowed values: rightHanded, leftHanded)
  //     color3f[] primvars:displayColor
  //     float[] primvars:displayOpacity
  //   From Boundable:
  //     float3[] extent
  //   From Xformable:
  //     uniform token[] xformOpOrder
  //   From Imageable:
  //     rel proxyPrim
  //     uniform token purpose = "default" (allowed values: default, render, proxy, guide)
  //     token visibility = "inherited" (allowed values: inherited, invisible)

  class USG_API SpherePrim : public GprimPrim {
  public:
    SpherePrim();
    SpherePrim(const Prim& prim);
    ~SpherePrim();

    bool validate();
    static bool isValidPrim(const Prim& prim);
    static SpherePrim defineInLayer(const LayerRef& layer, const Path& path);
    static SpherePrim overrideInLayer(const LayerRef& layer, const Prim& prim);
    static SpherePrim getInStage(const StageRef& stage, const Path& path);

    Attribute createRadiusAttr(const Value& defaultValue = Value());
    Attribute getRadiusAttr() const;
    void setRadius(double value, fdk::TimeValue time = fdk::defaultTimeValue());
    double getRadius(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    // --(BEGIN CUSTOM CODE)--
    // --(END CUSTOM CODE)--
  };

}

#endif
