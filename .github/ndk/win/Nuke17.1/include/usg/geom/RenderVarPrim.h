// Copyright (c) 2026 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef USG_GEOM_RENDERVARPRIM_H
#define USG_GEOM_RENDERVARPRIM_H

#include "usg/api.h"

#include "usg/geom/Prim.h"

namespace usg {
  // class RenderVarPrim
  //   inherits from Prim
  //
  // Attributes:
  //     uniform token dataType = "color3f"
  //     string sourceName
  //     uniform token sourceType = "raw" (allowed values: raw, primvar, lpe, intrinsic)

  class USG_API RenderVarPrim : public Prim {
  public:
    RenderVarPrim() = default;
    explicit RenderVarPrim(const Prim& prim);
    ~RenderVarPrim() override = default;

    bool validate() override;
    static bool isValidPrim(const Prim& prim);

    static RenderVarPrim defineInLayer(const LayerRef& layer, const Path& path);
    static RenderVarPrim overrideInLayer(const LayerRef& layer, const Prim& prim);
    static RenderVarPrim getInStage(const StageRef& stage, const Path& path);

    Attribute createDataTypeAttr(const Value& defaultValue = Value());
    Attribute getDataTypeAttr() const;
    void setDataType(const Token& value, fdk::TimeValue time = fdk::defaultTimeValue());
    Token getDataType(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createSourceNameAttr(const Value& defaultValue = Value());
    Attribute getSourceNameAttr() const;
    void setSourceName(const std::string& value, fdk::TimeValue time = fdk::defaultTimeValue());
    std::string getSourceName(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createSourceTypeAttr(const Value& defaultValue = Value());
    Attribute getSourceTypeAttr() const;
    void setSourceType(const Token& value, fdk::TimeValue time = fdk::defaultTimeValue());
    Token getSourceType(fdk::TimeValue time = fdk::defaultTimeValue()) const;
  };
}  // namespace usg

#endif  // USG_GEOM_RENDERVARPRIM_H
