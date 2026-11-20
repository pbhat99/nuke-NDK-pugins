// Copyright (c) 2025 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDIMAGE_LIGHTSCENEOPI_H
#define DDIMAGE_LIGHTSCENEOPI_H

#include "DDImage/LightOp.h"
#include "DDImage/SceneOpI.h"

#include "usg/geom/GeomTokens.h"
#include "usg/lux/LuxTokens.h"

namespace DD
{
  namespace Image
  {
    /*! \class DD::Image::LightSceneOp
        \brief LightSceneOp defines a 3D light.

        LightSceneOps are containers for holding parameters and do not implement the actual
        lighting calculations. They export their values into prims added to a 3D scene
        (a USD stage.)

        Lighting calculations are provided by shaders in the destination renderers like
        Storm and ScanlineRender2 which translate the scene prims into the actual light
        implementations.
     */
    class DDImage_API LightSceneOp : public LightOp
                                   , public SceneOpI
    {
    public:
      //! Which component of the camera to provide.
      //! LightProvider subclass can extend this to support additional custom
      //! components (radius, shaping controls, etc) to expose via output knobs.
      enum
      {
        INTENSITY = SceneOpI::LAST_XFORM_PARAM,
        EXPOSURE,
        COLOR,
        COLOR_TEMPERATURE,
        COLOR_TEMPERATURE_ENABLE,
        NORMALIZE,
        DIFFUSE_WEIGHT,
        SPECULAR_WEIGHT,
        //
        LAST_BASE_LIGHT_PARAM
      };

      // Extract transform and light values.
      class DDImage_API LightProvider : public AxisValsProvider
      {
      public:
        //! Assigns the light component index.
        LightProvider(Knob* knob, LightSceneOp* owner, uint64_t componentId);

      protected:
              LightSceneOp*  _lightOwner()       { return _owner->asLightSceneOp(); }
        const LightSceneOp*  _lightOwner() const { return _owner->asLightSceneOp(); }

        //! Individual light components can disable depending on options.
        bool  _isProviderEnabled() const override;

        //! Retrieve the default value for the assigned light component.
        void  _getDefault(double* values, size_t nValues) const override;

        //! Return true if the source component is at its default.
        bool  _isDefault() const override;

        //! Return true if the source component is an animating attribute.
        bool  _isAnimated() const override;

        //! Subclass portion of provideValues().
        //! Default implementation extracts the value for the assigned light component.
        bool  _provideValues(double* values, size_t nValues, const ArrayKnobI* arrayKnob, const OutputContext& context) const override;
      };

      enum class FalloffType : int32_t
      {
        NoFalloff,
        LinearFalloff,
        QuadraticFalloff,
        CubicFalloff
      };

      //----------------------------------------------------------------------------

      LightSceneOp(Node* node);
      ~LightSceneOp();

      static constexpr double kIntensityMin = 0.0;
      static constexpr double kIntensityMax = 50.0;


      //! Cast to a LightSceneOp. This is much cheaper and safer than using dynamic_cast.
      const LightSceneOp* lightSceneOp() const override { return this; }
            LightSceneOp* lightSceneOp()       override { return this; }

      //! SceneOps are owned by the 'SceneOpNode' Node identifier.
      const char* nodeManagerClass() const override { return "SceneOpNode"; }

      //! AxisOp: Return the node input to use for the parent axis source. Defaults to input 0.
      int32_t  parentInput() const override { return 0; }

      //! SceneOpI: Return the node input to use for the scene source. Defaults to input 1.
      int32_t  sceneInput() const override { return 1; }

      //! SceneOpI: Return the node input to use for the parent axis source. Defaults to input 2.
      int32_t  constrainInput() const override { return 2; }

      //-----------------------------------------------------------------

      //! SceneOpI: Return the parentInput() input cast to a SceneOpI, if possible.
      SceneOpI*  parentInputSceneOp() const override;

      //! SceneOpI: Return the sceneInput() input cast to a GeomOp, if possible.
      GeomOp*  sceneInputOp() const override;

      //! SceneOpI: Return the AxisOp::constrainInput() input cast to a SceneOpI, if possible.
      SceneOpI*  constrainInputSceneOp() const override;

      //! SceneOpI: Return the name of the primary USD schema this node is handling.
      //! Since lights do not have a base schema class we use Xformable, and the LightAPIs are
      //! declared via getAppliedApiSchemas().
      const usg::Token&  primarySchemaType() const override { return usg::GeomTokens.types.Xformable; }

      //! SceneOpI: Returns a list of additional named schema APIs this node is handling.
      //! Lights always have a LightAPI schema applied.
      usg::TokenArray  getAppliedApiSchemaTypes() const override { return {usg::lux::LuxTokens.types.LightAPI}; }

      //! SceneOpI: Return the default filter string to apply in the prim path knob.
      const char*  primPathFilter() const override { return "filter:\"Type:Light\""; }

      //-----------------------------------------------------------------

      //! Default is 3: axis, scene and constrain
      int          minimum_inputs() const override;
      //! Default is 3: axis, scene and constrain
      int          maximum_inputs() const override;
      //! Optional input index is the scene-importer input. This returns (maximum_inputs()-1).
      int          optional_input() const override;

      //! AxisOp and null works on 0, and GeomOp or null on input 1.
      bool         test_input(int node_input, Op* op) const override;

      //! Returns null for all inputs. Override this so Op::test_input() doesn't produce warnings.
      Op*          default_input(int node_input) const override;

      //!
      const char*  input_label(int node_input, char*) const override;

      //-----------------------------------------------------------------

      //! Default knobs puts the transform controls on the front tab, and
      //! the projection, shutter, and lens controls on tab 'Projection'.
      void knobs(Knob_Callback) override;

      void  addDisplayOptionsKnobs(Knob_Callback) override;

      //! Called by Nuke just BEFORE Op::knobs(EARLY_STORE).
      //! This is used to update the scene importer and exporter states.
      void  _knobStorePrep(const OutputContext& opStoreContext) override;

      //! Called by Nuke immediately AFTER Op::knobs().
      //! This is used to update the scene importer and exporter states.
      void  _knobStorePost(const OutputContext& opStoreContext, Hash& newOpHash, Hash& newOpEditVersionHash) override;

      //! Adds additional front-panel knobs, primarily the color controls.
      //! Called before addTransformKnobs().
      void  addExtraFrontPanelKnobs(Knob_Callback f) override;

        //! Adds the standard color/intensity/exposure knobs.
      virtual void  addColorKnobs(Knob_Callback f);

      //! Adds diffuse amount and specular amount knobs
      virtual void  addRefineKnobs(Knob_Callback f);

      //! Adds ScanlineRender-specific render options controlling object masking.
      virtual void  addScanlineRenderMaskingKnobs(Knob_Callback f);

      //! Adds ScanlineRender-specific render options for controlling shadows.
      virtual void  addScanlineRenderShadowKnobs(Knob_Callback f);

      //! Adds fall off type and fall off knobs for slr2
      virtual void  addScanlineRenderFalloffKnobs(Knob_Callback f);

      //! Adds SplatRender-specific render options for per-light shadow tuning.
      virtual void  addSplatRenderKnobs(Knob_Callback f);

      //-----------------------------------------------------------------

      void  _validate(bool for_real) override;

      int  knob_changed(Knob*) override;

      //-----------------------------------------------------------------

      //! Local parent axis transform matrix.
      const fdk::Mat4d&  localParentAxisTransform() const override { return _getSceneOpLocalParentAxisTransform(); }

      //! Set the local parent axis transform matrix. Note this only persists until the
      //! next knobs()/validate() pass.
      void  setLocalParentAxisTransform(const fdk::Mat4d& localParent) override { _xformComponents.localParentAxis = localParent; }

      //! Result of the local-axis TRS Axis_Knob (*without* lookat rotations.)
      //! Calculated in _validateTransforms().
      const fdk::Mat4d&  localAxisTransform() const override { return _getSceneOpLocalAxisTransform(); }

      //! Set the local axis transform matrix. Note this only persists until the
      //! next knobs()/validate() pass.
      void  setLocalAxisTransform(const fdk::Mat4d& localAxis) override { _xformComponents.localAxis = localAxis; }

      //! Does the local parent transform animate? This is only an indicator that the
      //! transform -MAY- vary in time, not that its value actually does.
      bool  localParentTransformVaries() const override { return _getLocalParentTransformAnimation(); }

      //! Does the local transform animate? This is only an indicator that the transform
      //! -MAY- vary in time, not that its value actually does.
      bool  localTransformVaries() const override { return _getLocalTransformAnimation(); }

      //-----------------------------------------------------------------

      void build_handles(ViewerContext*) override;

      //! Draws a picture of a point light.
      void draw_handle(ViewerContext*) override;

      //! Point light OpenGL settings.
      void shade_GL(ViewerContext*, int light_num) override;


      //----------------------------------------------------------------------------
      // SceneOpI handling
      //----------------------------------------------------------------------------

      //! SceneOpI: Return the AxisOp the interface is attached to.
      AxisOp*  getAxisOp() override { return this; }

      //! Cast to a SceneOpI.
            SceneOpI*  sceneOpI()       override { return this; }
      const SceneOpI*  sceneOpI() const override { return this; }

      //----------------------------------------------------------------------------
      // GeometryProviderI scene prim output handling
      //----------------------------------------------------------------------------

      //! GeometryProviderI: Return the Op the interface is attached to.
      Op*  getGeometryProviderOp() override { return this; }

      //! Cast to a GeometryProviderI interface. This is much cheaper and safer than using dynamic_cast.
            GeometryProviderI*  geometryProvider()       override { return asGeometryProvider(); }
      const GeometryProviderI*  geometryProvider() const override { return const_cast<LightSceneOp*>(this)->asGeometryProvider(); }

      //-----------------------------------------------------------------

      //! Builds a path from SceneOpI::buildBasePrimPath(), appending the Node
      //! displayName() to use as the light schema's prim name.
      //!
      //! For example if the Node class name is 'MySpotlight_v4' we want it to show up in the
      //! scenegraph as 'MySpotlight', while the light schema name may be 'DiskLight'.
      virtual usg::Path  buildLightSchemaPrimPath() const;

      //! SceneOpI: Prepends the parent connection if there is one.
      usg::Path  buildBasePrimPath() const override;

      //! Return true if the component id is a base light component shared by most lights.
      //! Default implementation tests if id is within range INTENSITY...LAST_LIGHT_PARAM.
      virtual bool  isBaseLightComponent(uint64_t componentId) const;

      //! Return true if the component id is a typed light component.
      //! Default implmentation returns false.
      virtual bool  isTypedLightComponent(uint64_t componentId) const { return false; }

      //! Retrieve the light component value for a single time sample at OutputContext.
      //! Component value element count must not exceed nValues. If element count is less
      //! than nValues then fill the remaining elements with an appropriate default.
      virtual bool  getLightComponentAt(const usg::StageRef& stage,
                                        const usg::Prim&     lightPrim,
                                        const OutputContext& context,
                                        uint64_t             componentId,
                                        double*              values,
                                        size_t               nValues) const;

    protected:
      //! Create the light-specific providers.
      AuthoringProvider* _createAuthoringProvider(Knob* k, uint64_t componentId) override;

      //----------------------------------------

      //! Calculate the transform components of this node at a specific OutputContext.
      //! Subclass implementation of GetTransformComponentsAt(AxisOp), redirects to
      //! SceneOpI::_getSceneOpTransformComponentsAt().
      ndk::XformComponents _getTransformComponentsAt(const OutputContext& context) const override;

      //----------------------------------------

      //! Calculate the input parent transform of this node at a specific OutputContext.
      //! Subclass implementation of GetInputParentTransformAt(AxisOp), redirects to
      //! SceneOpI::_getSceneOpInputParentTransformAt().
      fdk::Mat4d _getInputParentTransformAt(const OutputContext& context) const override;

      //! Calculate the local parent transform of this node at a specific OutputContext.
      //! Subclass implementation of GetLocalParentAxisTransformAt(AxisOp), redirects to
      //! SceneOpI::_getSceneOpLocalParentAxisTransformAt().
      fdk::Mat4d _getLocalParentAxisTransformAt(const OutputContext& context) const override;

      //! Calculate the local axis transform of this node at a specific OutputContext.
      //! Subclass implementation of GetLocalAxisTransformAt(AxisOp), redirects to
      //! SceneOpI::_getSceneOpLocalAxisTransformAt().
      fdk::Mat4d _getLocalAxisTransformAt(const OutputContext& context) const override;

      //----------------------------------------

      //! Calculate the world transform of this node at a specific OutputContext.
      //! Subclass implementation of GetWorldTransformAt(AxisOp), redirects to
      //! SceneOpI::_getSceneOpWorldTransformAt().
      fdk::Mat4d _getWorldTransformAt(const OutputContext& context) const override;

      //----------------------------------------

      //! SceneOpI: Subclass portion of writeScenePrims().
      void  _writeScenePrims(usg::LayerRef&           layer,
                             bool                     appendTo,
                             const fdk::TimeValueSet& times = fdk::TimeValueSet()) override;


      //! Apply any final scaling before drawing the locator.
      virtual void scaleLocator(ViewerContext* ctx, bool applyIconSizeScaling);

      //! Draws a picture of a point light.
      virtual void drawLocator(ViewerContext*);

      //! Get the color used to draw the locator's wireframe elements.
      fdk::Vec3f getLocatorOutlineColor(ViewerContext* ctx) const;

      //! Get the color used to draw the locator's filled elements.
      fdk::Vec3f getLocatorFillColor() const;

      //! Indicates whether the locator should use the current light color as its fill color.
      bool locatorUseLightForFillColor() const;

    protected:
      std::string _objectIllumMask = "//*";
      std::string _objectReceiveShadowMask = "//*";
      DD::Image::Channel _shadowChannels[3];

      FalloffType _falloffType   = FalloffType::NoFalloff;

      // Locator settings
      bool        _locatorFixedSize = false;
      fdk::Vec3f  _locatorFillColor = {0.8f, 0.8f, 0.8f};
      bool        _locatorUseLightForFillColor = true;

      //! For internal use only.
      usg::Value _getLightAttr(const usg::Prim&      lightPrim,
                               const usg::Token&     inputsAttrName,
                               const usg::Token&     attrName,
                               const fdk::TimeValue& time) const;

      // Shadows knob backing
      bool _castShadow = true;

      // SplatRender per-light knob backing
      float      _splatShadowStrength    = 1.0f;
      int        _splatShadowSoftness    = 2;
      float      _splatShadowDepthRange  = 0.1f;
      float      _splatShadowOnset       = 0.5f;
      float      _splatShadowBias        = 0.001f;
      int        _splatShadowResolution  = 1024;
      int        _splatShadowDepthQuality = 8;
      float      _splatShadowCutoff      = 0.001f;

#if kDDImageVersionMajorNum <= 17
    protected:
      bool _isLegacyAxisOp() const override { return false; }
#else
#error "Remove the above code block in Nuke 18 as they are deprecated in Nuke 14."
#endif
    };

  }
}

#endif
