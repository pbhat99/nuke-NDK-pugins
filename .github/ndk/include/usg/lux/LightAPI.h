// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

// This is automatically-generated code.
// You may add custom code between the --(BEGIN CUSTOM CODE)-- and --(END CUSTOM CODE)-- markers.
// Code added anywhere else will be destroyed when the code is generated.

#ifndef USG_LUX_LIGHTAPI_H
#define USG_LUX_LIGHTAPI_H

#include "usg/api.h"

#include "usg/lux/LuxTokens.h"

#include "usg/geom/Prim.h"
#include "usg/geom/CollectionAPI.h"
#include "usg/base/ArrayTypes.h"
#include "fdk/math/Vec3.h"

namespace usg {
namespace lux {

  // class LightAPI
  //
  // Attributes:
  //     color3f inputs:color = (1, 1, 1)
  //     float inputs:colorTemperature = 6500.0
  //     float inputs:diffuse = 1.0
  //     bool inputs:enableColorTemperature
  //     float inputs:exposure
  //     float inputs:intensity = 1.0
  //     bool inputs:normalize
  //     float inputs:specular = 1.0
  //     rel light:filters
  //     uniform token light:materialSyncMode = "noMaterialResponse" (allowed values: materialGlowTintsLight, independent, noMaterialResponse)
  //     uniform token light:shaderId

  class USG_API LightAPI {
  public:
    LightAPI();
    LightAPI(const Prim& prim);
    ~LightAPI();

    bool validate();
    static bool isValidPrim(const Prim& prim);
    explicit operator bool() const { return isValidPrim(_prim); }

    void apply();

    Attribute createColorAttr(const Value& defaultValue = Value());
    Attribute getColorAttr() const;
    void setColor(const fdk::Vec3f& value, fdk::TimeValue time = fdk::defaultTimeValue());
    fdk::Vec3f getColor(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createColorTemperatureAttr(const Value& defaultValue = Value());
    Attribute getColorTemperatureAttr() const;
    void setColorTemperature(float value, fdk::TimeValue time = fdk::defaultTimeValue());
    float getColorTemperature(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createDiffuseAttr(const Value& defaultValue = Value());
    Attribute getDiffuseAttr() const;
    void setDiffuse(float value, fdk::TimeValue time = fdk::defaultTimeValue());
    float getDiffuse(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createEnableColorTemperatureAttr(const Value& defaultValue = Value());
    Attribute getEnableColorTemperatureAttr() const;
    void setEnableColorTemperature(bool value, fdk::TimeValue time = fdk::defaultTimeValue());
    bool getEnableColorTemperature(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createExposureAttr(const Value& defaultValue = Value());
    Attribute getExposureAttr() const;
    void setExposure(float value, fdk::TimeValue time = fdk::defaultTimeValue());
    float getExposure(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createIntensityAttr(const Value& defaultValue = Value());
    Attribute getIntensityAttr() const;
    void setIntensity(float value, fdk::TimeValue time = fdk::defaultTimeValue());
    float getIntensity(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createNormalizeAttr(const Value& defaultValue = Value());
    Attribute getNormalizeAttr() const;
    void setNormalize(bool value, fdk::TimeValue time = fdk::defaultTimeValue());
    bool getNormalize(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createSpecularAttr(const Value& defaultValue = Value());
    Attribute getSpecularAttr() const;
    void setSpecular(float value, fdk::TimeValue time = fdk::defaultTimeValue());
    float getSpecular(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Relationship createFiltersRel();
    Relationship getFiltersRel() const;
    void setFilters(const PathArray& value);
    PathArray getFilters() const;

    Attribute createMaterialSyncModeAttr(const Value& defaultValue = Value());
    Attribute getMaterialSyncModeAttr() const;
    void setMaterialSyncMode(const Token& value, fdk::TimeValue time = fdk::defaultTimeValue());
    Token getMaterialSyncMode(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createShaderIdAttr(const Value& defaultValue = Value());
    Attribute getShaderIdAttr() const;
    void setShaderId(const Token& value, fdk::TimeValue time = fdk::defaultTimeValue());
    Token getShaderId(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    CollectionAPI getLightLink() const;

    CollectionAPI getShadowLink() const;

    // --(BEGIN CUSTOM CODE)--
    // --(END CUSTOM CODE)--
  protected:
    Prim _prim;
  };

}
}

#endif
