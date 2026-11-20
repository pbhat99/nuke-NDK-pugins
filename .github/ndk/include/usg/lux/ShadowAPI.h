// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

// This is automatically-generated code.
// You may add custom code between the --(BEGIN CUSTOM CODE)-- and --(END CUSTOM CODE)-- markers.
// Code added anywhere else will be destroyed when the code is generated.

#ifndef USG_LUX_SHADOWAPI_H
#define USG_LUX_SHADOWAPI_H

#include "usg/api.h"

#include "usg/lux/LuxTokens.h"

#include "usg/geom/Prim.h"
#include "fdk/math/Vec3.h"

namespace usg {
namespace lux {

  // class ShadowAPI
  //
  // Attributes:
  //     color3f inputs:shadow:color = (0, 0, 0)
  //     float inputs:shadow:distance = -1.0
  //     bool inputs:shadow:enable = True
  //     float inputs:shadow:falloff = -1.0
  //     float inputs:shadow:falloffGamma = 1.0

  class USG_API ShadowAPI {
  public:
    ShadowAPI();
    ShadowAPI(const Prim& prim);
    ~ShadowAPI();

    bool validate();
    static bool isValidPrim(const Prim& prim);
    explicit operator bool() const { return isValidPrim(_prim); }

    void apply();

    Attribute createShadowColorAttr(const Value& defaultValue = Value());
    Attribute getShadowColorAttr() const;
    void setShadowColor(const fdk::Vec3f& value, fdk::TimeValue time = fdk::defaultTimeValue());
    fdk::Vec3f getShadowColor(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createShadowDistanceAttr(const Value& defaultValue = Value());
    Attribute getShadowDistanceAttr() const;
    void setShadowDistance(float value, fdk::TimeValue time = fdk::defaultTimeValue());
    float getShadowDistance(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createShadowEnableAttr(const Value& defaultValue = Value());
    Attribute getShadowEnableAttr() const;
    void setShadowEnable(bool value, fdk::TimeValue time = fdk::defaultTimeValue());
    bool getShadowEnable(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createShadowFalloffAttr(const Value& defaultValue = Value());
    Attribute getShadowFalloffAttr() const;
    void setShadowFalloff(float value, fdk::TimeValue time = fdk::defaultTimeValue());
    float getShadowFalloff(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createShadowFalloffGammaAttr(const Value& defaultValue = Value());
    Attribute getShadowFalloffGammaAttr() const;
    void setShadowFalloffGamma(float value, fdk::TimeValue time = fdk::defaultTimeValue());
    float getShadowFalloffGamma(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    // --(BEGIN CUSTOM CODE)--
    // --(END CUSTOM CODE)--
  protected:
    Prim _prim;
  };

}
}

#endif
