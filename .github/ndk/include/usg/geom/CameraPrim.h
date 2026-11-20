// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

// This is automatically-generated code.
// You may add custom code between the --(BEGIN CUSTOM CODE)-- and --(END CUSTOM CODE)-- markers.
// Code added anywhere else will be destroyed when the code is generated.

#ifndef USG_GEOM_CAMERAPRIM_H
#define USG_GEOM_CAMERAPRIM_H

#include "usg/api.h"

#include "usg/geom/GeomTokens.h"

#include "usg/geom/Prim.h"
#include "fdk/math/Vec2.h"
#include "usg/base/ArrayTypes.h"
#include "usg/geom/XformablePrim.h"

#include <array>

namespace usg {

  // class CameraPrim
  //   inherits from Xformable, Imageable
  //
  // Attributes:
  //     float4[] clippingPlanes
  //     float2 clippingRange = (1, 1000000)
  //     float exposure
  //     float focalLength = 50.0
  //     float focusDistance
  //     float fStop
  //     float horizontalAperture = 20.954999923706055
  //     float horizontalApertureOffset
  //     token projection = "perspective" (allowed values: perspective, orthographic)
  //     double shutter:close
  //     double shutter:open
  //     uniform token stereoRole = "mono" (allowed values: mono, left, right)
  //     float verticalAperture = 15.290800094604492
  //     float verticalApertureOffset
  //
  // Inherited attributes:
  //   From Xformable:
  //     uniform token[] xformOpOrder
  //   From Imageable:
  //     rel proxyPrim
  //     uniform token purpose = "default" (allowed values: default, render, proxy, guide)
  //     token visibility = "inherited" (allowed values: inherited, invisible)

  class USG_API CameraPrim : public XformablePrim {
  public:
    CameraPrim();
    CameraPrim(const Prim& prim);
    ~CameraPrim();

    bool validate();
    static bool isValidPrim(const Prim& prim);
    static CameraPrim defineInLayer(const LayerRef& layer, const Path& path);
    static CameraPrim overrideInLayer(const LayerRef& layer, const Prim& prim);
    static CameraPrim getInStage(const StageRef& stage, const Path& path);

    Attribute createClippingPlanesAttr(const Value& defaultValue = Value());
    Attribute getClippingPlanesAttr() const;
    void setClippingPlanes(const Vec4fArray& value, fdk::TimeValue time = fdk::defaultTimeValue());
    Vec4fArray getClippingPlanes(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createClippingRangeAttr(const Value& defaultValue = Value());
    Attribute getClippingRangeAttr() const;
    void setClippingRange(const fdk::Vec2f& value, fdk::TimeValue time = fdk::defaultTimeValue());
    fdk::Vec2f getClippingRange(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createExposureAttr(const Value& defaultValue = Value());
    Attribute getExposureAttr() const;
    void setExposure(float value, fdk::TimeValue time = fdk::defaultTimeValue());
    float getExposure(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createFocalLengthAttr(const Value& defaultValue = Value());
    Attribute getFocalLengthAttr() const;
    void setFocalLength(float value, fdk::TimeValue time = fdk::defaultTimeValue());
    float getFocalLength(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createFocusDistanceAttr(const Value& defaultValue = Value());
    Attribute getFocusDistanceAttr() const;
    void setFocusDistance(float value, fdk::TimeValue time = fdk::defaultTimeValue());
    float getFocusDistance(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createFStopAttr(const Value& defaultValue = Value());
    Attribute getFStopAttr() const;
    void setFStop(float value, fdk::TimeValue time = fdk::defaultTimeValue());
    float getFStop(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createHorizontalApertureAttr(const Value& defaultValue = Value());
    Attribute getHorizontalApertureAttr() const;
    void setHorizontalAperture(float value, fdk::TimeValue time = fdk::defaultTimeValue());
    float getHorizontalAperture(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createHorizontalApertureOffsetAttr(const Value& defaultValue = Value());
    Attribute getHorizontalApertureOffsetAttr() const;
    void setHorizontalApertureOffset(float value, fdk::TimeValue time = fdk::defaultTimeValue());
    float getHorizontalApertureOffset(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createProjectionAttr(const Value& defaultValue = Value());
    Attribute getProjectionAttr() const;
    void setProjection(const Token& value, fdk::TimeValue time = fdk::defaultTimeValue());
    Token getProjection(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createCloseAttr(const Value& defaultValue = Value());
    Attribute getCloseAttr() const;
    void setClose(double value, fdk::TimeValue time = fdk::defaultTimeValue());
    double getClose(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createOpenAttr(const Value& defaultValue = Value());
    Attribute getOpenAttr() const;
    void setOpen(double value, fdk::TimeValue time = fdk::defaultTimeValue());
    double getOpen(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createStereoRoleAttr(const Value& defaultValue = Value());
    Attribute getStereoRoleAttr() const;
    void setStereoRole(const Token& value, fdk::TimeValue time = fdk::defaultTimeValue());
    Token getStereoRole(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createVerticalApertureAttr(const Value& defaultValue = Value());
    Attribute getVerticalApertureAttr() const;
    void setVerticalAperture(float value, fdk::TimeValue time = fdk::defaultTimeValue());
    float getVerticalAperture(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createVerticalApertureOffsetAttr(const Value& defaultValue = Value());
    Attribute getVerticalApertureOffsetAttr() const;
    void setVerticalApertureOffset(float value, fdk::TimeValue time = fdk::defaultTimeValue());
    float getVerticalApertureOffset(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    // --(BEGIN CUSTOM CODE)--
    //! Returns the world-space frustum points ordered as
    //!  - Left bottom near
    //!  - Right bottom near
    //!  - Right top near
    //!  - Left top near
    //!  - Left bottom far
    //!  - Right bottom far
    //!  - Right top far
    //!  - Left top far
    std::array<fdk::Vec3d, 8> getFrustumPoints(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    //! Returns the camera's projection matrix
    fdk::Mat4d getProjectionMatrix(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    // --(END CUSTOM CODE)--
  };

}

#endif
