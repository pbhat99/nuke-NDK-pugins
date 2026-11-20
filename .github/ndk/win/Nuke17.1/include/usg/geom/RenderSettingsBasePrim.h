// Copyright (c) 2026 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef USG_GEOM_RENDERSETTINGSBASEPRIM_H
#define USG_GEOM_RENDERSETTINGSBASEPRIM_H

#include "usg/api.h"

#include "usg/geom/Prim.h"
#include "usg/base/ArrayTypes.h"
#include "fdk/math/Vec2.h"
#include "fdk/math/Vec4.h"

namespace usg {
  // class RenderSettingsBasePrim
  //   inherits from Prim
  //
  // Attributes:
  //     uniform token aspectRatioConformPolicy
  //     rel camera
  //     float4 dataWindowNDC
  //     bool disableDepthOfField = false
  //     bool disableMotionBlur = false
  //     bool instantaneousShutter = false
  //     float pixelAspectRatio = 1.0
  //     int2 resolution = (2048, 1080)

  class USG_API RenderSettingsBasePrim : public Prim {
  public:
    RenderSettingsBasePrim() = default;
    explicit RenderSettingsBasePrim(const Prim& prim);
    ~RenderSettingsBasePrim() override = default;

    bool validate() override;
    static bool isValidPrim(const Prim& prim);

    static RenderSettingsBasePrim overrideInLayer(const LayerRef& layer, const Prim& prim);
    static RenderSettingsBasePrim getInStage(const StageRef& stage, const Path& path);

    Attribute createAspectRatioConformPolicyAttr(const Value& defaultValue = Value());
    Attribute getAspectRatioConformPolicyAttr() const;
    void setAspectRatioConformPolicy(const Token& value, fdk::TimeValue time = fdk::defaultTimeValue());
    Token getAspectRatioConformPolicy(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Relationship createCameraRel();
    Relationship getCameraRel() const;
    void setCamera(const PathArray& value);
    PathArray getCamera() const;

    Attribute createDataWindowNDCAttr(const Value& defaultValue = Value());
    Attribute getDataWindowNDCAttr() const;
    void setDataWindowNDC(const fdk::Vec4f& value, fdk::TimeValue time = fdk::defaultTimeValue());
    fdk::Vec4f getDataWindowNDC(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createDisableDepthOfFieldAttr(const Value& defaultValue = Value());
    Attribute getDisableDepthOfFieldAttr() const;
    void setDisableDepthOfField(bool value, fdk::TimeValue time = fdk::defaultTimeValue());
    bool getDisableDepthOfField(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createDisableMotionBlurAttr(const Value& defaultValue = Value());
    Attribute getDisableMotionBlurAttr() const;
    void setDisableMotionBlur(bool value, fdk::TimeValue time = fdk::defaultTimeValue());
    bool getDisableMotionBlur(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createInstantaneousShutterAttr(const Value& defaultValue = Value());
    Attribute getInstantaneousShutterAttr() const;
    void setInstantaneousShutter(bool value, fdk::TimeValue time = fdk::defaultTimeValue());
    bool getInstantaneousShutter(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createPixelAspectRatioAttr(const Value& defaultValue = Value());
    Attribute getPixelAspectRatioAttr() const;
    void setPixelAspectRatio(float value, fdk::TimeValue time = fdk::defaultTimeValue());
    float getPixelAspectRatio(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createResolutionAttr(const Value& defaultValue = Value());
    Attribute getResolutionAttr() const;
    void setResolution(fdk::Vec2i value, fdk::TimeValue time = fdk::defaultTimeValue());
    fdk::Vec2i getResolution(fdk::TimeValue time = fdk::defaultTimeValue()) const;
  };
}  // namespace usg

#endif  // USG_GEOM_RENDERSETTINGSBASEPRIM_H
