// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

// This is automatically-generated code.
// You may add custom code between the --(BEGIN CUSTOM CODE)-- and --(END CUSTOM CODE)-- markers.
// Code added anywhere else will be destroyed when the code is generated.

#ifndef USG_LUX_GEOMETRYLIGHTPRIM_H
#define USG_LUX_GEOMETRYLIGHTPRIM_H

#include "usg/api.h"

#include "usg/lux/LuxTokens.h"

#include "usg/geom/Prim.h"
#include "usg/geom/CollectionAPI.h"
#include "usg/base/ArrayTypes.h"
#include "fdk/math/Vec3.h"
#include "usg/lux/NonboundableLightBasePrim.h"

namespace usg {
namespace lux {

  // class GeometryLightPrim
  //   inherits from NonboundableLightBase
  //
  // Attributes:
  //     rel filters
  //     rel geometry
  //     color3f inputs:color = (1, 1, 1)
  //     float inputs:colorTemperature = 6500.0
  //     float inputs:diffuse = 1.0
  //     bool inputs:enableColorTemperature
  //     float inputs:exposure
  //     float inputs:intensity = 1.0
  //     bool inputs:normalize
  //     float inputs:specular = 1.0
  //     uniform token light:shaderId = "GeometryLight"
  //     rel proxyPrim
  //     uniform token purpose = "default"
  //     token visibility = "inherited"
  //     uniform token[] xformOpOrder
  //
  // Inherited attributes:
  //   From NonboundableLightBase:

  class USG_API GeometryLightPrim : public NonboundableLightBasePrim {
  public:
    GeometryLightPrim();
    GeometryLightPrim(const Prim& prim);
    ~GeometryLightPrim();

    bool validate();
    static bool isValidPrim(const Prim& prim);
    static GeometryLightPrim defineInLayer(const LayerRef& layer, const Path& path);
    static GeometryLightPrim overrideInLayer(const LayerRef& layer, const Prim& prim);
    static GeometryLightPrim getInStage(const StageRef& stage, const Path& path);

    Relationship createFiltersRel();
    Relationship getFiltersRel() const;
    void setFilters(const PathArray& value);
    PathArray getFilters() const;

    Relationship createGeometryRel();
    Relationship getGeometryRel() const;
    void setGeometry(const PathArray& value);
    PathArray getGeometry() const;

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

    Attribute createShaderIdAttr(const Value& defaultValue = Value());
    Attribute getShaderIdAttr() const;
    void setShaderId(const Token& value, fdk::TimeValue time = fdk::defaultTimeValue());
    Token getShaderId(fdk::TimeValue time = fdk::defaultTimeValue()) const;

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
