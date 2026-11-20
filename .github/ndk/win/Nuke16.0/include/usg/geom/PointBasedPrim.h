// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

// This is automatically-generated code.
// You may add custom code between the --(BEGIN CUSTOM CODE)-- and --(END CUSTOM CODE)-- markers.
// Code added anywhere else will be destroyed when the code is generated.

#ifndef USG_GEOM_POINTBASEDPRIM_H
#define USG_GEOM_POINTBASEDPRIM_H

#include "usg/api.h"

#include "usg/geom/GeomTokens.h"

#include "usg/geom/Prim.h"
#include "usg/base/ArrayTypes.h"
#include "usg/geom/GprimPrim.h"

namespace usg {

  // class PointBasedPrim
  //   inherits from Gprim, Boundable, Xformable, Imageable
  //
  // Attributes:
  //     vector3f[] accelerations
  //     normal3f[] normals
  //     point3f[] points
  //     vector3f[] velocities
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

  class USG_API PointBasedPrim : public GprimPrim {
  public:
    PointBasedPrim();
    PointBasedPrim(const Prim& prim);
    ~PointBasedPrim();

    bool validate();
    static bool isValidPrim(const Prim& prim);
    static PointBasedPrim overrideInLayer(const LayerRef& layer, const Prim& prim);
    static PointBasedPrim getInStage(const StageRef& stage, const Path& path);

    Attribute createAccelerationsAttr(const Value& defaultValue = Value());
    Attribute getAccelerationsAttr() const;
    void setAccelerations(const Vec3fArray& value, fdk::TimeValue time = fdk::defaultTimeValue());
    Vec3fArray getAccelerations(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createNormalsAttr(const Value& defaultValue = Value());
    Attribute getNormalsAttr() const;
    void setNormals(const Vec3fArray& value, fdk::TimeValue time = fdk::defaultTimeValue());
    Vec3fArray getNormals(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createPointsAttr(const Value& defaultValue = Value());
    Attribute getPointsAttr() const;
    void setPoints(const Vec3fArray& value, fdk::TimeValue time = fdk::defaultTimeValue());
    Vec3fArray getPoints(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createVelocitiesAttr(const Value& defaultValue = Value());
    Attribute getVelocitiesAttr() const;
    void setVelocities(const Vec3fArray& value, fdk::TimeValue time = fdk::defaultTimeValue());
    Vec3fArray getVelocities(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    // --(BEGIN CUSTOM CODE)--
    //! Retrieves the points property value.
    size_t    getPointsArray(Vec3fArray&           points,
                             const fdk::TimeValue& time = fdk::defaultTimeValue()) const;

    //! Set the primvars:st attribute and interpolation
    virtual Attribute  setNormalsAttr(const Vec3fArray&     normals,
                                      const Token&          interpolation,
                                      const fdk::TimeValue& time = fdk::defaultTimeValue());

    //! Set the primvars:st attribute and interpolation
    virtual Attribute  setStAttr(const Vec2fArray&     st,
                                 const Token&          interpolation,
                                 const fdk::TimeValue& time = fdk::defaultTimeValue());

    //! Retrieve the primvars:st property. If it doesn't exist the returned Attribute will be invalid(not bound.)
    virtual Attribute  getStAttr() const;
    // --(END CUSTOM CODE)--
  };

}

#endif
