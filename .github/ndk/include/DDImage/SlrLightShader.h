// Copyright (c) 2021 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef SLR_LIGHTSHADER_H
#define SLR_LIGHTSHADER_H

#include "DDImage/SlrShader.h"
#include "DDImage/SlrCoShader.h"
#include "DDImage/SlrEngineContext.h"

#include "usg/lux/LuxTokens.h" // for light shader convenience
#include "usg/tools/PathExpression.h"

namespace slr
{

  //-----------------------------------------------------------------------------

  //! Access these tokens like so: 'slr::LightTokens.falloffAmount'.
  //!
  struct DDImage_API LightTokensDef {
    // Geometry prim tokens:
    usg::Token::Lazy     castShadow{"slr:castShadow"   }; // Value::Bool
    usg::Token::Lazy  receiveShadow{"slr:receiveShadow"}; // Value::Bool
    //
    // Light shader tokens:
    usg::Token::Lazy     objectIlluminationMask{"slr:object_illumination_mask"}; // Value::PathExpr
    usg::Token::Lazy  objectReceiveShadowMask{"slr:object_receive_shadow_mask"}; // Value::String
    usg::Token::Lazy           shadowChannels{"slr:shadow_channels"           }; // Value::String
    //
    usg::Token::Lazy    falloffType{"slr:falloff_type"}; // Value::String
    //
    // For DomeLight specifically:
    usg::Token::Lazy  mirrorImageX{"slr:mirror_image_x"}; // Value::Int
    usg::Token::Lazy  mirrorImageY{"slr:mirror_image_y"}; // Value::Int
    usg::Token::Lazy      blurSize{"slr:blur_size"     }; // Value::Float2
  };
  extern DDImage_API LightTokensDef LightTokens;

  //-----------------------------------------------------------------------------


  /*! \class DD::Image::SlrLightShader
  */
  class DDImage_API SlrLightShader : public SlrShader
  {
  public:
    /*! Input values set by local knobs or input connections.
    */
    struct BaseInputParams
    {
      fdk::Vec3f  color     = fdk::Vec3f(1.0f); //!< Color of the emitted light
      float       intensity = 1.0f;             //!< Scale of the emitted light, multed with color
      float       exposure  = 0.0f;             //!< Scales the power of the light exponentially similar to an fstop and multed with intensity

      BaseInputParams() {}
      BaseInputParams(const fdk::Vec3f& _color,
                      float             _intensity,
                      float             _exposure = 0.0f) : color(_color), intensity(_intensity), exposure(_exposure) {}

      //! Return the scale factor from exposure.
      float  exposureFactor() const { return std::pow(2.0f, std::min(std::max(-50.0f, exposure), 50.0f)); }

      //! Return light color multed by intensity and exposure.
      fdk::Vec3f  illumColor() const { return color * (intensity * exposureFactor()); }
    };

    enum class FalloffType : int32_t
    {
      NoFalloff,
      LinearFalloff,      //!< 1 / distance
      QuadraticFalloff,   //!< Inverse-square point radiator, 1 / (distance * distance)
      CubicFalloff        //!< Inverse-cubed radiator, 1 / (distance * distance * distance)
    };

    DD::Image::Pixel    _color;           //!< Pre-calculated global output color - usually color*intensity*expFactor


  protected:
    fdk::TimeValueList _motionTimes;      //!< Time for each motion-sample
    fdk::Mat4dList     _motionXforms;     //!< Motion xform matrices
    fdk::Mat4dList     _motionInvXforms;  //!< Inverse motion xform matrices created from _motion_xforms
    //
    bool               _enabled;          //!< Light can illuminate scene. Set from result of validateEnabledState().
    //
    float              _diffuseBiasFactor;  //!< Default 1.0. Can be used to influence surface shaders that support it
    float              _specularBiasFactor; //!< Default 1.0. Can be used to influence surface shaders that support it
    //
    FalloffType        _falloffType;        //!< Default NoFalloff. Can be used to influence surface shaders that support it.
    int32_t            _nShadowChannels;    //!< If outputting shadow AOVs this is non-zero
    DD::Image::Channel _shadowChannels[3];  //!< Channels to write shadow weight to


  public:
    //!
    SlrLightShader(SlrEngineContext*     slrtx,
                   const InputKnobList&  inputs,
                   const OutputPortList& outputs);

    //---------------------------------------------------------

    //! Returns the class name, must implement.
    const char*  shaderClass() const override { return "SlrLightShader"; }
    //!
    SlrLightShader*  asLightShader() override { return this; }

    //!
    static SlrLightShader*  create(SlrEngineContext*, const std::string_view& slr_shader_class);
    static SlrLightShader*  create(SlrEngineContext*, const usg::Prim& lightPrim);

    //---------------------------------------------------------

    //! Convenience to get a SlrCoShader input coshader. Calls getConnection() and tests its shader type.
    SlrCoShader*   getCoShaderForInput(const std::string& input_name) const;

    /*! Build a SlrLightShaderGroup from a ShaderDescGroup, which for SlrLightShaders is not the light
        shader but any additional shader inputs, usually for texturing purposes. DomeLight or RectLight
        are good examples.

        The conversion will start at the ndk_shader_group's 'output' ShaderDesc and
        recursively convert all defined input connections, so unconnected ShaderDesc's
        in the group they will be ignored.

    */
    static size_t  buildLightShaderGroup(const SlrEngineContext&     slrtx,
                                         const usg::ShaderDescGroup& shader_desc_group,
                                         SlrLightShaderGroup&        slr_shader_group_out);

    /*! Update the properties of a SlrLightShaderGroup built from a ShaderDescGroup.
    */
    static bool    updateLightShaderGroup(const SlrEngineContext&     slrtx,
                                          const usg::ShaderDescGroup& shader_desc_group,
                                          SlrLightShaderGroup&        slr_shader_group_out);

    //---------------------------------------------------------

    //! Called either on shader creation or shader value update.
    //! Updates core light parameters then calls _updateLightPropertiesAt() to
    //! let the shader subclass update its locals.
    void  updateLightPropertiesAt(const SlrEngineContext& slrtx,
                                  const usg::Prim&        lightPrim,
                                  const fdk::TimeValue&   time);

    //! Subclass updates its local vars.
    //! Must implement.
    virtual void  _updateLightPropertiesAt(const SlrEngineContext& slrtx,
                                           const usg::Prim&        lightPrim,
                                           const fdk::TimeValue&   time) = 0;

    //! Does nothing for base light class since light shaders are generated from
    //! Prims. Use updateLightProperties() instead, or access the imaging shader
    //! group on the light material.
    void  updateProperties(const usg::ShaderDesc& shader_desc) override {}

    //! Assumes updateProperties() has already updated local vars.
    void  _validateShader() override;

    /*! Determine if the light is enabled which will be used to set the \a _enabled
        bool returned by isEnabled().
        This is called right after validateShader() which is when all uniform values
        should be established. Usually a light is enabled if its final color is >0.
        Disabling non-contributing lights speed up the shader light loops since they
        are not included in the final active light list.
        Must implement.
    */
    virtual bool  _validateLightEnabledState() = 0;

    /*! Quick test if light can illuminate scene.
        Usually valid only after updateProperties() has been called.
    */
    bool  isEnabled() const { return _enabled; }

    //---------------------------------------------------------

    /*! Assign the motion transforms matrices. The inverses will be automatically created.
        Subclasses can add add'l transforms before storage.
    */
    virtual void       setMotionXforms(const fdk::TimeValueList& motion_times,
                                       const fdk::Mat4dList&     motion_xforms);

    // Access to list of motion times and xforms. They should have the same count!
    uint32_t           numMotionTimes()  const { return (uint32_t)_motionTimes.size(); }
    uint32_t           numMotionXforms() const { return (uint32_t)_motionXforms.size(); }

    //! Get a time from the list of motion times. No range checking!
    double             getMotionTime(uint32_t index) const { return _motionTimes[index]; }
    //! Get a matrix from the list of motion xforms. No range checking!
    const fdk::Mat4d&  getMotionXform(uint32_t index) const { return _motionXforms[index]; }
    //! Get a matrix from the list of inversed motion xforms. No range checking!
    const fdk::Mat4d&  getMotionInvXform(uint32_t index) const { return _motionInvXforms[index]; }


    //! Get a possibly-interpolated matrix at 'time'.
    fdk::Mat4d         getMotionXformAt(const fdk::TimeValue& time) const;
    //! Get a possibly-interpolated inverse matrix at 'time'.
    //! This is faster than using getMotionXformAt(time).inverse() because it interpolates
    //! between two precalculated inverse matrices.
    fdk::Mat4d         getInverseMotionXformAt(const fdk::TimeValue& time) const;

    //! Get both at once (saves a motion step calculation.)
    //! This is faster than using getMotionXformAt(time).inverse() because it interpolates
    //! between two precalculated forward and inverted matrices at the same time.
    void               getMotionXformsAt(const fdk::TimeValue& time,
                                         fdk::Mat4d&           xform,
                                         fdk::Mat4d&           ixform) const;


  public:
    //--------------------------------------------------------------------
    // Shading methods
    //--------------------------------------------------------------------

    /*! Determine if the light is enabled. This calls the subclass _validateLightEnabledState()
        and sets \a _enabled to its returned value.
    */
    void   validateLightEnabledState();

    /*! Whether the light has a delta distribution (point/spot/direct lights)
        Must implement.
    */
    virtual bool  isDeltaLight() const = 0;

    /*! Can be set by a LightShader to influence the light's diffuse contribution in surface
        shaders that support it. Does not automatically affect light color and a LightShader
        should not apply this bias to the light color set in illuminate().
        Note that applying a weighting factor like this normally breaks energy-conserving
        surface shaders, but only if they explictly want to apply it.
    */
    float  diffuseBiasFactor() const { return _diffuseBiasFactor; }

    /*! Can be set by a LightShader to influence the light's specular contribution in surface
        shaders that support it. Does not automatically affect light color and a LightShader
        should not apply this bias to the light color set in illuminate().
        Note that applying a weighting factor like this normally breaks energy-conserving
        surface shaders, but only if they explictly want to apply it.
    */
    float  specularBiasFactor() const { return _specularBiasFactor; }

    //! Can be set by a LightShader to influence the light's falloff type in surface
    //! shaders that support it.
    FalloffType  fallOffType() const { return _falloffType; }

    //! Returns true if the light should copy shadow results to the surfaceShaderAOV Pixel
    //! passed to the illuminate() method.
    bool  outputShadowAOVs() const { return (_nShadowChannels > 0); }

    //! Convenience function to copy shadow channels to surface AOVs.
    void  copyShadowsToOutputColorAOVs(const fdk::Vec3f& shadowTransmissionFactor,
                                       DD::Image::Pixel& surfaceShaderColorAOVs) const;


    /*! Evaluate the light's contribution to a surface intersection.
        Returns false if light does not contribute to surface illumination.
        Must implement.

        \a stx is the shading context of the world-space point being illuminated.

        \a lobeN is the nominal center normal of the material lobe being evaluated which
        a light can use to modulate its output color. Most simple light types will ignore
        this value. It's most useful for shaped lights like domes, geometry lights, or
        textured area lights which can change color based on lobe angle vs just the
        light's orientation direction. Simple lights usually require just the light
        orientation.
        A 'lobe' determines the direction the energy hitting the surface is coming from
        and is usually a cone-ish shape anchored on the surface describing the radiation
        scatter behavior of the surface. A perfect mirror would have near zero scatter
        while a rough diffuse surface has a very large scatter.

        \a illumRayOut is built by the SlrLightShader and normally points from surface
        point to light origin and can be used for specular angle, etc.

        \a illumRayOut.minT' should be set to 0 or an epsilon bias off the light's
        'surface', clamped to the light's near value, if it has one.

        \a illumRayOut.maxT' should be the distance between surface point and light,
        clamped to the light's far value, if it has one.

        \a directPdfWeightOut is the direct-lighting power distribution function
        weight of the light for illumRayOut. Should -not- include shadow weighting.

        \a colorOut is the color(s) of light without any shadowing or falloff
        effects applied. i.e. it's the 'raw' output color of the light at the
        nominal 'surface' of the light for the angle described by illumRayOut.

        \a shadowTransmissionFactorOut is the percentage amount of transmission
        (shadowing/occlusion) between the light and the surface point, only valid between
        0.0 (100% occlusion) and 1.0 (0% occlusion.) This is kept separate from \a colorOut
        and \a directPdfWeightOut so that per-channel shadowing contribution can be output
        from a shader separated from the illumination color and falloff.

        \a surfaceShaderColorAOVs is the output Pixel that the surface shader is
        currently calculating, and is provided to the light shader so it can add
        arbitrary values (AOVs) to the surface output, typically to provide isolated
        shadowing or illumination contributions. Warning - a light shader should not
        mess with any channels it does not 'know' it can affect, and it should assume
        the channels have been zeroed out beforehand. A light shader will want to
        combine its value with the contents of the channel using an add or max. Note
        that these channels are blended like all other color channels so any float
        values that won't interpolate will likely be destroyed.

        Final illumination of the light is typically calculated by multiplying
        (colorOut * shadowTransmissionFactorOut) * directPdfWeightOut.
    */
    virtual bool    illuminate(const SlrShadingContext&      stx,
                               const SlrPrimEnabledLighting& primLightingInfo,
                               const fdk::Vec3f&             lobeN,
                               SlrRayContext&                illumRayOut,
                               float&                        directPdfWeightOut,
                               fdk::Vec3f&                   colorOut,
                               fdk::Vec3f&                   shadowTransmissionFactorOut,
                               DD::Image::Pixel&             surfaceShaderColorAOVs) const = 0;


    /*! Get the optical transmission percentage along \a illumRay between its minT and maxT,
        which is only valid between 0.0 (0% transmission) and 1.0 (100% transmission,) returning
        true if illumRay origin is fully occluded (0% transmission.)

        The returned factor is usually applied as a multiplier against the light's emission color.
        The shading context object information can be used for light shadow masking
        functionality and to determine if global shadowing is enabled.

        Default implementation checks ltInfo.castShadows() and if enabled calls
        stx.getThinTransmissionFactor() with illumRay.
    */
    virtual bool   getShadowTransmissionFactor(const SlrShadingContext&      stx,
                                               const SlrPrimEnabledLighting& primLightingInfo,
                                               const SlrRayContext&          illumRay,
                                               fdk::Vec3f&                   shadowTransmissionFactor) const;

    //! Get the distance based attenuation factor based on the light's falloff type.
    //! The defined falloff rates are: inverse-square (common point radiator), inverse-cubed,
    //! linear, and no falloff. No falloff always returns 1.0.
    virtual float  getDistanceAttenuation(float distance) const;

  protected:
    //! Convert a usg::Token to a SlrLightShader::FalloffType.
    //! This is used to convert the 'slr:falloff_type' token from the light prim
    //! into a SlrLightShader::FalloffType enum value and can be overriden by subclasses
    //! if required.
    virtual FalloffType convertToFalloffType(const usg::Token& token) const;

    //--------------------------------------------------------------------
    // Light Color and Temperature methods
    //--------------------------------------------------------------------
    //! Converts a color temperature (in Kelvin) to an RGB color.
    static fdk::Vec3f ColorTemperatureAsRgb(float colorTemperature);

  private:
    friend class SlrLightMaterial;
    //! Disabled copy constructor.
    SlrLightShader(const SlrLightShader&) = delete;
    //! Disabled assignment operator.
    SlrLightShader& operator = (const SlrLightShader&) = delete;

  };

  using SlrLightShaderList = std::vector<SlrLightShader*>;
  using SlrLightShaderMap  = std::unordered_map<std::string, SlrLightShader*>;


  //-----------------------------------------------------------------------------


  /*! Entry point for SlrLightShader groups.

      There's one of these created for each enabled SlrLightInfo in the primary render
      scene.

      They're stored in the SlrEngineContext, owned by the renderer, and are static for
      the entire render session so any texture bindings remain stable and thread safe
      through the entirety of the render.
  */
  class DDImage_API SlrLightMaterial
  {
  protected:
    int32_t               _renderVersion;       //!< Updated each time material is refreshed

  public:
    SlrLightShader*       lightShader;          //!< Shader for light itself
    SlrCoShaderGroup      imagingGroup;         //!< Shader group for light image source - managed externally
    //
    DD::Image::Hash       hash;                 //!< Global hash of object material
    DD::Image::ChannelSet inputTextureChannels; //!< All the channels from all input texture samplers
    DD::Image::ChannelSet outputColorChannels;  //!< All the color (blendable) channels this material outputs
    DD::Image::ChannelSet outputDataChannels;   //!< All the data (non-blendable) channels this material outputs
    //
    bool                  castsShadows;         //!< True if light casts shadows
    std::unique_ptr<usg::PathExpression> objectIllumMask; //!< Path Expression for object illumination matching if present
    std::unique_ptr<usg::PathExpression> objectReceiveShadowMask; //!< Path Expression for recieving shadowing matching if present
    //
    SlrShader::InputBindingList textureBindings;  //!< List of all texture bindings used in material


  public:

    //!
    SlrLightMaterial();

    //! Releases the SlrShader allocations in the SlrLightShaderGroup
    ~SlrLightMaterial();

    //! Current render version the material has been updated at.
    int32_t  renderVersion() const { return _renderVersion; }
    void     setRenderVersion(int32_t v) { _renderVersion = v; }


    /*! Fills in/updates the SlrEngineContext with the SlrShaders created from the
        enabled SlrLightInfos.

        The lightinfo will be updated to point at any created or existing material.
        This is usually called by the renderer in its _validate() method as all the
        shaders are required to know what input textures and channels the renderer
        needs to output and request.

        The SlrEngineContext holds the shared resources that are pointed to by the
        SlrLightInfos.
    */
    static SlrLightMaterial* translateUsgMaterial(SlrEngineContext&     slrtx,
                                                  const usg::Prim&      lightPrim,
                                                  const fdk::TimeValue& time,
                                                  const usg::Path&      materialPath,
                                                  SlrLightMaterial*     existingSlrMaterial);


  public:
    //!
    SlrLightShader*   light() const { return lightShader; }

    //! Color/texture used by light.
    SlrCoShader*      imagingShader() const { return imagingGroup.output; }

    //------------------------------------------------------------
    // Shading methods redirect to 'light_shaders' group.
    //------------------------------------------------------------

    //! Called either on shader creation or shader value update.
    //! Updates core light parameters on the material and shaders.
    void  updateLightPropertiesAt(const SlrEngineContext& slrtx,
                                  const usg::Prim&        lightPrim,
                                  const fdk::TimeValue&   time);

    //! Lights that need to sample input textures declare their shading layers via this.
    const ShadingLayerReqSet&  writableShadingLayers() const;

    //! Validates the output light shader, sets the output material channels,
    //! and determines enabled state of the light.
    void  validateLight(SlrEngineContext& slrtx);

    //! Calls requestResources() on the set of input shaders.
    void  requestResources(const DD::Image::ChannelSet& request_channels);

    //! Calls initializeForShadingThread() on the set of input shaders.
    void  initializeForShadingThread(SlrThreadContext& sttx);

    //! Calls cleanupAfterShading() on the set of input shaders.
    void  cleanupAfterShading();

  };

  using SlrLightMaterialSet = std::unordered_set<SlrLightMaterial*>;



  //--------------------------------------------------------------------
  //--------------------------------------------------------------------
  //--------------------------------------------------------------------

  inline void SlrLightShader::copyShadowsToOutputColorAOVs(const fdk::Vec3f& shadowTransmissionFactor,
                                                           DD::Image::Pixel& surfaceShaderColorAOVs) const
  {
    if (_nShadowChannels == 0) {
      ; // most common case
    }
    else if (_nShadowChannels > 2) {
      float& aov0 = surfaceShaderColorAOVs[_shadowChannels[0]];
      float& aov1 = surfaceShaderColorAOVs[_shadowChannels[1]];
      float& aov2 = surfaceShaderColorAOVs[_shadowChannels[2]];
      aov0 = std::max(aov0, (1.0f - shadowTransmissionFactor.x));
      aov1 = std::max(aov1, (1.0f - shadowTransmissionFactor.y));
      aov2 = std::max(aov2, (1.0f - shadowTransmissionFactor.z));
    }
    else if (_nShadowChannels > 1) {
      float& aov0 = surfaceShaderColorAOVs[_shadowChannels[0]];
      float& aov1 = surfaceShaderColorAOVs[_shadowChannels[1]];
      aov0 = std::max(aov0, (1.0f - shadowTransmissionFactor.x));
      aov1 = std::max(aov1, (1.0f - shadowTransmissionFactor.y));
    }
    else if (_nShadowChannels > 0) {
      float& aov0 = surfaceShaderColorAOVs[_shadowChannels[0]];
      aov0 = std::max(aov0, (1.0f - shadowTransmissionFactor.x));
    }
  }

} // namespace slr


#endif
