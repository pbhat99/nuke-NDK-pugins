// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

// This is automatically-generated code.
// You may add custom code between the --(BEGIN CUSTOM CODE)-- and --(END CUSTOM CODE)-- markers.
// Code added anywhere else will be destroyed when the code is generated.

#ifndef USG_LUX_PLUGINLIGHTPRIM_H
#define USG_LUX_PLUGINLIGHTPRIM_H

#include "usg/api.h"

#include "usg/lux/LuxTokens.h"

#include "usg/geom/Prim.h"
#include "usg/geom/CollectionAPI.h"
#include "usg/base/ArrayTypes.h"
#include "fdk/math/Vec3.h"

namespace usg {
namespace lux {

  // class PluginLightPrim
  //   inherits from Prim
  //
  // Attributes:
  //     rel filters
  //     uniform token info:id
  //     uniform token info:implementationSource = "id"
  //     color3f inputs:color = (1, 1, 1)
  //     float inputs:colorTemperature = 6500.0
  //     float inputs:diffuse = 1.0
  //     bool inputs:enableColorTemperature
  //     float inputs:exposure
  //     float inputs:intensity = 1.0
  //     bool inputs:normalize
  //     float inputs:specular = 1.0
  //     rel proxyPrim
  //     uniform token purpose = "default"
  //     token visibility = "inherited"
  //     uniform token[] xformOpOrder

  class USG_API PluginLightPrim : public Prim {
  public:
    PluginLightPrim();
    PluginLightPrim(const Prim& prim);
    ~PluginLightPrim();

    bool validate();
    static bool isValidPrim(const Prim& prim);
    static PluginLightPrim defineInLayer(const LayerRef& layer, const Path& path);
    static PluginLightPrim overrideInLayer(const LayerRef& layer, const Prim& prim);
    static PluginLightPrim getInStage(const StageRef& stage, const Path& path);

    Relationship createFiltersRel();
    Relationship getFiltersRel() const;
    void setFilters(const PathArray& value);
    PathArray getFilters() const;

    Attribute createIdAttr(const Value& defaultValue = Value());
    Attribute getIdAttr() const;
    void setId(const Token& value, fdk::TimeValue time = fdk::defaultTimeValue());
    Token getId(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createImplementationSourceAttr(const Value& defaultValue = Value());
    Attribute getImplementationSourceAttr() const;
    void setImplementationSource(const Token& value, fdk::TimeValue time = fdk::defaultTimeValue());
    Token getImplementationSource(fdk::TimeValue time = fdk::defaultTimeValue()) const;

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

    Relationship createProxyPrimRel();
    Relationship getProxyPrimRel() const;
    void setProxyPrim(const PathArray& value);
    PathArray getProxyPrim() const;

    Attribute createPurposeAttr(const Value& defaultValue = Value());
    Attribute getPurposeAttr() const;
    void setPurpose(const Token& value, fdk::TimeValue time = fdk::defaultTimeValue());
    Token getPurpose(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createVisibilityAttr(const Value& defaultValue = Value());
    Attribute getVisibilityAttr() const;
    void setVisibility(const Token& value, fdk::TimeValue time = fdk::defaultTimeValue());
    Token getVisibility(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createXformOpOrderAttr(const Value& defaultValue = Value());
    Attribute getXformOpOrderAttr() const;
    void setXformOpOrder(const TokenArray& value, fdk::TimeValue time = fdk::defaultTimeValue());
    TokenArray getXformOpOrder(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    CollectionAPI getLightLink() const;

    CollectionAPI getShadowLink() const;

    // --(BEGIN CUSTOM CODE)--
    // --(END CUSTOM CODE)--
  };

}
}

#endif
