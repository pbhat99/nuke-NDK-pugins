// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

// This is automatically-generated code.
// You may add custom code between the --(BEGIN CUSTOM CODE)-- and --(END CUSTOM CODE)-- markers.
// Code added anywhere else will be destroyed when the code is generated.

#ifndef USG_GEOM_POINTSPRIM_H
#define USG_GEOM_POINTSPRIM_H

#include "usg/api.h"

#include "usg/geom/GeomTokens.h"

#include "usg/geom/Prim.h"
#include "usg/base/ArrayTypes.h"
#include "usg/geom/PointBasedPrim.h"

namespace usg {

  // class PointsPrim
  //   inherits from PointBased, Gprim, Boundable, Xformable, Imageable
  //
  // Attributes:
  //     int64[] ids
  //     float[] widths
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

  class USG_API PointsPrim : public PointBasedPrim {
  public:
    PointsPrim();
    PointsPrim(const Prim& prim);
    ~PointsPrim();

    bool validate();
    static bool isValidPrim(const Prim& prim);
    static PointsPrim defineInLayer(const LayerRef& layer, const Path& path);
    static PointsPrim overrideInLayer(const LayerRef& layer, const Prim& prim);
    static PointsPrim getInStage(const StageRef& stage, const Path& path);

    Attribute createIdsAttr(const Value& defaultValue = Value());
    Attribute getIdsAttr() const;
    void setIds(const Int64Array& value, fdk::TimeValue time = fdk::defaultTimeValue());
    Int64Array getIds(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createWidthsAttr(const Value& defaultValue = Value());
    Attribute getWidthsAttr() const;
    void setWidths(const FloatArray& value, fdk::TimeValue time = fdk::defaultTimeValue());
    FloatArray getWidths(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    // --(BEGIN CUSTOM CODE)--
    // --(END CUSTOM CODE)--
  };

}

#endif
