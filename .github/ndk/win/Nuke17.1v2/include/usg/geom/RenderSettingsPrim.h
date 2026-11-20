// Copyright (c) 2026 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef USG_GEOM_RENDERSETTINGSPRIM_H
#define USG_GEOM_RENDERSETTINGSPRIM_H

#include "usg/api.h"

#include "usg/geom/RenderSettingsBasePrim.h"

namespace usg {
  // class RenderSettingsPrim
  //   inherits from RenderSettingsBase
  //
  // Attributes:
  //     uniform token[] includedPurposes = ["geometry", "render"]
  //     uniform token[] materialBindingPurposes = ["full", "allPurpose"]
  //     rel products
  //     uniform token renderingColorSpace
  //
  // Inherited attributes:
  //   From RenderSettingsBase:
  //     uniform token aspectRatioConformPolicy
  //     rel camera
  //     float4 dataWindowNDC
  //     bool disableDepthOfField
  //     bool disableMotionBlur
  //     bool instantaneousShutter
  //     float pixelAspectRatio
  //     int2 resolution

  class USG_API RenderSettingsPrim : public RenderSettingsBasePrim {
  public:
    RenderSettingsPrim() = default;
    explicit RenderSettingsPrim(const Prim& prim);
    ~RenderSettingsPrim() override = default;

    bool validate() override;
    static bool isValidPrim(const Prim& prim);

    static RenderSettingsPrim defineInLayer(const LayerRef& layer, const Path& path);
    static RenderSettingsPrim overrideInLayer(const LayerRef& layer, const Prim& prim);
    static RenderSettingsPrim getInStage(const StageRef& stage, const Path& path);

    Attribute createIncludedPurposesAttr(const Value& defaultValue = Value());
    Attribute getIncludedPurposesAttr() const;
    void setIncludedPurposes(const TokenArray& value, fdk::TimeValue time = fdk::defaultTimeValue());
    TokenArray getIncludedPurposes(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createMaterialBindingPurposesAttr(const Value& defaultValue = Value());
    Attribute getMaterialBindingPurposesAttr() const;
    void setMaterialBindingPurposes(const TokenArray& value, fdk::TimeValue time = fdk::defaultTimeValue());
    TokenArray getMaterialBindingPurposes(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Relationship createProductsRel();
    Relationship getProductsRel() const;
    void setProducts(const PathArray& value);
    PathArray getProducts() const;

    Attribute createRenderingColorSpaceAttr(const Value& defaultValue = Value());
    Attribute getRenderingColorSpaceAttr() const;
    void setRenderingColorSpace(const Token& value, fdk::TimeValue time = fdk::defaultTimeValue());
    Token getRenderingColorSpace(fdk::TimeValue time = fdk::defaultTimeValue()) const;
  };
}  // namespace usg

#endif  // USG_GEOM_RENDERSETTINGSPRIM_H
