// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

// This is automatically-generated code.
// You may add custom code between the --(BEGIN CUSTOM CODE)-- and --(END CUSTOM CODE)-- markers.
// Code added anywhere else will be destroyed when the code is generated.

#ifndef USG_GEOM_BASISCURVESPRIM_H
#define USG_GEOM_BASISCURVESPRIM_H

#include "usg/api.h"

#include "usg/geom/GeomTokens.h"

#include "usg/geom/Prim.h"
#include "usg/geom/CurvesPrim.h"

namespace usg {

  // class BasisCurvesPrim
  //   inherits from Curves, PointBased, Gprim, Boundable, Xformable, Imageable
  //
  // Attributes:
  //     uniform token basis = "bezier" (allowed values: bezier, bspline, catmullRom)
  //     uniform token type = "cubic" (allowed values: linear, cubic)
  //     uniform token wrap = "nonperiodic" (allowed values: nonperiodic, periodic, pinned)
  //
  // Inherited attributes:
  //   From Curves:
  //     int[] curveVertexCounts
  //     float[] widths
  //   From PointBased:
  //     vector3f[] accelerations
  //     normal3f[] normals
  //     point3f[] points
  //     vector3f[] velocities
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

  class USG_API BasisCurvesPrim : public CurvesPrim {
  public:
    BasisCurvesPrim();
    BasisCurvesPrim(const Prim& prim);
    ~BasisCurvesPrim();

    bool validate();
    static bool isValidPrim(const Prim& prim);
    static BasisCurvesPrim defineInLayer(const LayerRef& layer, const Path& path);
    static BasisCurvesPrim overrideInLayer(const LayerRef& layer, const Prim& prim);
    static BasisCurvesPrim getInStage(const StageRef& stage, const Path& path);

    Attribute createBasisAttr(const Value& defaultValue = Value());
    Attribute getBasisAttr() const;
    void setBasis(const Token& value, fdk::TimeValue time = fdk::defaultTimeValue());
    Token getBasis(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createTypeAttr(const Value& defaultValue = Value());
    Attribute getTypeAttr() const;
    void setType(const Token& value, fdk::TimeValue time = fdk::defaultTimeValue());
    Token getType(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createWrapAttr(const Value& defaultValue = Value());
    Attribute getWrapAttr() const;
    void setWrap(const Token& value, fdk::TimeValue time = fdk::defaultTimeValue());
    Token getWrap(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    // --(BEGIN CUSTOM CODE)--
    // --(END CUSTOM CODE)--
  };

}

#endif
