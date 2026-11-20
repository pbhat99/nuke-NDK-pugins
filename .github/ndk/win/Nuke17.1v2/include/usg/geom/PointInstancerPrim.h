// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

// This is automatically-generated code.
// You may add custom code between the --(BEGIN CUSTOM CODE)-- and --(END CUSTOM CODE)-- markers.
// Code added anywhere else will be destroyed when the code is generated.

#ifndef USG_GEOM_POINTINSTANCERPRIM_H
#define USG_GEOM_POINTINSTANCERPRIM_H

#include "usg/api.h"

#include "usg/geom/GeomTokens.h"

#include "usg/geom/Prim.h"
#include "usg/base/ArrayTypes.h"
#include "usg/geom/BoundablePrim.h"

namespace usg {

  // class PointInstancerPrim
  //   inherits from Boundable, Xformable, Imageable
  //
  // Attributes:
  //     vector3f[] accelerations
  //     vector3f[] angularVelocities
  //     int64[] ids
  //     int64[] invisibleIds
  //     quath[] orientations
  //     point3f[] positions
  //     int[] protoIndices
  //     rel prototypes
  //     float3[] scales
  //     vector3f[] velocities
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

  class USG_API PointInstancerPrim : public BoundablePrim {
  public:
    PointInstancerPrim();
    PointInstancerPrim(const Prim& prim);
    ~PointInstancerPrim();

    bool validate();
    static bool isValidPrim(const Prim& prim);
    static PointInstancerPrim defineInLayer(const LayerRef& layer, const Path& path);
    static PointInstancerPrim overrideInLayer(const LayerRef& layer, const Prim& prim);
    static PointInstancerPrim getInStage(const StageRef& stage, const Path& path);

    Attribute createAccelerationsAttr(const Value& defaultValue = Value());
    Attribute getAccelerationsAttr() const;
    void setAccelerations(const Vec3fArray& value, fdk::TimeValue time = fdk::defaultTimeValue());
    Vec3fArray getAccelerations(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createAngularVelocitiesAttr(const Value& defaultValue = Value());
    Attribute getAngularVelocitiesAttr() const;
    void setAngularVelocities(const Vec3fArray& value, fdk::TimeValue time = fdk::defaultTimeValue());
    Vec3fArray getAngularVelocities(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createIdsAttr(const Value& defaultValue = Value());
    Attribute getIdsAttr() const;
    void setIds(const Int64Array& value, fdk::TimeValue time = fdk::defaultTimeValue());
    Int64Array getIds(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createInvisibleIdsAttr(const Value& defaultValue = Value());
    Attribute getInvisibleIdsAttr() const;
    void setInvisibleIds(const Int64Array& value, fdk::TimeValue time = fdk::defaultTimeValue());
    Int64Array getInvisibleIds(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createOrientationsAttr(const Value& defaultValue = Value());
    Attribute getOrientationsAttr() const;
    void setOrientations(const QuathArray& value, fdk::TimeValue time = fdk::defaultTimeValue());
    QuathArray getOrientations(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createPositionsAttr(const Value& defaultValue = Value());
    Attribute getPositionsAttr() const;
    void setPositions(const Vec3fArray& value, fdk::TimeValue time = fdk::defaultTimeValue());
    Vec3fArray getPositions(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createProtoIndicesAttr(const Value& defaultValue = Value());
    Attribute getProtoIndicesAttr() const;
    void setProtoIndices(const IntArray& value, fdk::TimeValue time = fdk::defaultTimeValue());
    IntArray getProtoIndices(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Relationship createPrototypesRel();
    Relationship getPrototypesRel() const;
    void setPrototypes(const PathArray& value);
    PathArray getPrototypes() const;

    Attribute createScalesAttr(const Value& defaultValue = Value());
    Attribute getScalesAttr() const;
    void setScales(const Vec3fArray& value, fdk::TimeValue time = fdk::defaultTimeValue());
    Vec3fArray getScales(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createVelocitiesAttr(const Value& defaultValue = Value());
    Attribute getVelocitiesAttr() const;
    void setVelocities(const Vec3fArray& value, fdk::TimeValue time = fdk::defaultTimeValue());
    Vec3fArray getVelocities(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    // --(BEGIN CUSTOM CODE)--
    fdk::Box3f computeExtentAtTime(fdk::TimeValue time, fdk::TimeValue baseTime) const;
    // --(END CUSTOM CODE)--
  };

}

#endif
