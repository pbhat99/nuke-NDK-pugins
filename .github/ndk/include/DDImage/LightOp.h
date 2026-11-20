//------------------------------------------------------------------------------
// DDImage/LightOp.h
//
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
//------------------------------------------------------------------------------

#ifndef DDImage_LightOp_h
#define DDImage_LightOp_h

#if kDDImageVersionMajorNum <= 16
#include "DDImage/CameraOp.h"
#include "DDImage/TextureFilter.h"
#include "DDImage/Pixel.h"
#else
#error "Remove the above code block in Nuke 17 as they are deprecated in Nuke 14."
#include "DDImage/AxisOp.h"
#endif
#include "DDImage/MaterialOpI.h"


namespace DD
{
  namespace Image
  {
#if kDDImageVersionMajorNum <= 16
    class LightContext;
    class rPrimitive;
    class VertexContext;
#else
#error "Remove the above code block in Nuke 17 as they are deprecated in Nuke 14."
#endif

    /*! \class DD::Image::LightOp
        \brief LightOp defines a 3D light.

        LightOps are containers for holding parameters and do not implement the actual
        lighting calculations. They export their values into prims added to a 3D scene
        (a USD stage) via the ScenePrimExporterOpI interface methods.

        Lighting calculations are provided by shaders in the destination renderers like
        Storm and ScanlineRender2 which translate the scene prims into the actual light
        implementations.
     */
    /*!
       Light base class which defaults to point source lighting with falloff.
     */
    class DDImage_API LightOp :
#if kDDImageVersionMajorNum <= 16
                                public CameraOp,
#else
#error "Remove the above code block in Nuke 17 as they are deprecated in Nuke 14."
                                public AxisOp,
#endif
                                public MaterialOpI
    {
    protected:
#if kDDImageVersionMajorNum <= 16
      //----------------------------------------------------------------------------
      // Deprecation unions to retain backward-compatibility in Nuke 16
      // The old var names will disappear in a Nuke major rev after 16
      //----------------------------------------------------------------------------
      union {
        Pixel _color;               //!< Color of the light (can be >3 channels!)
        mFnDeprecatedLegacy3DSystemInNuke14("color_ has been deprecated, use _color instead.")
        Pixel color_;               //!< Use _color instead
      };
      union {
        double _intensity;          //!< Global intensity
        mFnDeprecatedLegacy3DSystemInNuke14("intensity_ has been deprecated, use _intensity instead.")
        double intensity_;          //!< Use _intensity instead
      };
      //
      mFnDeprecatedLegacy3DSystemInNuke14("falloff_ has been deprecated.")
      bool  falloff_;               //!< Whether to factor in physical falloff
      mFnDeprecatedLegacy3DSystemInNuke14("falloffType_ has been deprecated.")
      int falloffType_;             //!< falloff type
      mFnDeprecatedLegacy3DSystemInNuke14("samples_ has been deprecated.")
      int samples_;                 //!< The number of samples (for area testing)
      mFnDeprecatedLegacy3DSystemInNuke14("sample_width_ has been deprecated.")
      float sample_width_;          //!< sample width (for shadowing)
      //
      union {
        bool  _castShadows;         //!< Whether light casts shadows
        mFnDeprecatedLegacy3DSystemInNuke14("cast_shadows_ has been deprecated, use _castShadows instead.")
        bool  cast_shadows_;        //!< Use _castShadows instead
      };
      mFnDeprecatedLegacy3DSystemInNuke14("shadows_mode_ has been deprecated.")
      int shadows_mode_;            //!< Shadow casting mode
      mFnDeprecatedLegacy3DSystemInNuke14("shadows_mode_ has been deprecated.")
      TextureFilter shadow_filter_; //!< Default filter to use in shadow texture sampling
      //
      mFnDeprecatedLegacy3DSystemInNuke14("scene_epsilon_ has been deprecated.")
      float scene_epsilon_;         //!< Scene epsilon for shadow comparison
      mFnDeprecatedLegacy3DSystemInNuke14("depthmap_bias_ has been deprecated.")
      float depthmap_bias_;         //!< Offset in Z of shadow buffer
      mFnDeprecatedLegacy3DSystemInNuke14("depthmap_slope_bias_ has been deprecated.")
      float depthmap_slope_bias_;   //!< Offset in Z to be scale in proportion of the local gradient
      mFnDeprecatedLegacy3DSystemInNuke14("alpha_threshold_ has been deprecated.")
      float alpha_threshold_;       //!< Alpha threshold for transparent shadow map mode
      mFnDeprecatedLegacy3DSystemInNuke14("shadow_jitter_scale_ has been deprecated.")
      float shadow_jitter_scale_;   //!< Size of shadow jitter
      mFnDeprecatedLegacy3DSystemInNuke14("depthmap_width_ has been deprecated.")
      int   depthmap_width_;        //!< Size in pixels of shadow buffer
#else
#error "Remove the above code block in Nuke 17 as they are deprecated in Nuke 14."
      fdk::Vec3d _color;                  //!< Color of the light
      double     _intensity;              //!< Global intensity
#endif
      bool       _enableColorTemperature; //!< Enables color temperature
      double     _colorTemperature;       //!< Color temperature in Kelvin
      double     _exposure;               //!< Power multiplier for the light (2^exposure_ * power)
      bool       _normalize;              //!< Disable area gain compensation
      double     _diffuse;                //!< Difuse multiplier
      double     _specular;               //!< Specular multiplier
      //
#if kDDImageVersionMajorNum <= 16
#else
#error "Remove the above code block in Nuke 17 as they are deprecated in Nuke 14."
      bool       _castShadows;            //!< Whether light casts shadows
#endif
      Channel    _shadowMaskChannel;      //!< Output the shadow mask to this channel

    public:
      LightOp(Node* node);
      ~LightOp();

      //! Cast to a LightOp. This is much cheaper and safer than using dynamic_cast.
      const LightOp* lightOp() const override { return this; }
            LightOp* lightOp()       override { return this; }

      //! Set the default color of a light node.
      unsigned node_color() const override { return 0xFFFF9A00; }

      //--------------------------------------------

      //! Return this LightOp cast to the MaterialOpI interface. Must implement.
#if kDDImageVersionMajorNum <= 16
      virtual MaterialOpI*  asMaterialOp() { return nullptr; }
#else
#error "Remove the above code block in Nuke 17 as they are deprecated in Nuke 14."
      virtual MaterialOpI*  asMaterialOp() = 0;
#endif

      //! The 'shader node' name for LightOps is its full prim path, so this method
      //! returns 'LightOp::exportBasePrimPath()'.
      //! The \a target_renderer arg is ignored.
      std::string  getShaderNodeName(const usg::Token& target_renderer = usg::Token()) override;

      //! Adds 'LightAPI' to list.
      //! Returns a list of API's this output shader must implement.
      usg::TokenArray getAppliedApiSchemas() const override;

      //! Builds a path from AxisOp::exportBasePrimPath(), appending the Node displayName()
      //! to use as the light schema's prim name. The light schema's type name comes from
      //! \a getOutputSchema().
      //!
      //! For example if the Node class name is 'MySpotlight_v4' we want it to show up in the
      //! scenegraph as 'MySpotlight', while the light schema name may be 'DiskLight'.
      virtual usg::Path  exportLightSchemaPrimPath() const;

      //--------------------------------------------

#if kDDImageVersionMajorNum <= 16
      const Pixel& color() const { return color_; }
      void setColor(const Pixel& color) { color_ = color; }
#else
#error "Remove the above code block in Nuke 17 as they are deprecated in Nuke 14."
      const fdk::Vec3d& color() const { return _color; }
      void setColor(const fdk::Vec3d& color) { _color = color; }
#endif

      double intensity() const { return _intensity; }
      void setIntensity(double v) { _intensity = v; }

      double exposure() const { return _exposure; }
      void setExposure(double v) { _exposure = v; }

      bool  enableColorTemperature() const { return _enableColorTemperature; }
      void setEnableColorTemperature(bool v) { _enableColorTemperature = v; }

      double colorTemperature() const { return _colorTemperature; }
      void setColorTemperature(double v) { _colorTemperature = v; }

      bool normalize() const { return _normalize; }
      void setNormalize(bool v) { _normalize = v; }

      double diffuse() const { return _diffuse; }
      void setDiffuse(double v) { _diffuse = v; }

      double specular() const { return _specular; }
      void setSpecular(double v) { _specular = v; }

      Channel shadowMaskChannel() const { return _shadowMaskChannel; }
      void setShadowMaskChannel(const Channel& channel) { _shadowMaskChannel = channel; }

      //--------------------------------------------

      //! Whether the light has a delta shaped distribution (point/spot/direct lights)
      //! Must implement.
#if kDDImageVersionMajorNum <= 16
      virtual bool isDeltaLight() const { return true; }
#else
#error "Remove the above code block in Nuke 17 as they are deprecated in Nuke 14."
      virtual bool isDeltaLight() const = 0;
#endif

      //! Cone angle is the half-angle (radius) of the code, so the default 90.0
      //! value describes a 180 degree wide cone. Any angle >= 90 is an invalid
      //! cone angle, so a point light would return 90.0 and a spot light would
      //! return a value below 90, while a direct light would return 0.0 which
      //! defines perfectly parallel rays.
      virtual double coneAngle() const { return 90.0; }

      //! Horizontal / vertical angle ratio.
      virtual double coneAspectRatio() const { return 1.0; }


    public:
      void _validate(bool for_real) override;

      //! Default knobs puts the transform controls on the front tab, and
      //! the projection, shutter, and lens controls on tab 'Projection'.
      void knobs(Knob_Callback) override;

      //! Adds the standard color/intensity/exposure knobs.
      void addColorKnobs(Knob_Callback);

      //!
      int knob_changed(Knob*) override;

      //--------------------------------------------

      void build_handles(ViewerContext*) override;

      //! Draws a picture of a point light.
      void draw_handle(ViewerContext*) override;

      //! Point light OpenGL settings.
      virtual void shade_GL(ViewerContext*, int light_num);


    protected:
      //! ScenePrimExporterOpI: Subclass portion of exportScenePrims().
      void _exportScenePrims(usg::LayerRef&            exportLayer,
                             int32_t                   exportMode,
                             const fdk::FrameValueSet& sampleFrames = fdk::FrameValueSet()) const override;


#if kDDImageVersionMajorNum <= 16
    private:
      //! Clamped values from knob controls.
      int _samples;
      int _depthmap_width;

    public:
      //----------------------------------------------------------------------------
      // Deprecated functions - will disappear in a Nuke major rev after 16
      //----------------------------------------------------------------------------
      mFnDeprecatedLegacy3DSystemInNuke14("request() has been deprecated, use requestTextures().")
        virtual void request(ChannelMask, int count);
      mFnDeprecatedLegacy3DSystemInNuke14("color_knobs() has been deprecated, use addColorKnobs() instead.")
        void color_knobs(Knob_Callback);
      mFnDeprecatedLegacy3DSystemInNuke14("attenuation_knobs() has been deprecated.")
        void attenuation_knobs(Knob_Callback);
      mFnDeprecatedLegacy3DSystemInNuke14("shadow_knobs() has been deprecated, use addShadowKnobs() instead.")
        void shadow_knobs(Knob_Callback);
      //-----------------------------
      mFnDeprecatedLegacy3DSystemInNuke14("color(Pixel) has been deprecated, use setColor() instead.")
        void color(const Pixel& v) { color_ = v; }
      mFnDeprecatedLegacy3DSystemInNuke14("set_intensity has been deprecated, use setIntensity() instead.")
        void set_intensity(float v) { intensity_ = v; }
      mFnDeprecatedLegacy3DSystemInNuke14("falloff() has been deprecated.")
        bool falloff() const { return falloff_; }
      mFnDeprecatedLegacy3DSystemInNuke14("falloffType() has been deprecated.")
        int falloffType() const { return falloffType_; }
      mFnDeprecatedLegacy3DSystemInNuke14("set_falloffType() has been deprecated.")
        void set_falloffType(int b) { falloffType_ = b; }
      //-----------------------------
      mFnDeprecatedLegacy3DSystemInNuke14("samples() has been deprecated.")
        int samples() const { return samples_; }
      mFnDeprecatedLegacy3DSystemInNuke14("set_samples() has been deprecated.")
        void set_samples(int n) { samples_ = n; }
      mFnDeprecatedLegacy3DSystemInNuke14("sample_width() has been deprecated.")
        float sample_width() const { return sample_width_; }
      mFnDeprecatedLegacy3DSystemInNuke14("set_sample_width() has been deprecated.")
        void set_sample_width(float width) { sample_width_ = width; }
      //-----------------------------
      mFnDeprecatedLegacy3DSystemInNuke14("Falloff enums have been deprecated.")
        enum { eNoFalloff, eLinearFalloff, eQuadraticFalloff, eCubicFalloff };
      mFnDeprecatedLegacy3DSystemInNuke14("LightType enums have been deprecated.")
        enum LightType {
          ePointLight,
          eDirectionalLight,
          eSpotLight,
          eOtherLight
        };
      mFnDeprecatedLegacy3DSystemInNuke14("ShadowMode enums have been deprecated.")
        enum ShadowMode {
          eSolidShadow,           //!< the light cast shadow considering every geometry fully solid
          eTransparentShadow,     //!< the light cast shadow considering the alpha channel of the texture mapped on the geometry
          eSemiTransparentShadow  //!< the light cast shadow in semi transparent mode
        };
      //-----------------------------
      mFnDeprecatedLegacy3DSystemInNuke14("cast_shadows() has been deprecated, use castShadows() instead.")
        bool cast_shadows() const { return cast_shadows_; }
      mFnDeprecatedLegacy3DSystemInNuke14("set_cast_shadows() has been deprecated, use setCastShadows() instead.")
        void set_cast_shadows(bool b) { cast_shadows_ = b; }
      mFnDeprecatedLegacy3DSystemInNuke14("shadows_mode() has been deprecated, use  instead.")
        int shadows_mode() const { return shadows_mode_; }
      mFnDeprecatedLegacy3DSystemInNuke14("alpha_threshold() has been deprecated, use  instead.")
        float alpha_threshold() const { return alpha_threshold_; }
      mFnDeprecatedLegacy3DSystemInNuke14("shadow_filter() has been deprecated, use  instead.")
        TextureFilter* shadow_filter() { return &shadow_filter_; }
      mFnDeprecatedLegacy3DSystemInNuke14("depthmap_bias() has been deprecated, use  instead.")
        float depthmap_bias() const { return depthmap_bias_; }
      mFnDeprecatedLegacy3DSystemInNuke14("set_depthmap_bias() has been deprecated, use  instead.")
        void set_depthmap_bias(float v) { depthmap_bias_ = v; }
      mFnDeprecatedLegacy3DSystemInNuke14("shadow_jitter_scale() has been deprecated, use  instead.")
        float shadow_jitter_scale() const { return shadow_jitter_scale_; }
      mFnDeprecatedLegacy3DSystemInNuke14("set_shadow_jitter_scale() has been deprecated, use  instead.")
        void set_shadow_jitter_scale(float v) { shadow_jitter_scale_ = v; }
      mFnDeprecatedLegacy3DSystemInNuke14("depthmap_width() has been deprecated, use  instead.")
        int depthmap_width() const;
      mFnDeprecatedLegacy3DSystemInNuke14("set_depthmap_width() has been deprecated, use  instead.")
        void set_depthmap_width(int n) { depthmap_width_ = n; }
      mFnDeprecatedLegacy3DSystemInNuke14("getShadowMaskChannel() has been deprecated, use shadowMaskChannel() instead.")
        Channel getShadowMaskChannel() const { return _shadowMaskChannel; }
      //-----------------------------
      mFnDeprecatedLegacy3DSystemInNuke14("Class() has been deprecated, implement on subclass plugin instead.")
        const char* Class() const override { return "Light"; }
      mFnDeprecatedLegacy3DSystemInNuke14("node_help() has been deprecated, implement on subclass plugin instead.")
        const char* node_help() const override { return ""; }
      mFnDeprecatedLegacy3DSystemInNuke14("displayName() has been deprecated, implement on subclass plugin instead.")
        const char* displayName() const override;
      //-----------------------------
      mFnDeprecatedLegacy3DSystemInNuke14("is_delta_light() has been deprecated, use isDeltaLight() instead.")
        virtual bool is_delta_light() const;
      mFnDeprecatedLegacy3DSystemInNuke14("hfov() has been deprecated, use coneAngle() and coneAspectRatio() instead.")
        virtual double hfov() const { return 90.0; }
      mFnDeprecatedLegacy3DSystemInNuke14("vfov() has been deprecated, use coneAngle() and coneAspectRatio() instead.")
        virtual double vfov() const { return 90.0; }
      mFnDeprecatedLegacy3DSystemInNuke14("aspect() has been deprecated, use coneAspectRatio() instead.")
        virtual double aspect() const { return hfov() / vfov(); }
      mFnDeprecatedLegacy3DSystemInNuke14("lightType() has been deprecated, implement on subclass plugin instead.")
        virtual int lightType() const { return ePointLight; }
      //-----------------------------
      mFnDeprecatedLegacy3DSystemInNuke14("get_L_vector() has been deprecated.")
        virtual void  get_L_vector(LightContext&  ltx,
                                   const Vector3& surfP,
                                   const Vector3& lobeN,
                                   Vector3&       L, float& D) const;
      mFnDeprecatedLegacy3DSystemInNuke14("get_color() has been deprecated.")
        virtual void  get_color(LightContext&  ltx,
                                const Vector3& surfP,
                                const Vector3& lobeN,
                                const Vector3& L, float D,
                                Pixel& out);
      mFnDeprecatedLegacy3DSystemInNuke14("get_shadowing() has been deprecated.")
        virtual float get_shadowing(const LightContext&  ltx,
                                    const VertexContext& vtx,
                                    const Vector3&       surfP,
                                    Pixel&               out);
#else
#error "Remove the above code block in Nuke 17 as they are deprecated in Nuke 14."
#endif
    };

}} // namespace DD::Image

#endif
