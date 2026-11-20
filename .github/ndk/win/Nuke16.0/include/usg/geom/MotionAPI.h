// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

// This is automatically-generated code.
// You may add custom code between the --(BEGIN CUSTOM CODE)-- and --(END CUSTOM CODE)-- markers.
// Code added anywhere else will be destroyed when the code is generated.

#ifndef USG_GEOM_MOTIONAPI_H
#define USG_GEOM_MOTIONAPI_H

#include "usg/api.h"

#include "usg/geom/GeomTokens.h"

#include "usg/geom/Prim.h"

namespace usg {

  // class MotionAPI
  //
  // Attributes:
  //     float motion:blurScale = 1.0
  //     int motion:nonlinearSampleCount = 3
  //     float motion:velocityScale = 1.0

  class USG_API MotionAPI {
  public:
    MotionAPI();
    MotionAPI(const Prim& prim);
    ~MotionAPI();

    bool validate();
    static bool isValidPrim(const Prim& prim);
    explicit operator bool() const { return isValidPrim(_prim); }

    void apply();

    Attribute createBlurScaleAttr(const Value& defaultValue = Value());
    Attribute getBlurScaleAttr() const;
    void setBlurScale(float value, fdk::TimeValue time = fdk::defaultTimeValue());
    float getBlurScale(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createNonlinearSampleCountAttr(const Value& defaultValue = Value());
    Attribute getNonlinearSampleCountAttr() const;
    void setNonlinearSampleCount(int value, fdk::TimeValue time = fdk::defaultTimeValue());
    int getNonlinearSampleCount(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createVelocityScaleAttr(const Value& defaultValue = Value());
    Attribute getVelocityScaleAttr() const;
    void setVelocityScale(float value, fdk::TimeValue time = fdk::defaultTimeValue());
    float getVelocityScale(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    // --(BEGIN CUSTOM CODE)--
    // --(END CUSTOM CODE)--
  protected:
    Prim _prim;
  };

}

#endif
