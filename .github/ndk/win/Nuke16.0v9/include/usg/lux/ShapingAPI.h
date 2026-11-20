// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

// This is automatically-generated code.
// You may add custom code between the --(BEGIN CUSTOM CODE)-- and --(END CUSTOM CODE)-- markers.
// Code added anywhere else will be destroyed when the code is generated.

#ifndef USG_LUX_SHAPINGAPI_H
#define USG_LUX_SHAPINGAPI_H

#include "usg/api.h"

#include "usg/lux/LuxTokens.h"

#include "usg/geom/Prim.h"
#include "fdk/math/Vec3.h"

namespace usg {
namespace lux {

  // class ShapingAPI
  //
  // Attributes:
  //     float inputs:shaping:cone:angle = 90.0
  //     float inputs:shaping:cone:softness
  //     float inputs:shaping:focus
  //     color3f inputs:shaping:focusTint = (0, 0, 0)
  //     float inputs:shaping:ies:angleScale
  //     asset inputs:shaping:ies:file
  //     bool inputs:shaping:ies:normalize

  class USG_API ShapingAPI {
  public:
    ShapingAPI();
    ShapingAPI(const Prim& prim);
    ~ShapingAPI();

    bool validate();
    static bool isValidPrim(const Prim& prim);
    explicit operator bool() const { return isValidPrim(_prim); }

    void apply();

    Attribute createShapingConeAngleAttr(const Value& defaultValue = Value());
    Attribute getShapingConeAngleAttr() const;
    void setShapingConeAngle(float value, fdk::TimeValue time = fdk::defaultTimeValue());
    float getShapingConeAngle(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createShapingConeSoftnessAttr(const Value& defaultValue = Value());
    Attribute getShapingConeSoftnessAttr() const;
    void setShapingConeSoftness(float value, fdk::TimeValue time = fdk::defaultTimeValue());
    float getShapingConeSoftness(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createShapingFocusAttr(const Value& defaultValue = Value());
    Attribute getShapingFocusAttr() const;
    void setShapingFocus(float value, fdk::TimeValue time = fdk::defaultTimeValue());
    float getShapingFocus(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createShapingFocusTintAttr(const Value& defaultValue = Value());
    Attribute getShapingFocusTintAttr() const;
    void setShapingFocusTint(const fdk::Vec3f& value, fdk::TimeValue time = fdk::defaultTimeValue());
    fdk::Vec3f getShapingFocusTint(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createShapingIesAngleScaleAttr(const Value& defaultValue = Value());
    Attribute getShapingIesAngleScaleAttr() const;
    void setShapingIesAngleScale(float value, fdk::TimeValue time = fdk::defaultTimeValue());
    float getShapingIesAngleScale(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createShapingIesFileAttr(const Value& defaultValue = Value());
    Attribute getShapingIesFileAttr() const;
    void setShapingIesFile(const AssetPath& value, fdk::TimeValue time = fdk::defaultTimeValue());
    AssetPath getShapingIesFile(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createShapingIesNormalizeAttr(const Value& defaultValue = Value());
    Attribute getShapingIesNormalizeAttr() const;
    void setShapingIesNormalize(bool value, fdk::TimeValue time = fdk::defaultTimeValue());
    bool getShapingIesNormalize(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    // --(BEGIN CUSTOM CODE)--
    // --(END CUSTOM CODE)--
  protected:
    Prim _prim;
  };

}
}

#endif
