// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

// This is automatically-generated code.
// You may add custom code between the --(BEGIN CUSTOM CODE)-- and --(END CUSTOM CODE)-- markers.
// Code added anywhere else will be destroyed when the code is generated.

#ifndef USG_GEOM_NURBSCURVESPRIM_H
#define USG_GEOM_NURBSCURVESPRIM_H

#include "usg/api.h"

#include "usg/geom/GeomTokens.h"

#include "usg/geom/Prim.h"
#include "usg/base/ArrayTypes.h"
#include "usg/geom/CurvesPrim.h"

namespace usg {

  // class NurbsCurvesPrim
  //   inherits from Curves, PointBased, Gprim, Boundable, Xformable, Imageable
  //
  // Attributes:
  //     double[] knots
  //     int[] order
  //     double2[] ranges
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

  class USG_API NurbsCurvesPrim : public CurvesPrim {
  public:
    NurbsCurvesPrim();
    NurbsCurvesPrim(const Prim& prim);
    ~NurbsCurvesPrim();

    bool validate();
    static bool isValidPrim(const Prim& prim);
    static NurbsCurvesPrim defineInLayer(const LayerRef& layer, const Path& path);
    static NurbsCurvesPrim overrideInLayer(const LayerRef& layer, const Prim& prim);
    static NurbsCurvesPrim getInStage(const StageRef& stage, const Path& path);

    Attribute createKnotsAttr(const Value& defaultValue = Value());
    Attribute getKnotsAttr() const;
    void setKnots(const DoubleArray& value, fdk::TimeValue time = fdk::defaultTimeValue());
    DoubleArray getKnots(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createOrderAttr(const Value& defaultValue = Value());
    Attribute getOrderAttr() const;
    void setOrder(const IntArray& value, fdk::TimeValue time = fdk::defaultTimeValue());
    IntArray getOrder(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createRangesAttr(const Value& defaultValue = Value());
    Attribute getRangesAttr() const;
    void setRanges(const Vec2dArray& value, fdk::TimeValue time = fdk::defaultTimeValue());
    Vec2dArray getRanges(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    // --(BEGIN CUSTOM CODE)--
    // --(END CUSTOM CODE)--
  };

}

#endif
