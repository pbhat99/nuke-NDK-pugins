// Copyright (c) 2021 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef SLR_LIGHTSHADER_H
#define SLR_LIGHTSHADER_H

#include "DDImage/SlrShader.h"
#include "DDImage/SlrCoShader.h"
#include "DDImage/SlrPrimInfo.h"

#include "usg/lux/LuxTokens.h" // for light shader convenience

namespace slr
{

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
    void  updateLightPropertiesAt(const usg::Prim&      lightPrim,
                                  const fdk::TimeValue& time);

    //! Subclass updates its local vars.
    //! Must implement.
    virtual void  _updateLightPropertiesAt(const usg::Prim&      lightPrim,
                                          const fdk::TimeValue& time) = 0;

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

        \a shadowTransmissionOut is the amount of transmission (shadowing/occlusion)
        between the light and the surface point. This is kept separate from \a colorOut
        and \a directPdfWeightOut so that per-channel shadowing contribution can be
        output from a shader separated from the illumination color and falloff.

        Final illumination of the light is typically calculated by multiplying
        (colorOut.rgb() * shadowTransmissionOut.rgb()) * directPdfWeightOut,
        bearing in mind that DD::Image::Pixels can be multi-channel/layer!
    */
    virtual bool    illuminate(const SlrShadingContext& stx,
                               const fdk::Vec3f&        lobeN,
                               const SlrLightInfo&      ltInfo,
                               ndk::RayContext&         illumRayOut,
                               float&                   directPdfWeightOut,
                               DD::Image::Pixel&        colorOut,
                               DD::Image::Pixel&        shadowTransmissionOut) const = 0;

    /*! Get the optical occlusion weight along \a illumRay between its minT and maxT.
        The returned value is usually applied as a multiplier against the light's emission.
        The shading context object information can be used for light shadow masking
        functionality and to determine if global shadowing is enabled.

        Default implementation checks stx.shadowingEnabled() and if enabled calls
        stx.getThinTransmissionFactor() with illumRay, returning true if there's any
        transmission, or false if 100% occluded.
    */
    virtual bool  shadowTransmissionFactor(const SlrShadingContext& stx,
                                           const ndk::RayContext&   illumRay,
                                           fdk::Vec3f&              shadowTransmissionFactor) const;


  private:
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
    DD::Image::Channel    shadowChannel;        //!< Channels to write shadow weight to
    DD::Image::ChannelSet outputChannels;       //!< All the channels this material outputs
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

    //! Lights that need to sample input textures declare their shading layers via this.
    const ShadingLayerReqSet&  writableShadingLayers() const;

    //! Validates the output light shader, sets the output material channels,
    //! and determines enabled state of the light.
    void  validateLight(SlrEngineContext& slrtx);

    //! Calls requestResources() on the set of input shaders.
    void  requestResources(const DD::Image::ChannelSet& request_channels);

    //! Calls initializeForShading() on the set of input shaders.
    void   initializeForShading(uint32_t num_shading_threads);


    //! Redirects to \a light_group.output, if valid.
    bool  illuminate(const SlrShadingContext& stx,
                     const fdk::Vec3f&        lobeN,
                     const SlrLightInfo&      ltInfo,
                     ndk::RayContext&         illumRayOut,
                     float&                   directPdfWeightOut,
                     DD::Image::Pixel&        colorOut,
                     DD::Image::Pixel&        shadowTransmissionOut) const;

  };

  using SlrLightMaterialSet = std::unordered_set<SlrLightMaterial*>;


} // namespace slr


#endif
