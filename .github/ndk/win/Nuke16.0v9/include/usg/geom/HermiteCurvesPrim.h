// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

// This is automatically-generated code.
// You may add custom code between the --(BEGIN CUSTOM CODE)-- and --(END CUSTOM CODE)-- markers.
// Code added anywhere else will be destroyed when the code is generated.

#ifndef USG_GEOM_HERMITECURVESPRIM_H
#define USG_GEOM_HERMITECURVESPRIM_H

#include "usg/api.h"

#include "usg/geom/GeomTokens.h"

#include "usg/geom/Prim.h"
#include "usg/base/ArrayTypes.h"
#include "usg/geom/CurvesPrim.h"

namespace usg {

  // class HermiteCurvesPrim
  //   inherits from Curves, PointBased, Gprim, Boundable, Xformable, Imageable
  //
  // Attributes:
  //     vector3f[] tangents
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

  class USG_API HermiteCurvesPrim : public CurvesPrim {
  public:
    HermiteCurvesPrim();
    HermiteCurvesPrim(const Prim& prim);
    ~HermiteCurvesPrim();

    bool validate();
    static bool isValidPrim(const Prim& prim);
    static HermiteCurvesPrim defineInLayer(const LayerRef& layer, const Path& path);
    static HermiteCurvesPrim overrideInLayer(const LayerRef& layer, const Prim& prim);
    static HermiteCurvesPrim getInStage(const StageRef& stage, const Path& path);

    Attribute createTangentsAttr(const Value& defaultValue = Value());
    Attribute getTangentsAttr() const;
    void setTangents(const Vec3fArray& value, fdk::TimeValue time = fdk::defaultTimeValue());
    Vec3fArray getTangents(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    // --(BEGIN CUSTOM CODE)--
    // --(END CUSTOM CODE)--
  };

}

#endif
