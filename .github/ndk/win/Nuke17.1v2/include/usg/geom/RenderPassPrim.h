// Copyright (c) 2026 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef USG_GEOM_RENDERPASSPRIM_H
#define USG_GEOM_RENDERPASSPRIM_H

#include "usg/api.h"

#include "usg/geom/Prim.h"
#include "usg/geom/AssetPath.h"
#include "usg/base/ArrayTypes.h"

namespace usg {
  // class RenderPassPrim
  //   inherits from Prim
  //
  // Collections:
  //     collection:cameraVisibility (excludes, expansionRule, includeRoot, includes, mode)
  //     collection:matte            (excludes, expansionRule, includeRoot, includes, mode)
  //     collection:prune            (excludes, expansionRule, includeRoot, includes, mode)
  //     collection:renderVisibility (excludes, expansionRule, includeRoot, includes, mode)
  //
  // Attributes:
  //     string[] command
  //     asset fileName
  //     rel inputPasses
  //     uniform token passType
  //     rel renderSource

  class USG_API RenderPassPrim : public Prim {
  public:
    RenderPassPrim() = default;
    explicit RenderPassPrim(const Prim& prim);
    ~RenderPassPrim() override = default;

    bool validate() override;
    static bool isValidPrim(const Prim& prim);

    static RenderPassPrim defineInLayer(const LayerRef& layer, const Path& path);
    static RenderPassPrim overrideInLayer(const LayerRef& layer, const Prim& prim);
    static RenderPassPrim getInStage(const StageRef& stage, const Path& path);

    Relationship createCameraVisibilityExcludesRel();
    Relationship getCameraVisibilityExcludesRel() const;
    void setCameraVisibilityExcludes(const PathArray& value);
    PathArray getCameraVisibilityExcludes() const;

    Attribute createCameraVisibilityExpansionRuleAttr(const Value& defaultValue = Value());
    Attribute getCameraVisibilityExpansionRuleAttr() const;
    void setCameraVisibilityExpansionRule(const Token& value, fdk::TimeValue time = fdk::defaultTimeValue());
    Token getCameraVisibilityExpansionRule(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createCameraVisibilityIncludeRootAttr(const Value& defaultValue = Value());
    Attribute getCameraVisibilityIncludeRootAttr() const;
    void setCameraVisibilityIncludeRoot(bool value, fdk::TimeValue time = fdk::defaultTimeValue());
    bool getCameraVisibilityIncludeRoot(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Relationship createCameraVisibilityIncludesRel();
    Relationship getCameraVisibilityIncludesRel() const;
    void setCameraVisibilityIncludes(const PathArray& value);
    PathArray getCameraVisibilityIncludes() const;

    Attribute createCameraVisibilityModeAttr(const Value& defaultValue = Value());
    Attribute getCameraVisibilityModeAttr() const;
    void setCameraVisibilityMode(const Token& value, fdk::TimeValue time = fdk::defaultTimeValue());
    Token getCameraVisibilityMode(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Relationship createMatteExcludesRel();
    Relationship getMatteExcludesRel() const;
    void setMatteExcludes(const PathArray& value);
    PathArray getMatteExcludes() const;

    Attribute createMatteExpansionRuleAttr(const Value& defaultValue = Value());
    Attribute getMatteExpansionRuleAttr() const;
    void setMatteExpansionRule(const Token& value, fdk::TimeValue time = fdk::defaultTimeValue());
    Token getMatteExpansionRule(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createMatteIncludeRootAttr(const Value& defaultValue = Value());
    Attribute getMatteIncludeRootAttr() const;
    void setMatteIncludeRoot(bool value, fdk::TimeValue time = fdk::defaultTimeValue());
    bool getMatteIncludeRoot(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Relationship createMatteIncludesRel();
    Relationship getMatteIncludesRel() const;
    void setMatteIncludes(const PathArray& value);
    PathArray getMatteIncludes() const;

    Attribute createMatteModeAttr(const Value& defaultValue = Value());
    Attribute getMatteModeAttr() const;
    void setMatteMode(const Token& value, fdk::TimeValue time = fdk::defaultTimeValue());
    Token getMatteMode(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Relationship createPruneExcludesRel();
    Relationship getPruneExcludesRel() const;
    void setPruneExcludes(const PathArray& value);
    PathArray getPruneExcludes() const;

    Attribute createPruneExpansionRuleAttr(const Value& defaultValue = Value());
    Attribute getPruneExpansionRuleAttr() const;
    void setPruneExpansionRule(const Token& value, fdk::TimeValue time = fdk::defaultTimeValue());
    Token getPruneExpansionRule(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createPruneIncludeRootAttr(const Value& defaultValue = Value());
    Attribute getPruneIncludeRootAttr() const;
    void setPruneIncludeRoot(bool value, fdk::TimeValue time = fdk::defaultTimeValue());
    bool getPruneIncludeRoot(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Relationship createPruneIncludesRel();
    Relationship getPruneIncludesRel() const;
    void setPruneIncludes(const PathArray& value);
    PathArray getPruneIncludes() const;

    Attribute createPruneModeAttr(const Value& defaultValue = Value());
    Attribute getPruneModeAttr() const;
    void setPruneMode(const Token& value, fdk::TimeValue time = fdk::defaultTimeValue());
    Token getPruneMode(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Relationship createRenderVisibilityExcludesRel();
    Relationship getRenderVisibilityExcludesRel() const;
    void setRenderVisibilityExcludes(const PathArray& value);
    PathArray getRenderVisibilityExcludes() const;

    Attribute createRenderVisibilityExpansionRuleAttr(const Value& defaultValue = Value());
    Attribute getRenderVisibilityExpansionRuleAttr() const;
    void setRenderVisibilityExpansionRule(const Token& value, fdk::TimeValue time = fdk::defaultTimeValue());
    Token getRenderVisibilityExpansionRule(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createRenderVisibilityIncludeRootAttr(const Value& defaultValue = Value());
    Attribute getRenderVisibilityIncludeRootAttr() const;
    void setRenderVisibilityIncludeRoot(bool value, fdk::TimeValue time = fdk::defaultTimeValue());
    bool getRenderVisibilityIncludeRoot(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Relationship createRenderVisibilityIncludesRel();
    Relationship getRenderVisibilityIncludesRel() const;
    void setRenderVisibilityIncludes(const PathArray& value);
    PathArray getRenderVisibilityIncludes() const;

    Attribute createRenderVisibilityModeAttr(const Value& defaultValue = Value());
    Attribute getRenderVisibilityModeAttr() const;
    void setRenderVisibilityMode(const Token& value, fdk::TimeValue time = fdk::defaultTimeValue());
    Token getRenderVisibilityMode(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createCommandAttr(const Value& defaultValue = Value());
    Attribute getCommandAttr() const;
    void setCommand(const StringArray& value, fdk::TimeValue time = fdk::defaultTimeValue());
    StringArray getCommand(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createFileNameAttr(const Value& defaultValue = Value());
    Attribute getFileNameAttr() const;
    void setFileName(const AssetPath& value, fdk::TimeValue time = fdk::defaultTimeValue());
    AssetPath getFileName(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Relationship createInputPassesRel();
    Relationship getInputPassesRel() const;
    void setInputPasses(const PathArray& value);
    PathArray getInputPasses() const;

    Attribute createPassTypeAttr(const Value& defaultValue = Value());
    Attribute getPassTypeAttr() const;
    void setPassType(const Token& value, fdk::TimeValue time = fdk::defaultTimeValue());
    Token getPassType(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Relationship createRenderSourceRel();
    Relationship getRenderSourceRel() const;
    void setRenderSource(const PathArray& value);
    PathArray getRenderSource() const;
  };
}  // namespace usg

#endif  // USG_GEOM_RENDERPASSPRIM_H
