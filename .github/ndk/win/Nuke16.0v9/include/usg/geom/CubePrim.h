// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

// This is automatically-generated code.
// You may add custom code between the --(BEGIN CUSTOM CODE)-- and --(END CUSTOM CODE)-- markers.
// Code added anywhere else will be destroyed when the code is generated.

#ifndef USG_GEOM_CUBEPRIM_H
#define USG_GEOM_CUBEPRIM_H

#include "usg/api.h"

#include "usg/geom/GeomTokens.h"

#include "usg/geom/Prim.h"
#include "usg/geom/GprimPrim.h"

namespace usg {

  // class CubePrim
  //   inherits from Gprim, Boundable, Xformable, Imageable
  //
  // Attributes:
  //     double size = 2.0
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

  class USG_API CubePrim : public GprimPrim {
  public:
    CubePrim();
    CubePrim(const Prim& prim);
    ~CubePrim();

    bool validate();
    static bool isValidPrim(const Prim& prim);
    static CubePrim defineInLayer(const LayerRef& layer, const Path& path);
    static CubePrim overrideInLayer(const LayerRef& layer, const Prim& prim);
    static CubePrim getInStage(const StageRef& stage, const Path& path);

    Attribute createSizeAttr(const Value& defaultValue = Value());
    Attribute getSizeAttr() const;
    void setSize(double value, fdk::TimeValue time = fdk::defaultTimeValue());
    double getSize(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    // --(BEGIN CUSTOM CODE)--
    // --(END CUSTOM CODE)--
  };

}

#endif
