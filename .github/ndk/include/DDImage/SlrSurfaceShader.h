// Copyright (c) 2021 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef SLR_SURFACESHADER_H
#define SLR_SURFACESHADER_H

#include "DDImage/SlrShader.h"
#include "DDImage/SlrCoShader.h"
#include "DDImage/SlrLightShader.h"

namespace slr
{

  //-----------------------------------------------------------------------------

  //! Access these tokens like so: 'slr::PrimTokens.visibleToCamera'.
  //!
  struct DDImage_API PrimTokensDef {
    usg::Token::Lazy  visibleToCamera{"slr:visibleToCamera"}; // Value::Bool
  };
  extern DDImage_API PrimTokensDef PrimTokens;

  //-----------------------------------------------------------------------------


  /*! \class DD::Image::SlrSurfaceShader

      Surface shaders apply lighting calculations and are the final shading before
      fragment integration. There should be only one surface shader per shading
      shading network which may contain an arbitrarily-large coshader input network
      feeding the surface shader.
  */
  class DDImage_API SlrSurfaceShader : public SlrShader
                                     , public SlrShaderCutoutControlsI
  {
  public:
    //! Intersection testing sides mode
    //! TODO ndk16: this enum is duplicated on SurfaceShaderOp! We need 'common' shader
    //!             include headers that let us share these definitions since the schemas
    //!             also need the same values.
    enum SideVisibilityMode
    {
      SIDE_VISIBILITY_BOTH,
      SIDE_VISIBILITY_FRONT,
      SIDE_VISIBILITY_BACK
    };

    //! What 'kind' of surface is this and how does it react optically to light? While Slr2 is
    //! not intended to be a photoreal renderer the way surfaces react to light is still
    //! important for proper surface simulation, especially with respect to the handling of
    //! opacity/presence for accurate compositing.
    //!
    //! Some surfaces are always solid (100% opaque,) like metal, and others can be partially
    //! transparent (ignoring vapor-deposited metals) like a plastic or glass. Some surfaces have
    //! fine geometric details or gaps and can vary their presence at a subpixel rate, usually
    //! by texture. Some surfaces are very transparent to light and produce refraction effects,
    //! like glass or water, but are otherwise 'solid'. Other surfaces can filter light without
    //! significant refraction, like a thin plastic film (a gel or transparency) or thin glass.
    //!
    //! All of these are considered 'optical traits' of the surface and affect how rays are handled
    //! for intersection. For example a surface declaring it supports 'thin transmission' means
    //! rays can pass through without refraction and only stop if the opacity or presence is 100%.
    //! A shadow path is an example of this where typically we don't want to compute costly
    //! refractions(caustics) through the surface but still want to know the attenuation of the
    //! light through the surface for accurately filtered or colored shadows.
    //!
    //! Note on presenceVaries() - can this surface change its presence weight per sample or is it
    //! constant (solid) across the entire surface? If it can vary that indicates this shader
    //! needs to be run when calculating occlusion rays since the shadow silhouette of the surface
    //! may be affected by the per-sample surfaceShader() calculations.
    //! Note this is only an efficiency hint to the renderer to save on surface shading during
    //! occlusion transmission (shadow) testing, it will not affect integration of the surface
    //! which uses the final presence weight returned from surfaceShader().
    //!

    //! Surface visibility mask type used for ray type testing.
    using VisibilityTraits = SlrRayContext::TypeMask;

  public:
    //!
    SlrSurfaceShader(SlrEngineContext*     slrtx,
                     const InputKnobList&  inputs,
                     const OutputPortList& outputs,
                     void*                 inputs_storage_data = nullptr,
                     size_t                inputs_storage_size = 0);

    //---------------------------------------------------------

    //! Returns the class name, must implement.
    const char*  shaderClass() const override { return "SlrSurfaceShader"; }

    //!
    SlrSurfaceShader*  asSurfaceShader() override { return this; }

    //! Default to always output Mask_RGBA channels, in addition to any input channels.
    //! Adds default St & Color/Opacity primvars to the _geomPrimvarsReq.
    void  _validateShader() override;

    //!
    static SlrSurfaceShader*  create(SlrEngineContext*, const std::string_view& slr_shader_class);
    static SlrSurfaceShader*  create(SlrEngineContext*, const usg::ShaderDesc* shader_desc);

    //---------------------------------------------------------

    //! Convenience to get a SlrCoShader input coshader. Calls getConnection() and tests its shader type.
    SlrCoShader*   getCoShaderForInput(const std::string& input_name) const;


    /*! Build a SlrSurfaceShaderGroup from a ShaderDescGroup.

        The conversion will start at the ndk_shader_group's 'output' ShaderDesc and
        recursively convert all defined input connections, so unconnected ShaderDesc's
        in the group they will be ignored.

    */
    static size_t  buildSurfaceShaderGroup(const SlrEngineContext&     slrtx,
                                           const usg::ShaderDescGroup& shader_desc_group,
                                           SlrSurfaceShaderGroup&      slr_shader_group_out);

    /*! Update the properties of a SlrSurfaceShaderGroup built from a ShaderDescGroup.
    */
    static bool    updateSurfaceShaderGroup(const SlrEngineContext&     slrtx,
                                            const usg::ShaderDescGroup& shader_desc_group,
                                            SlrSurfaceShaderGroup&      slr_shader_group_out);


    /*! Update the common surface properties. A subclass can call this from
        within its updateProperty() method to update the common visibility
        and blending parameters.
    */
    void           updateProperties(const usg::ShaderDesc& shader_desc) override;

    //! Defaults St primvar to request as named 'st' and a Vec2 data type.
    ShadingLayerReq  parametricStPrimvar() const override;

    //! Defaults to requesting the 'st' primvar.
    void _requestResources(const DD::Image::ChannelSet& request_channels) override;


  public:
    //--------------------------------------------------------------------
    // Slr2 surface shading specific methods
    //
    // These are similar to the legacy Iop::fragment_shader() method (see
    // Iop.h for explanation of those methods) but are explicitly -only-
    // for surface shading which can include lighting calculations and
    // fragment merging.
    //
    // See SlrCoShader for passing general Pixel values between shaders.
    //--------------------------------------------------------------------

    /*! Global opacity traits of the surface.
        This value is assigned in _validateShader() by calling getGlobalOpacityTraits(), which
        a shader can override, but note these are only the traits derived from the shader controls
        which can be shared between multiple objects.

        If the shader has a valid input on baseInputIndex() the opacity traits from that input
        are automatically copied and the local shader traits applied as overrides.

        For a surface shader to take into account individual geometry primvars which may affect
        opacity for that particular surface (for example using 'primvars:displayOpacity') the
        shader should implement getOpacityTraitsForRenderable() for any custom opacity handling.
    */
    ndk::OpacityTraitsMask  globalOpacityTraits() const { return _globalOpacityTraits; }

    /*! Return the global opacity traits for the shader, usually derived from the control
        parameters.

        An opaque surface without varying opacity or presence should return ndk::opaqueSolid()
        by default, or if using the CommonSurfaceProperties parameters then return
        getCommonSurfaceOpacityTraits(), which is the default behavior.

        Per-renderable opacity traits that may require primvar values is retrievable via
        getOpacityTraitsForRenderable().
    */
    virtual ndk::OpacityTraitsMask  getGlobalOpacityTraits() const;

    /*! Return the opacity traits set by the CommonSurfaceProperties parameters, taking
        into account the optional opacity trait override argument. These overrides are
        usually provided on a per-renderable basis so that primvars affecting the opacity
        traits can be considered.

        The default implementation uses the common surface properties provided by the
        SlrSurfaceShader class, so a subclass is free to use the default behavior or can
        implement this method to change the behavior.
    */
    virtual ndk::OpacityTraitsMask  getCommonSurfaceOpacityTraits(const ndk::OpacityTraitsMask opacityTraitOverrides = 0x0) const;

    /*! Return the opacity traits for the renderable influenced by this surface shader.
        This method will be called for each renderable Gprim this shader is assigned to.

        By default this returns the shader's global opacity traits, ignoring any
        renderable specific attributes.
    */
    ndk::OpacityTraitsMask  getRenderableOpacityTraits(const SlrPrimInfo& primInfo) const override;

    //! Shader turns on cutout mode, which is passed down to the surface shader and
    //! applied by the integrator.
    bool  cutoutModeEnabled() const override;

    //! Shader turns on cutout data aovs mode, which is passed down to the surface shader
    //! and applied by the integrator.
    bool  cutoutModeRemovesDataAovsEnabled() const override;


    //--------------------------------------

    /*! Visibility traits of the surface.
        This value is assigned in _validateShader() by calling getGlobalVisibilityTraits(), which
        a shader can override, but note these are only the traits derived from the shader controls
        which can be shared between multiple objects.
    */
    VisibilityTraits  globalVisibilityTraits() const { return _globalVisibilityTraits; }

    /*! Return the visibility traits for the surface.
        Called from SlrSurfaceShader::_validateShader() to fill in _globalVisibilityTraits.

        A surface with no visibility restrictions should return VisibilityTraits() by
        default (SIDE_VISIBILITY_BOTH and all ray types visible,) or if using CommonSurfaceProperties
        parameters then return getCommonSurfaceVisibilityTraits().

        Default returns getCommonSurfaceVisibilityTraits().
    */
    virtual VisibilityTraits  getGlobalVisibilityTraits() const { return getCommonSurfaceVisibilityTraits(); }

    /*!
    */
    virtual VisibilityTraits  getRenderableVisibilityTraits(const SlrPrimInfo& primInfo) const;

    /*! Return the visibility traits set by the CommonSurfaceProperties parameters.

        The default implementation uses the common surface properties provided by the
        SlrSurfaceShader class, so a subclass is free to use the default behavior or can
        implement this method to change the behavior.
    */
    virtual VisibilityTraits  getCommonSurfaceVisibilityTraits() const;


    //--------------------------------------

    const DD::Image::Channel&    opacityChannel() const { return _opacityChannel; }
    const DD::Image::ChannelSet& opacityChannelSet() const { return _opacityChannelSet; }

    const DD::Image::Channel&    presenceChannel() const { return _presenceChannel; }
    const DD::Image::ChannelSet& presenceChannelSet() const { return _presenceChannelSet; }

    float  opacityFactor() const { return _opacityFactorClamped; }
    float  opacityPresenceThreshold() const { return _opacityPresenceThresholdClamped; }

    //--------------------------------------

    //! Uniform int value defining the priority of the surface w/respect to other
    //! overlapping surfaces inside the surface. Often retrieved from input
    //! 'dielectric_priority'.
    //! Defaults to 0(disabled) if shader does not define the parameter.
    int32_t  transmissionPriority() const { return _transmissionPriority; }

    //--------------------------------------

    /*! Calculate the shaded color of a surface, possibly with lighting.

        The \a stx shading context contains all the shade-time information
        the shader should need including interpolated vertex attribute
        values (a limited, fixed-size, pre-determined set) and references to
        scene information like the camera xforms.

        If the shader computes a varying presence then it will be calculated during
        ray intersection testing by calling this surfacePresence() on this shader,
        and retrievable from the shading context's presenceWeight() method.

        If opacity is required to determing presence then it will also be calculated
        during ray intersection testing by calling this surfaceOpacity() on this
        shader, and retrievable from the shading context's opacity() method.

        Note that the surfaceOpacity() value is used during thin-transmission occlusion.
        A refractive clear surface will want to report the true opacity via surfaceOpacity()
        so that shadows have proper transparency while returning 100% opacity from
        surfaceShader() so that the surface does not become additive when blended.

        The color, opacity and aovs of the surface are returned via the \a out Pixel.
    */
    virtual void  surfaceShader(const SlrShadingContext& stx,
                                DD::Image::Pixel&        out) = 0;


    /*! Calculate the presence weight for the shading context. Called during ray
        intersection only when doesPresenceVary() is true for this surface.
        Default implementation returns 100% presence.

        If varying opacity is being used to create presence then it will be
        pre-calculated by the intersection code by calling surfaceOpacity() on this
        shader and the value passed in via the \a opacity arg, and this method then
        implements the threshold test. Note - do not call surfaceOpacity() from this
        method as you will be doing double work.

        Usually 1.0 (solid) should be returned from this call unless the surface
        shader knows what its presence weight is. Note that presence is explicitly
        different than transparency/opacity and communicates how physically solid the
        surface is, not how opaque the surface is to light transmission.
        For example a glass surface is solid and yet transparent to light, so a
        shader representing glass would set out.opacity() to 0.0 or some thin value,
        and yet return 1.0 for presence since the glass surface is physically solid.
        Note that a fog or similar participating-medium atmospheric effect is considered
        'physically solid' for the purposes of presence/coverage since the effect usually
        covers the entirety of the subpixel sample and thus is 'fully there', but is not
        optically opaque and thus would affect light transmission.

        Coverage/presence can be between 'not there'(0) and 'fully there'(1) typically
        when filtered texture values are used as the presence argument. For example a
        texture representing the pattern of a fine black mesh could have a uniform albedo
        color of 0,0,0, a uniform opacity of 1.0, and one channel of the texture used to
        modulate the returned sample's presence weight rather than sample's opacity.

        Why does this matter? When subsampled surfaces are merged (stacked) together the
        presence weight is used to determine if the sample should be composited (>0) or
        just ignored (0). Total pixel coverage is calculated by adding up all the
        composited subpixel presence weights.
    */
    virtual float  surfacePresence(const SlrShadingContext& stx,
                                   const fdk::Vec3f*        opacity = nullptr) { return 1.0f; }

    /*! Calculate the rgb opacity for the shading context. Called during ray
        occlusion/transmission evaluation, usually during shadow evaluation, only when
        doesOpacityVary() and doesThinInterfaceTransmission() are both true for this
        surface.

        This value is always used for thin-transmission during shadow occlusion testing, so
        a refractive clear surface will want to report the true opacity from this method while
        returning 100% opacity from surfaceShader() so that the surface does not become
        additive when blended.

        Default implementation returns 100% opacity.
    */
    virtual fdk::Vec3f  surfaceOpacity(const SlrShadingContext& stx) { return fdk::Vec3f(1.0f); }

    /*! Copy the results from the surfaceShader() method to the \a layeredOut fragment output
        with no blending. stx.shadedOutput() contains the shaded result from surfaceShader().

        Default implementation copies the aov/primars then copies the outputChannels and
        coverage to \a layeredOut.
    */
    virtual void   copySurface(const SlrShadingContext& stx, DD::Image::Pixel& layeredOut);

    /*! Combine the results from the surfaceShader() method with the current accumulated value
        of the layered fragment output \a layeredAccum. stx.shadedOutput() contains the
        shaded result from surfaceShader().

        'Under' blending is only used for standard front-to-back alpha blending and cannot do
        most special merging operations. An advantage of front to back blending is that it can
        end early when the accumulated opacity reaches 1.0. Back-to-front blending cannot end
        early since the final result must include the frontmost surface. Thus front to back is
        potentially faster but is limited in its compositing operations.

        Color channels in \a shadedSurface should be pre-multiplied at this point by the surface
        presence weight, usually handled by the shading context. \a shadedSurface.coverage() can
        be combined like a matte channel using an under.

        Default implementation does an under of all color channels and coverage.
    */
    virtual void   blendSurfaceFrontToBack(DD::Image::Pixel& layeredAccum, const SlrShadingContext& stx);

    //! Same but for 'Over' back-to-front layering order supporting all compositing operations.
    //! \a shadedSurface.coverage() can be combined like a matte channel using an over.
    virtual void   blendSurfaceBackToFront(const SlrShadingContext& stx, DD::Image::Pixel& layeredOut);


  protected:
    // Uniform InputKnobs:
    int32_t _sideVisibilityMode;      //!< Which side is visible?
    bool    _cameraVisibility;        //!< Is this surface visible to camera rays?
    bool    _shadowVisibility;        //!< Is this surface visible to shadow rays?
    bool    _specularVisibility;      //!< Is this surface visible to specular rays?
    bool    _diffuseVisibility;       //!< Is this surface visible to diffuse rays?
    bool    _transmissionVisibility;  //!< Is this surface visible to transmitted rays?
    int32_t _transmissionPriority;    //!< Typically used for dielectric priority parameter
    // Uniform InputKnobs:
    FloatBinding          _opacityFactor;
    FloatBinding          _opacityPresenceThreshold;
    DD::Image::Channel    _opacityChannel;
    DD::Image::ChannelSet _opacityChannelSet;
    DD::Image::Channel    _presenceChannel;
    DD::Image::ChannelSet _presenceChannelSet;

    // Resolved only if at least one Gprim renderable provides the associated primvar so the
    // named primvar is in the SlrEngineContext global primvar channel map:
    Dual2Chans  _stPrimvarChans;    //!< Start indices of primvar(s) in shader channelset
    Dual2Chans  _CfPrimvarChans;    //!< Start indices of color primvar(s) in shader channelset
    Dual2Chans  _CoPrimvarChans;    //!< Start indices of opacity primvar(s) in shader channelset

    DD::Image::ChannelSet  _blendingColorChannelSet;  //!< Set of channels to apply blending function to
    DD::Image::ChannelPack _blendingColorChannelPack; //!< Automatically built from _blendingColorChannels

  private:
    //! Disabled copy constructor.
    SlrSurfaceShader(const SlrSurfaceShader&) = delete;
    //! Disabled assignment operator.
    SlrSurfaceShader& operator = (const SlrSurfaceShader&) = delete;

    VisibilityTraits       _globalVisibilityTraits;           //!< Global traits derived from knobs
    ndk::OpacityTraitsMask _globalOpacityTraits;              //!< Global traits derived from knobs
    float                  _opacityFactorClamped;             //!< Clamped version of _opacityFactor
    float                  _opacityPresenceThresholdClamped;  //!< Clamped version of _opacityPresenceThreshold
  };

  using SlrSurfaceShaderList = std::vector<SlrSurfaceShader*>;
  using SlrSurfaceShaderMap  = std::unordered_map<std::string, SlrSurfaceShader*>;


  //-----------------------------------------------------------------------------


  class DDImage_API SlrBlendingSurfaceShader : public SlrSurfaceShader
  {
  public:
    //! Sample blending operations.
    //! TODO ndk16: this enum is duplicated on SurfaceShaderOp! We need 'common' shader
    //!             include headers that let us share these definitions since the schemas
    //!             also need the same values.
    enum class BlendOp
    {
      None, Replace,
      // ones needing alpha which only affect color channels are first:
      Over, Stencil, Mask,
      // cutout operations also use presence*opacity and affect all channels including data:
      Cutout, Reveal,
      // the rest don't need alpha and treat all color channels equally:
      Plus, Max, Min
    };
    typedef void (*BlendOpFunc)(const DD::Image::Pixel&       surface_sample,
                                const DD::Image::ChannelPack& color_chans,
                                DD::Image::Pixel&             out);

    //! Returns one of the builtin BlendOp functions.
    static BlendOpFunc  GetBuiltinBlendOpFunc(BlendOp mode = BlendOp::Over);

    //! Returns whether a BlendOp mode can produce an opaque (alpha = 1) result after blending
    //! is performed.
    //! Certain blend modes like Plus, Min, Max will never combine to an opaque result even if
    //! the alpha of the result is >= 1 because the final result is expected to be divided by
    //! the total number of blended samples.
    static bool  CanBlendToOpaque(BlendOp mode);

    //! Does blend mode need opacity value?
    static bool  BlendNeedsOpacity(BlendOp mode);

    //! Blending mode the surface shader is set to. Because ScanlineRender is a utility renderer
    //! it can do special combining between samples that production renderers usually avoid or
    //! support via hacks, like min or max.
    //! Integrators need to know the shader's blend mode to properly handle combining channels
    //! and whether to stop at the first solid surface. For example if the shader is set to
    //! BlendOp::Plus or BlendOp::Max then the integrator cannot stop shading at the first
    //! solid sample.
    BlendOp      blendOpMode() const { return _blendOpMode; }

    //! The blend function this shader is using. Provided externally so the func can be directly
    //! called by an integrator, otherwise the blendSurface() virtual method should be used.
    //! This value is assigned in _validateShader() by calling getBlendOpFunc().
    BlendOpFunc  blendOpFunc() const { return _blendOpFunc; }

    //! Return the BlendOpFunc set by the CommonSurfaceProperties parameters.
    BlendOpFunc  commonSurfaceBlendOpFunc() const;


  public:
    //!
    SlrBlendingSurfaceShader(SlrEngineContext*     slrtx,
                             const InputKnobList&  inputs,
                             const OutputPortList& outputs,
                             void*                 inputs_storage_data = nullptr,
                             size_t                inputs_storage_size = 0);

    //! Return the thin-transmission blending operation for the surface - must implement.
    //! Called from SlrSurfaceShader::_validateShader() to fill in _blendOpFunc.
    //!
    //! A standard surface with no blending controls should return GetBuiltinBlendOpFunc()
    //! by default, or if using CommonSurfaceProperties parameters then return
    //! commonSurfaceBlendOpFunc().
    virtual BlendOpFunc  getBlendOpFunc() const = 0;

    //! Updates states from the blend knobs.
    void  updateProperties(const usg::ShaderDesc& shader_desc) override;

    //!
    void  _validateShader() override;

    //! Return the opacity traits set by the CommonSurfaceProperties parameters.
    //!
    //! SlrBlendingSurfaceShader adds tests for whether blend mode can blend to opaque.
    ndk::OpacityTraitsMask  getCommonSurfaceOpacityTraits(const ndk::OpacityTraitsMask opacityTraitOverrides = 0x0) const override;

    //!
    void  blendSurfaceBackToFront(const SlrShadingContext& stx, DD::Image::Pixel& layeredOut) override;


  protected:
    BlendOp     _blendOpMode;     //!< InputKnob value

  private:
    BlendOpFunc _blendOpFunc;     //!< Derived from input value
  };


  //-----------------------------------------------------------------------------
  //-----------------------------------------------------------------------------


  /*! \class DD::Image::SlrGeometryShader

      Displacement shader
  */
  class DDImage_API SlrGeometryShader : public SlrShader
  {
  public:
    //!
    SlrGeometryShader(SlrEngineContext*     slrtx,
                      const InputKnobList&  inputs,
                      const OutputPortList& outputs,
                      void*                 inputs_storage_data = nullptr,
                      size_t                inputs_storage_size = 0);


    //! Returns the class name, must implement.
    const char*  shaderClass() const override { return "SlrGeometryShader"; }

    //!
    SlrGeometryShader*  asGeometryShader() override { return this; }

    //!
    static SlrGeometryShader*  create(SlrEngineContext*, const std::string_view& slr_shader_class);
    static SlrGeometryShader*  create(SlrEngineContext*, const usg::ShaderDesc* shader_desc);


  public:
    //---------------------------------------------------------
    //
    // Slr compatible shader methods to implement if a surface shader.
    // Refer to Iop.h for explanation of methods as these are intended
    // to match the legacy shading methods for backwards-compatibility.
    //
    //---------------------------------------------------------

#if 0
    //!
    virtual void    displacementShader(const SlrShadingContext& stx,
                                       SlrShadingParams&        out) = 0;
#endif

    //!
    virtual float   displacementBound() { return 0.0f; }

  private:
    //! Disabled copy constructor.
    SlrGeometryShader(const SlrGeometryShader&) = delete;
    //! Disabled assignment operator.
    SlrGeometryShader& operator = (const SlrGeometryShader&) = delete;

  };

  using SlrGeometryShaderList = std::vector<SlrGeometryShader*>;
  using SlrGeometryShaderMap  = std::unordered_map<std::string, SlrGeometryShader*>;


  //-----------------------------------------------------------------------------
  //-----------------------------------------------------------------------------


  /*! Entry point for SlrSurfaceShader and SlrGeometryShader groups.

      There's one of these created for each enabled SlrPrimInfo in the primary render
      scene.

      They're stored in the SlrEngineContext, owned by the renderer, and are are static
      for the entire render session so any texture bindings remain stable and thread safe
      through the entirety of the render.
  */
  class DDImage_API SlrSurfaceMaterial
  {
  protected:
    int32_t                _renderVersion;        //!< Updated each time material is refreshed

  public:
    SlrSurfaceShaderGroup  surfaceGroup;          //!< Slr(ScanlineRender) Shader group - managed externally
    SlrGeometryShaderGroup displacementGroup;     //!< Slr(ScanlineRender) Shader group - managed externally
    //
    DD::Image::Hash        hash;                  //!< Global hash of object material
    DD::Image::ChannelSet  inputTextureChannels;  //!< All the channels from all input texture samplers
    DD::Image::ChannelSet  outputColorChannels;   //!< All the color (blendable) channels this material outputs
    DD::Image::ChannelSet  outputDataChannels;    //!< All the data (non-blendable) channels this material outputs
    DD::Image::ChannelSet  outputChannels;        //!< All the channels this material outputs
    DD::Image::Format      outputFormat;          //!< Nominal Format if the material represents a texture
    //
    SlrShader::InputBindingList textureBindings;          //!< List of all texture bindings used in material
    //
    bool                   displacementEnabled;           //!< Is displacement enabled?
    int                    displacementSubdivisionLevel;  //!< What recursion level to subdivide to
    fdk::Vec3f             displacementBounds;            //!< Displacement bounds scaled by local-to-world matrix


  public:

    //!
    SlrSurfaceMaterial();

    //! Releases the SlrShader allocations in the SlrSurfaceShaderGroup
    ~SlrSurfaceMaterial();

    //! Current render version the material has been updated at.
    int32_t  renderVersion() const { return _renderVersion; }
    void     setRenderVersion(int32_t v) { _renderVersion = v; }

    //! Return the path of the surface_group output shader, if there is one. Used mainly for debugging.
    const std::string& path() const;

    /*! Fills in/updates the SlrSurfaceMaterial with the SlrShaders created from the
        enabled SlrPrimInfo and its assigned material prim path.

        The priminfo will be updated to point at any created or existing material.
        This is usually called by the renderer in its _validate() method as all the
        shaders are required to know what input textures and channels the renderer
        needs to output and request.

        The SlrEngineContext holds the shared resources that are pointed to by the
        SlrPrimInfos.
    */
    static SlrSurfaceMaterial* translateUsgMaterial(SlrEngineContext&     slrtx,
                                                    const usg::Prim&      prim,
                                                    const fdk::TimeValue& time,
                                                    const usg::Path&      material_path,
                                                    SlrSurfaceMaterial*   existing_slr_material);


  public:
    //!
    SlrSurfaceShader*   surface() const { return surfaceGroup.output; }
    //!
    SlrGeometryShader*  displacement() const { return displacementGroup.output; }

    //! The list of primvars this material needs from the Gprim it's assigned to.
    //! At tessellation time this list is used to build the array of values read
    //! from prim.
    const ShadingLayerReqSet&  inputPrimvars() const;

    //! Return the primary 'st' primvar name this material uses for parametric surface
    //! evaluation. Note that this is usually the same primvar that's used for
    //! texture coordinates, but there can be different texture primvars used in each
    //! texture sampler, so it's not obvious what is the 'primary' parametric primvar.
    const ShadingLayerReq      parametricStPrimvar() const;

    //!
    const ShadingLayerReqSet&  writableShadingLayers() const;

    //! Global opacity traits from the surface shader (may apply to all prims).
    ndk::OpacityTraitsMask  globalOpacityTraits() const;

    //! Opacity traits from the surface shader for a renderable.
    ndk::OpacityTraitsMask  getRenderableOpacityTraits(const SlrPrimInfo& primInfo) const;

    //! Visibility traits from the surface shader.
    SlrSurfaceShader::VisibilityTraits  getRenderableVisibilityTraits(const SlrPrimInfo& primInfo) const;

    //------------------------------------------------------------
    // Shading methods redirect to either 'surface_group' or
    // 'displacement_group'.
    //------------------------------------------------------------

    //! Calls the output shader, if assigned, and sets the material channels.
    void   validateShader(SlrEngineContext& slrtx);

    //! Calls requestResources() on the set of input shaders.
    void   requestResources(const DD::Image::ChannelSet& request_channels);

    //! Calls initializeForShadingThread() on the set of input shaders.
    void   initializeForShadingThread(SlrThreadContext& sttx);

    //! Calls cleanupAfterShading() on the set of input shaders.
    void   cleanupAfterShading();

    //!
    float  displacementBound();

  };

  using SlrSurfaceMaterialSet = std::unordered_set<SlrSurfaceMaterial*>;


} // namespace slr


#endif
