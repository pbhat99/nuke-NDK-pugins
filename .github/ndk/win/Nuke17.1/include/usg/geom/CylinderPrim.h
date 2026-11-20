// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

// This is automatically-generated code.
// You may add custom code between the --(BEGIN CUSTOM CODE)-- and --(END CUSTOM CODE)-- markers.
// Code added anywhere else will be destroyed when the code is generated.

#ifndef USG_GEOM_CYLINDERPRIM_H
#define USG_GEOM_CYLINDERPRIM_H

#include "usg/api.h"

#include "usg/geom/GeomTokens.h"

#include "usg/geom/Prim.h"
#include "usg/geom/GprimPrim.h"

namespace usg {

  // class CylinderPrim
  //   inherits from Gprim, Boundable, Xformable, Imageable
  //
  // Attributes:
  //     uniform token axis = "Z" (allowed values: X, Y, Z)
  //     double height = 2.0
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

  class USG_API CylinderPrim : public GprimPrim {
  public:
    CylinderPrim();
    CylinderPrim(const Prim& prim);
    ~CylinderPrim();

    bool validate();
    static bool isValidPrim(const Prim& prim);
    static CylinderPrim defineInLayer(const LayerRef& layer, const Path& path);
    static CylinderPrim overrideInLayer(const LayerRef& layer, const Prim& prim);
    static CylinderPrim getInStage(const StageRef& stage, const Path& path);

    Attribute createAxisAttr(const Value& defaultValue = Value());
    Attribute getAxisAttr() const;
    void setAxis(const Token& value, fdk::TimeValue time = fdk::defaultTimeValue());
    Token getAxis(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createHeightAttr(const Value& defaultValue = Value());
    Attribute getHeightAttr() const;
    void setHeight(double value, fdk::TimeValue time = fdk::defaultTimeValue());
    double getHeight(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createRadiusAttr(const Value& defaultValue = Value());
    Attribute getRadiusAttr() const;
    void setRadius(double value, fdk::TimeValue time = fdk::defaultTimeValue());
    double getRadius(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    // --(BEGIN CUSTOM CODE)--
    // --(END CUSTOM CODE)--
  };

}

#endif
