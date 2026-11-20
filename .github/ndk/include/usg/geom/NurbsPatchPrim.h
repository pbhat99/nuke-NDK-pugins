// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

// This is automatically-generated code.
// You may add custom code between the --(BEGIN CUSTOM CODE)-- and --(END CUSTOM CODE)-- markers.
// Code added anywhere else will be destroyed when the code is generated.

#ifndef USG_GEOM_NURBSPATCHPRIM_H
#define USG_GEOM_NURBSPATCHPRIM_H

#include "usg/api.h"

#include "usg/geom/GeomTokens.h"

#include "usg/geom/Prim.h"
#include "fdk/math/Vec2.h"
#include "usg/base/ArrayTypes.h"
#include "usg/geom/PointBasedPrim.h"

namespace usg {

  // class NurbsPatchPrim
  //   inherits from PointBased, Gprim, Boundable, Xformable, Imageable
  //
  // Attributes:
  //     double[] pointWeights
  //     int[] trimCurve:counts
  //     double[] trimCurve:knots
  //     int[] trimCurve:orders
  //     double3[] trimCurve:points
  //     double2[] trimCurve:ranges
  //     int[] trimCurve:vertexCounts
  //     uniform token uForm = "open" (allowed values: open, closed, periodic)
  //     double[] uKnots
  //     int uOrder
  //     double2 uRange
  //     int uVertexCount
  //     uniform token vForm = "open" (allowed values: open, closed, periodic)
  //     double[] vKnots
  //     int vOrder
  //     double2 vRange
  //     int vVertexCount
  //
  // Inherited attributes:
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

  class USG_API NurbsPatchPrim : public PointBasedPrim {
  public:
    NurbsPatchPrim();
    NurbsPatchPrim(const Prim& prim);
    ~NurbsPatchPrim();

    bool validate();
    static bool isValidPrim(const Prim& prim);
    static NurbsPatchPrim defineInLayer(const LayerRef& layer, const Path& path);
    static NurbsPatchPrim overrideInLayer(const LayerRef& layer, const Prim& prim);
    static NurbsPatchPrim getInStage(const StageRef& stage, const Path& path);

    Attribute createPointWeightsAttr(const Value& defaultValue = Value());
    Attribute getPointWeightsAttr() const;
    void setPointWeights(const DoubleArray& value, fdk::TimeValue time = fdk::defaultTimeValue());
    DoubleArray getPointWeights(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createCountsAttr(const Value& defaultValue = Value());
    Attribute getCountsAttr() const;
    void setCounts(const IntArray& value, fdk::TimeValue time = fdk::defaultTimeValue());
    IntArray getCounts(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createKnotsAttr(const Value& defaultValue = Value());
    Attribute getKnotsAttr() const;
    void setKnots(const DoubleArray& value, fdk::TimeValue time = fdk::defaultTimeValue());
    DoubleArray getKnots(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createOrdersAttr(const Value& defaultValue = Value());
    Attribute getOrdersAttr() const;
    void setOrders(const IntArray& value, fdk::TimeValue time = fdk::defaultTimeValue());
    IntArray getOrders(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createPointsAttr(const Value& defaultValue = Value());
    Attribute getPointsAttr() const;
    void setPoints(const Vec3dArray& value, fdk::TimeValue time = fdk::defaultTimeValue());
    Vec3dArray getPoints(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createRangesAttr(const Value& defaultValue = Value());
    Attribute getRangesAttr() const;
    void setRanges(const Vec2dArray& value, fdk::TimeValue time = fdk::defaultTimeValue());
    Vec2dArray getRanges(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createVertexCountsAttr(const Value& defaultValue = Value());
    Attribute getVertexCountsAttr() const;
    void setVertexCounts(const IntArray& value, fdk::TimeValue time = fdk::defaultTimeValue());
    IntArray getVertexCounts(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createUFormAttr(const Value& defaultValue = Value());
    Attribute getUFormAttr() const;
    void setUForm(const Token& value, fdk::TimeValue time = fdk::defaultTimeValue());
    Token getUForm(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createUKnotsAttr(const Value& defaultValue = Value());
    Attribute getUKnotsAttr() const;
    void setUKnots(const DoubleArray& value, fdk::TimeValue time = fdk::defaultTimeValue());
    DoubleArray getUKnots(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createUOrderAttr(const Value& defaultValue = Value());
    Attribute getUOrderAttr() const;
    void setUOrder(int value, fdk::TimeValue time = fdk::defaultTimeValue());
    int getUOrder(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createURangeAttr(const Value& defaultValue = Value());
    Attribute getURangeAttr() const;
    void setURange(const fdk::Vec2d& value, fdk::TimeValue time = fdk::defaultTimeValue());
    fdk::Vec2d getURange(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createUVertexCountAttr(const Value& defaultValue = Value());
    Attribute getUVertexCountAttr() const;
    void setUVertexCount(int value, fdk::TimeValue time = fdk::defaultTimeValue());
    int getUVertexCount(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createVFormAttr(const Value& defaultValue = Value());
    Attribute getVFormAttr() const;
    void setVForm(const Token& value, fdk::TimeValue time = fdk::defaultTimeValue());
    Token getVForm(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createVKnotsAttr(const Value& defaultValue = Value());
    Attribute getVKnotsAttr() const;
    void setVKnots(const DoubleArray& value, fdk::TimeValue time = fdk::defaultTimeValue());
    DoubleArray getVKnots(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createVOrderAttr(const Value& defaultValue = Value());
    Attribute getVOrderAttr() const;
    void setVOrder(int value, fdk::TimeValue time = fdk::defaultTimeValue());
    int getVOrder(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createVRangeAttr(const Value& defaultValue = Value());
    Attribute getVRangeAttr() const;
    void setVRange(const fdk::Vec2d& value, fdk::TimeValue time = fdk::defaultTimeValue());
    fdk::Vec2d getVRange(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createVVertexCountAttr(const Value& defaultValue = Value());
    Attribute getVVertexCountAttr() const;
    void setVVertexCount(int value, fdk::TimeValue time = fdk::defaultTimeValue());
    int getVVertexCount(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    // --(BEGIN CUSTOM CODE)--
    // --(END CUSTOM CODE)--
  };

}

#endif
