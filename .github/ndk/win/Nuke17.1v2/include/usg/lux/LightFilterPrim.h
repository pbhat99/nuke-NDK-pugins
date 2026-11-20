// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

// This is automatically-generated code.
// You may add custom code between the --(BEGIN CUSTOM CODE)-- and --(END CUSTOM CODE)-- markers.
// Code added anywhere else will be destroyed when the code is generated.

#ifndef USG_LUX_LIGHTFILTERPRIM_H
#define USG_LUX_LIGHTFILTERPRIM_H

#include "usg/api.h"

#include "usg/lux/LuxTokens.h"

#include "usg/geom/Prim.h"
#include "usg/geom/CollectionAPI.h"
#include "usg/base/ArrayTypes.h"

namespace usg {
namespace lux {

  // class LightFilterPrim
  //   inherits from Prim
  //
  // Attributes:
  //     uniform token lightFilter:shaderId
  //     rel proxyPrim
  //     uniform token purpose = "default"
  //     token visibility = "inherited"
  //     uniform token[] xformOpOrder

  class USG_API LightFilterPrim : public Prim {
  public:
    LightFilterPrim();
    LightFilterPrim(const Prim& prim);
    ~LightFilterPrim();

    bool validate();
    static bool isValidPrim(const Prim& prim);
    static LightFilterPrim overrideInLayer(const LayerRef& layer, const Prim& prim);
    static LightFilterPrim getInStage(const StageRef& stage, const Path& path);

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

    CollectionAPI getFilterLink() const;

    // --(BEGIN CUSTOM CODE)--
    // --(END CUSTOM CODE)--
  };

}
}

#endif
