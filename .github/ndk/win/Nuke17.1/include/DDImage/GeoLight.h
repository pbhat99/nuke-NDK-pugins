// Copyright (c) 2025 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDIMAGE_GEOLIGHT_H
#define DDIMAGE_GEOLIGHT_H

#include "DDImage/CreateOrEditGeomOp.h"
#include "DDImage/GeomOpMixIns.h"
#include "DDImage/KnobMixIns.h"

#include "ndk/graph/private/GeomOpNode.h" // TODO ndk17: change to non-private locale at some point post Nuke 17

#include "DDImage/private/SchemaGeomOpI.h"

#include "usg/geom/Prim.h"
#include "usg/lux/LuxTokens.h"


class DDImage_API GeoLight : public DD::Image::CreateOrEditGeomOp,
                             public DD::Image::TransformGeomOpI,
                             public DD::Image::SchemaGeomOpI
{
public:
  static const char* const kBasicKnobGroup;
  static const char* const kRefineKnobGroup;
  static const char* const kScanlineRenderKnobGroup;
  static const char* const kSplatRenderKnobGroup;

  static const char* const kColorKnobName;
  static const char* const kIntensityKnobName ;
  static const char* const kExposureKnobName;
  static const char* const kNormalizeKnobName;
  static const char* const kEnableColorTemperatureKnobName;
  static const char* const kColorTemperatureKnobName;

  static const char* const kInputsDiffuseKnobName;
  static const char* const kInputsSpecularKnobName;
  
  static const char* const kCastShadowKnobName;
  static const char* const kFalloffTypeKnobName;
  static const char* const kObjectIlluminationMaskKnobName;
  static const char* const kObjectReceiveShadowMaskKnobName;
  static const char* const kShadowChannelsKnobName;

  static const char* const kShadowStrengthKnobName;
  static const char* const kShadowSoftnessKnobName;
  static const char* const kShadowDepthRangeKnobName;
  static const char* const kShadowOnsetThresholdKnobName;
  static const char* const kShadowBiasKnobName;
  static const char* const kShadowResolutionKnobName;
  static const char* const kShadowDepthQualityKnobName;
  static const char* const kShadowCutoffKnobName;


  GeoLight(Node* node, usg::Token typeName, CreateOrEditGeomOp::Mode mode);

  int minimum_inputs() const override;
  int maximum_inputs() const override;
  int optional_input() const override;
  Op* default_input(int input) const override;
  bool test_input(int input, Op* op) const override;
  const char* input_label(int input, char* buffer) const override;

  void knobs(DD::Image::Knob_Callback f) override;
  int knob_changed(DD::Image::Knob* k) override;
  fdk::Mat4d handlesParentTransform(DD::Image::ViewerContext *ctx) override;

  // Handles

  HandlesMode doAnyHandles(DD::Image::ViewerContext* ctx) override;
  void build_handles(DD::Image::ViewerContext*) override;
  void draw_handle(DD::Image::ViewerContext*) override;

protected:

  template<class T>
  T getKnobValue(const std::string_view name) const
  {
    if (const auto engine = dynamic_cast<GeoLight::Engine*>(geomEngine())) {
      if (const auto knobBinding = engine->knob(name.data())) {
        return knobBinding.get<T>(fdk::defaultTimeValue());
      }
    }
    return T();
  }

  static void AddDynamicKnobs(void*, DD::Image::Knob_Callback f);
  void addDynamicKnobs(DD::Image::Knob_Callback f);

  void appendGeomState(DD::Image::Hash& op_hash) override;
  void addDisplayOptionsKnobs(DD::Image::Knob_Callback) override;

  usg::TokenArray getAppliedApiSchemas() const override;
  UsdKnobLayout getKnobLayout() const override;
  UsdKnobCustomisations getKnobCustomisations() const override;

  //! Apply any final scaling before drawing the locator.
  virtual void scaleLocator(DD::Image::ViewerContext* ctx, bool applyIconSizeScaling);

  //! Draws a picture of a point light.
  virtual void drawLocator(DD::Image::ViewerContext*);

  //! Get the color used to draw the locator's wireframe elements.
  fdk::Vec3f getLocatorOutlineColor(DD::Image::ViewerContext* ctx) const;

  //! Get the color used to draw the locator's filled elements.
  fdk::Vec3f getLocatorFillColor() const;

  //! Indicates whether the locator should use the current light color as its fill color.
  bool locatorUseLightForFillColor() const;

  // Easy access to specific named knob values
  double getIntensity() const;
  fdk::Vec3f getColor() const;

  // GUI settings
  bool editable() const { return _editable; }
  class Engine : public DD::Image::CreateOrEditGeomOp::Engine
  {
  public:
    Engine(ndk::GeomOpNode* parent)
    : DD::Image::CreateOrEditGeomOp::Engine(parent)
    , _transformSubEngine(this)
    , _schemaSubEngine(this)
    {
    }

    void processEdits(usg::GeomSceneContext& context, const usg::Path& path, Mode mode) override;
  
  protected:
    TransformGeomOpI::SubEngine _transformSubEngine;
    SchemaGeomOpI::SubEngine _schemaSubEngine;
  };

private:
  void setKnobsFromPrim();
  void setDefaultAndSetKnobToDefault(UsdKnobDefinitionsWithStorage & knobDefsWithStore,
                                     fdk::TimeValue time,
                                     const usg::Prim& prim);
private:  

  DD::Image::Knob* _dynamicDividerKnob = nullptr;
  int _dynamicKnobs  = 0;

  DD::Image::SnapKnobs _snapKnobs;
  DD::Image::TransformKnobs::XformOpOrder _xformOpOrder = DD::Image::TransformKnobs::XformOpOrder::NoChange;

  // Locator settings
  bool        _locatorFixedSize = false;
  fdk::Vec3f  _locatorFillColor = {0.8f, 0.8f, 0.8f};
  bool        _locatorUseLightForFillColor = true;
  
protected:

  // GUI settings
  int   _display3d = DD::Image::DISPLAY_WIREFRAME;
  bool  _selectable = true;
  bool  _editable = true;
};

#endif
