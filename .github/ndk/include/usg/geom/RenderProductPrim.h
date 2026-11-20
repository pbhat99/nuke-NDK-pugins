// Copyright (c) 2026 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef USG_GEOM_RENDERPRODUCTPRIM_H
#define USG_GEOM_RENDERPRODUCTPRIM_H

#include "usg/api.h"

#include "usg/geom/RenderSettingsBasePrim.h"

namespace usg {
  // class RenderProductPrim
  //   inherits from RenderSettingsBase
  //
  // Attributes:
  //     rel orderedVars
  //     token productName
  //     uniform token productType = "raster"
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

  class USG_API RenderProductPrim : public RenderSettingsBasePrim {
  public:
    RenderProductPrim() = default;
    explicit RenderProductPrim(const Prim& prim);
    ~RenderProductPrim() override = default;

    bool validate() override;
    static bool isValidPrim(const Prim& prim);

    static RenderProductPrim defineInLayer(const LayerRef& layer, const Path& path);
    static RenderProductPrim overrideInLayer(const LayerRef& layer, const Prim& prim);
    static RenderProductPrim getInStage(const StageRef& stage, const Path& path);

    Relationship createOrderedVarsRel();
    Relationship getOrderedVarsRel() const;
    void setOrderedVars(const PathArray& value);
    PathArray getOrderedVars() const;

    Attribute createProductNameAttr(const Value& defaultValue = Value());
    Attribute getProductNameAttr() const;
    void setProductName(const Token& value, fdk::TimeValue time = fdk::defaultTimeValue());
    Token getProductName(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createProductTypeAttr(const Value& defaultValue = Value());
    Attribute getProductTypeAttr() const;
    void setProductType(const Token& value, fdk::TimeValue time = fdk::defaultTimeValue());
    Token getProductType(fdk::TimeValue time = fdk::defaultTimeValue()) const;
  };
}  // namespace usg

#endif  // USG_GEOM_RENDERPRODUCTPRIM_H
