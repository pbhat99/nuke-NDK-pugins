// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

// This is automatically-generated code.
// You may add custom code between the --(BEGIN CUSTOM CODE)-- and --(END CUSTOM CODE)-- markers.
// Code added anywhere else will be destroyed when the code is generated.

#ifndef USG_LUX_PLUGINLIGHTFILTERPRIM_H
#define USG_LUX_PLUGINLIGHTFILTERPRIM_H

#include "usg/api.h"

#include "usg/lux/LuxTokens.h"

#include "usg/geom/Prim.h"
#include "usg/geom/CollectionAPI.h"
#include "usg/lux/LightFilterPrim.h"

namespace usg {
namespace lux {

  // class PluginLightFilterPrim
  //   inherits from LightFilter
  //
  // Attributes:
  //     uniform token info:id
  //     uniform token info:implementationSource = "id"
  //
  // Inherited attributes:
  //   From LightFilter:
  //     rel collection:filterLink:excludes
  //     uniform token collection:filterLink:expansionRule = "expandPrims"
  //     uniform bool collection:filterLink:includeRoot = True
  //     rel collection:filterLink:includes
  //     uniform token lightFilter:shaderId
  //     rel proxyPrim
  //     uniform token purpose = "default"
  //     token visibility = "inherited"
  //     uniform token[] xformOpOrder

  class USG_API PluginLightFilterPrim : public LightFilterPrim {
  public:
    PluginLightFilterPrim();
    PluginLightFilterPrim(const Prim& prim);
    ~PluginLightFilterPrim();

    bool validate();
    static bool isValidPrim(const Prim& prim);
    static PluginLightFilterPrim defineInLayer(const LayerRef& layer, const Path& path);
    static PluginLightFilterPrim overrideInLayer(const LayerRef& layer, const Prim& prim);
    static PluginLightFilterPrim getInStage(const StageRef& stage, const Path& path);

    Attribute createIdAttr(const Value& defaultValue = Value());
    Attribute getIdAttr() const;
    void setId(const Token& value, fdk::TimeValue time = fdk::defaultTimeValue());
    Token getId(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createImplementationSourceAttr(const Value& defaultValue = Value());
    Attribute getImplementationSourceAttr() const;
    void setImplementationSource(const Token& value, fdk::TimeValue time = fdk::defaultTimeValue());
    Token getImplementationSource(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    CollectionAPI getFilterLink() const;

    // --(BEGIN CUSTOM CODE)--
    // --(END CUSTOM CODE)--
  };

}
}

#endif
