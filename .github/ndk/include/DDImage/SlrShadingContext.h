//------------------------------------------------------------------------------
// DDImage/SlrShadingContext.h
//
// Copyright (c) 2021 The Foundry Visionmongers Ltd.  All Rights Reserved.
//------------------------------------------------------------------------------

#ifndef SLR_SHADINGCONTEXT_H
#define SLR_SHADINGCONTEXT_H

#include "DDImage/SlrPrimInfo.h" // for SlrLightInfoList
#include "DDImage/SlrAovLayer.h"
#include "DDImage/SlrRayContext.h"

#include "ndk/base/Channel.h"

#include "DDImage/ChannelSet.h"
#include "DDImage/Pixel.h"
#include "DDImage/Thread.h" // for ThreadId

namespace DD {
  namespace Image {
    class Iop;
    class Sampler;
    class TextureFilter;
  }
}

namespace ndk {
  class SamplingRng;
}

namespace slr
{
  class SlrThreadContext;
  class SlrIntegratorContext;
  class SlrSurfaceShader;
  class SlrGeometryShader;
  class SlrVolumeShader;
  class SlrRayBundle;
  class SlrDeepPixel;
  class SlrShader;
  struct SlrShaderThreadData;
  class StxLayeringStack;

  using OutputDeepPixel = SlrDeepPixel;
  using OutputDeepPixelList = std::vector<OutputDeepPixel>;

  //-----------------------------------------------------------------------------

  //! Possible blending orders. Note that 'Front' != nearest surface! 'Front' is the first
  //! surface in a sorted list of surfaces. That list can be sorted farthest to nearest,
  //! so in that case 'Front' == farthest and 'Back' == nearest.
  enum BlendingOrder
  {
    Disabled,     //!< Don't blend (single surface mode)
    FrontToBack,  //!< Blend using 'Under', stopping when opacity == 1. Many non-standard modes will not work with this order.
    BackToFront,  //!< Blend using 'Over'. All non-standard blend modes like min, max, plus will work with this order.
  };

  enum ShadeSides
  {
    Both,   //!< Shade both sides
    Front,  //!< Shade front only
    Back,   //!< Shade back only
  };

  enum PrimvarInterpolation
  {
    Invalid_Rate,   //!< Interpolation rate in unassigned, unknown, or primvar itself is invalid
    //
    Prim_Rate,      //!< Prim rate - 1 value for entire GPrim
    Face_Rate,      //!< Face rate - 1 value for each face
    Facevert_Rate,  //!< Facevert rate - 1 value at each face vertex
    Point_Rate,     //!< Point rate - 1 value at each point
  };

  //-----------------------------------------------------------------------------

  //! Per sample time and motion step info
  struct SampleInfo
  {
    int32_t        index;             //!< Index of sample in a list of rays
    fdk::TimeValue time;              //!< Absolute time
    float          shutterPercentage; //!< Percentage (0..1) of total shutter time of sample
    uint32_t       motionStepMode;    //!< Interpolation mode (MOTIONSTEP_START, MOTIONSTEP_MID, MOTIONSTEP_END)
    uint32_t       motionSampleStart; //!< Motion sample index for start of shutter segment
    float          t;                 //!< Offset of time in motionStep (0..1)
  };

  using SampleInfoList = std::vector<SampleInfo>;

  //-----------------------------------------------------------------------------

  //! Convenience function to remove any leading 'primvars:' notation from string.
  DDImage_API
  std::string  StripPrimvarNotation(const char* name);
  DDImage_API
  std::string  StripPrimvarNotation(const std::string& name);
  DDImage_API
  std::string  StripPrimvarNotation(const usg::Token& name);

  //-----------------------------------------------------------------------------

  //! Used to communicate the name and channel count of layers that shaders are requesting as
  //! primvars or writable shading layers.
  //!
  //! Note when requesting primvars the name should -not- contain the leading 'primars:' of the
  //! source attribute. For example if requesting the displayColor primvar then name should be
  //! 'displayColor' and -not- 'primvars:displayColor'.
  //! if it's more convenient to use the attribute names with the leading 'primvars:' then use
  //! the constructors which will strip off any leading 'primvars:'.
  //!
  //! The channel count -must- be within 1..4, channel counts > 4 or < 1 are ignored.
  //! If 'interp' is true then additional '<name>_Dx' and '<name>_Dy' shading layers are requested
  //! with the same channel count. This is ignored when primvar sample layers are being requested
  //! as the primvar interpolation type is used.
  struct ShadingLayerReq
  {
    usg::Token name;        //!< Requested layer name (if primvar this is -without- the leading 'primvars:')
    int32_t    nChans;      //!< Number of channels, between 1..4. Requests with > 4 chans will be ignored.
    bool       interp;      //!< Does shading layer interpolate across primitives? Ignored for primvar layers.

    //! Request is considered invalid if number of channels requested is > 4.
    bool empty() const { return (name.empty() || nChans < 1 || nChans > 4); }

    explicit operator bool() const { return !empty(); }

    bool operator == (const ShadingLayerReq& b) const { return (name == b.name && nChans == b.nChans); }
    bool operator != (const ShadingLayerReq& b) const { return (name != b.name || nChans != b.nChans); }
    bool operator <  (const ShadingLayerReq& b) const { return (name < b.name); }

    ShadingLayerReq() : nChans(0) {}

    //! Removes any leading 'primvars:' before setting name.
    ShadingLayerReq(const usg::Token& _name, int32_t n, bool _interp = false)
      : name(StripPrimvarNotation(_name.text())), nChans(n), interp(_interp)
    {}

    //! Removes any leading 'primvars:' before setting name.
    ShadingLayerReq(const char* _name, int32_t n, bool _interp = false)
      : name(StripPrimvarNotation(_name)), nChans(n), interp(_interp)
    {}
  };

  std::ostream& operator << (std::ostream&, const ShadingLayerReq&);

  using ShadingLayerReqList = std::vector<ShadingLayerReq>;
  using ShadingLayerReqSet  = std::set<ShadingLayerReq>;

  //-----------------------------------------------------------------------------

  // Built-in parameter channel definitions:

  static constexpr ndk::Channel  Chan_R  = 1; // Same as ndk::Chan_R - vertex red
  static constexpr ndk::Channel  Chan_G  = 2; // Same as ndk::Chan_G - vertex green
  static constexpr ndk::Channel  Chan_B  = 3; // Same as ndk::Chan_B - vertex blue
  static constexpr ndk::Channel  Chan_A  = 4; // Same as ndk::Chan_A - vertex alpha-transparency
  //
  static constexpr ndk::Channel  Chan_Presence  =  5; // float - contributes to coverage
  static constexpr ndk::Channel  Chan_Oi        =  6; // Vec3f - rgb opacity
  //
  static constexpr ndk::Channel  Chan_Zlinear   =  9; // float - Z-linear (projected Z, if applicable)
  static constexpr ndk::Channel  Chan_Zdistance = 10; // float - Z distance
  //
  static constexpr ndk::Channel  Chan_Ps        = 11; // Vec3f - shading position
  static constexpr ndk::Channel  Chan_Pg        = 14; // Vec3f - geometric position
  static constexpr ndk::Channel  Chan_dPgdx     = 17; // Vec3f
  static constexpr ndk::Channel  Chan_dPgdy     = 20; // Vec3f
  static constexpr ndk::Channel  Chan_dPgdu     = 23; // Vec3f
  static constexpr ndk::Channel  Chan_dPgdv     = 26; // Vec3f
  static constexpr ndk::Channel  Chan_dPgdw     = 29; // Vec3f
  static constexpr ndk::Channel  Chan_dPdtime   = 32; // Vec3f
  //
  static constexpr ndk::Channel  Chan_TriUvw    = 35; // Vec3f
  //
  static constexpr ndk::Channel  Chan_Ns        = 38; // Vec3f - shading normal
//static constexpr ndk::Channel  Chan_dNsdy     = 41; // Vec3f
//static constexpr ndk::Channel  Chan_dNsdy     = 44; // Vec3f
  static constexpr ndk::Channel  Chan_Ng        = 41; // Vec3f - geometric normal
  static constexpr ndk::Channel  Chan_Ts        = 44; // Vec3f - shading tangent
  //
  static constexpr ndk::Channel  Chan_UV        = 47; // Vec2f - surface parametric-uv
  static constexpr ndk::Channel  Chan_dUVdx     = 49; // Vec2f
  static constexpr ndk::Channel  Chan_dUVdy     = 51; // Vec2f
  //
  static constexpr ndk::Channel  Chan_UVW       = 53; // float - parametric-w (for volumes)
  static constexpr ndk::Channel  Chan_dUVWdx    = 54; // float
  static constexpr ndk::Channel  Chan_dUVWdy    = 55; // float
  //
  static constexpr ndk::Channel  Chan_St        = 56; // Vec2f - texture coord
  static constexpr ndk::Channel  Chan_dStdx     = 58; // Vec2f
  static constexpr ndk::Channel  Chan_dStdy     = 60; // Vec2f
  static constexpr ndk::Channel  Chan_dStdu     = 62; // Vec2f
  static constexpr ndk::Channel  Chan_dStdv     = 64; // Vec2f
  //
  static constexpr ndk::Channel  Chan_PgStart   = 66; // Vec3f - geometric position at motion sample start
  static constexpr ndk::Channel  Chan_PgEnd     = 69; // Vec3f - geometric position at motion sample end
  static constexpr ndk::Channel  Chan_MotionFwd = 72; // Vec2f - forward motion vectors
  static constexpr ndk::Channel  Chan_MotionBwd = 74; // Vec2f - backwards motion vectors
  //
  // Predefined contribution layers for use in surface shaders:
  static constexpr ndk::Channel  Chan_Albedo       = 76; // Vec3f - rgb surface albedo reflectance factor
  static constexpr ndk::Channel  Chan_Diffuse      = 79; // Vec3f - rgb surface default diffuse lobe contribution
  static constexpr ndk::Channel  Chan_Specular     = 82; // Vec3f - rgb surface default specular lobe contribution
  static constexpr ndk::Channel  Chan_Transmission = 85; // Vec3f - rgb surface default transmission lobe contribution
  static constexpr ndk::Channel  Chan_Emission     = 88; // Vec3f - rgb surface default emission lobe contribution
  static constexpr ndk::Channel  Chan_Shadow       = 91; // Vec3f - rgb surface shadow color
  //
  //
  static constexpr ndk::Channel  Chan_LastPredefined = Chan_Shadow + 2;

  //------------------------------------------------------------------------
  // Predefined constants commonly used in shaders
  
  static const fdk::Vec2f vec2Zero(0.0f);
  static const fdk::Vec3f vec3Zero(0.0f);
  static const fdk::Vec4f vec4Zero(0.0f);
  static const fdk::Vec2f vec2One(1.0f);
  static const fdk::Vec3f vec3One(1.0f);
  static const fdk::Vec4f vec4One(1.0f);

  static const fdk::Vec3f defaultBlack(0.0f);
  static const fdk::Vec3f defaultGrey(0.18f);
  static const fdk::Vec3f defaultWhite(1.0f);

  //------------------------------------------------------------------------
  // Specialized types for returning a value and its dx/dy derivatives in a single object.
  // More lightweight than packing them inside a Pixel or Mat4d.
  // Named for the similar OSL class but not using templates so these can contain fdk Vec classes.
  // However they will not be as efficient as the vectorizable data layout in OSL.

  struct Dual2f     { float      val, dx, dy; };
  struct Dual2d     { double     val, dx, dy; };
  struct Dual2Vec2f { fdk::Vec2f val, dx, dy; };
  struct Dual2Vec3f { fdk::Vec3f val, dx, dy; };
  struct Dual2Vec4f { fdk::Vec4f val, dx, dy; };

  //! Channels pointing at the data indices for a Dual2 package.
  struct Dual2Chans
  {
    ndk::Channel val, dx, dy;

    Dual2Chans() = default;
    constexpr Dual2Chans(ndk::Channel v, ndk::Channel x, ndk::Channel y) : val(v), dx(x), dy(y) {}
    bool isAssigned() const { return (val != ndk::Chan_Empty); }
    bool empty() const { return !isAssigned(); }
  };

  // Built-in parameter Dual2Chans definitions:

  static constexpr Dual2Chans  Dual2Chans_Empty = { ndk::Chan_Empty, ndk::Chan_Empty, ndk::Chan_Empty };
  static constexpr Dual2Chans  Dual2Chans_Pg  = { slr::Chan_Pg , slr::Chan_dPgdx , slr::Chan_dPgdy  }; // Vec3f
  static constexpr Dual2Chans  Dual2Chans_UV  = { slr::Chan_UV , slr::Chan_dUVdx , slr::Chan_dUVdy  }; // Vec2f parametric-uv
  static constexpr Dual2Chans  Dual2Chans_UVw = { slr::Chan_UVW, slr::Chan_dUVWdx, slr::Chan_dUVWdy }; // float parametric-w (for volumes)
  static constexpr Dual2Chans  Dual2Chans_St  = { slr::Chan_St , slr::Chan_dStdx , slr::Chan_dStdy  }; // Vec2f texture coord

  // Convenience ray intersection distance constants:

  static constexpr float  defaultMinDistance() { return ndk::RayContext::defaultMinT(); }
  static constexpr float  defaultMaxDistance() { return ndk::RayContext::defaultMaxT(); }

  //-----------------------------------------------------------------------------


  /*! This class is intended to be passed up a SlrShader tree.
  */
  class DDImage_API SlrShadingContext
  {
  public:
    virtual ~SlrShadingContext() = default;

    uint16_t           rayDepth;            //!< Ray depth that created this context where 0 indicates a camera, or primary ray
    uint16_t           diffuseDepth;        //!< Ray depth along a diffuse/indirect scatter path
    uint16_t           reflectionDepth;     //!< Ray depth along a specular/glossy reflection path
    uint16_t           refractionDepth;     //!< Ray depth along a refraction/transmission path
    //
    SlrSurfaceShader*  surfaceShader;       //!< Current surface shader being evaluated
    SlrVolumeShader*   atmosphereShader;    //!< Current atmospheric volume shader being evaluated, if any

  public:
    //! Ray that created this shading context.
    //! If this is a 'virtual' surface (usually the camera origin stx) then this ray data will
    //! be the InvalidRay(), and isValid() will return false.
    const SlrRayContext&  srcRtx() const;

    //! Returns true is shading context is bound to valid ray data.
    //! If this returns false then any of the methods that access shading data (srcRtx) will
    //! crash!
    bool  isValid() const;

    explicit operator bool() const { return isValid(); }

    //! Previous SlrShadingContext, normally the last surface intersected/shaded and one level
    //! up in ray depth.
    const SlrShadingContext*  previousStx() const;

    //! Create an editable shading context that's a copy of this one. Use the setAttrib() methods
    //! to change the values on the returned context. The setAttrib() methods will fail on a
    //! non-editable context.
    //! This is primarily used to modify the shading context when passing it to an input shader.
    //! A good example of this is modifying the UV attrib based on the current point location to
    //! perform high quality texture projections.
    SlrShadingContext*  createEditableShadingContextCopy(bool copyShadingLayers = true) const;

    //! Number of assigned shading channels.
    int32_t  nShadingChannels() const;

    //! Set all param channels in the Stx to 0. Does nothing if Stx is not writable.
    void     setShadingParamsToZero();

    // Set a writable shading layer value by Channel index. The Channel index references
    // the first element in the value and the elements are assumed to be packed in order in
    // the array.
    // Shading layer channels are arbitrary data channels shared between shaders through the
    // SlrShadingContext and are separate from the AOV channels the renderer outputs.
    // These methods do nothing if this SlrShadingContext is not writable.
    // Returns true if write is successful.
    // See the PrimAttrib accessor methods for reading primvars.

    virtual bool    setIntShadingLayer(ndk::Channel chan, int32_t value) = 0;
    virtual bool  setFloatShadingLayer(ndk::Channel chan, float   value) = 0;
    virtual bool  setVec2fShadingLayer(ndk::Channel startChan, const fdk::Vec2f& value) = 0;
    virtual bool  setVec3fShadingLayer(ndk::Channel startChan, const fdk::Vec3f& value) = 0;
    virtual bool  setVec4fShadingLayer(ndk::Channel startChan, const fdk::Vec4f& value) = 0;

    virtual bool      setDual2fShadingLayer(const Dual2Chans& chans, const Dual2f& value) = 0;
    virtual bool      setDual2dShadingLayer(const Dual2Chans& chans, const Dual2d& value) = 0;
    virtual bool  setDual2Vec2fShadingLayer(const Dual2Chans& chans, const Dual2Vec2f& value) = 0;
    virtual bool  setDual2Vec3fShadingLayer(const Dual2Chans& chans, const Dual2Vec3f& value) = 0;
    virtual bool  setDual2Vec4fShadingLayer(const Dual2Chans& chans, const Dual2Vec4f& value) = 0;


  public:
    //! Thread memory context.
    const SlrThreadContext&  threadContext() const;

    //! Integrator context that created this shading context.
    SlrIntegratorContext&  Itx() const;

    //! Absolute time of the context, same as sampleInfo().time.
    fdk::TimeValue  time() const;

    //! Sample info of the context containing the sample time, motion step,
    //! motion step offset, etc.
    const SampleInfo&  sampleInfo() const;

    //! Thread index from current SlrThreadContext, guaranteed to be unique and never reused.
    uint32_t  threadIndex() const;

    //! Thread id from current SlrThreadContext.
    DD::Image::ThreadId  threadId() const;

    //! Retrieve the optional thread-safe data blob an SlrShader can create in
    //! SlrShader::_initializeForShadingThread().
    SlrShaderThreadData&  getShaderThreadData(const SlrShader* shader) const;

    //! Same but uses shader index Id (SlrShader::identifierIndex()).
    SlrShaderThreadData&  getShaderThreadData(int32_t shaderIdentifierIndex) const;

    //! Sampling random number generator to use for this subpixel sample.
    ndk::SamplingRng&  samplingRng() const;

    //! Ray id of \a srcRtx
    int32_t   rayId() const { return srcRtx().id; }

    //! Ray hit index in a bundle/stack of shading contexts. Typically the ray hit index
    //! order is the surface visibility order from front to back where hit index 0 is
    //! the closest hit.
    int32_t  hitIndex() const;

    //! Unique ID of the current intersection renderable/subpart/subtri.
    uint64_t  primId() const;

    //! Path of prim currently being evaluated - for debugging only!
    const std::string&  primPath() const;

    //! Renderable currently being evaluated.
    const SlrPrimInfo&  primInfo() const;

    //! Surface quality flags from the renderable - same as calling primInfo().flags;
    //! See SlrPrimInfo for flag definitions.
    SlrPrimInfo::PrimFlag  primFlags() const;

    //! PrimInfo index of current intersected object in SlrEngineContext.
    int32_t   renderableIndex() const;

    //! This context's shading data can be written to.
    bool      isWritable() const;

    //----------------------------------------------------------

    //! Opacity traits for this renderable prim, defined by the surface shader assigned to
    //! the prim being shaded.
    ndk::OpacityTraitsMask  surfaceOpacityTraits() const;

    //! Override / set the opacity traits mask, destroying the mask derived from the
    //! surface shader.
    void                    setSurfaceOpacityTraits(ndk::OpacityTraitsMask mask);

    //! 100% opaque, 100% presence, 0% transmission
    bool  isTriviallyOpaque() const { return ndk::isTriviallyOpaque(surfaceOpacityTraits()); }

    //! Presence may vary across surface
    bool  doesPresenceVary() const { return ndk::doesPresenceVary(surfaceOpacityTraits()); }
    //! Opacity may vary across surface
    bool  doesOpacityVary() const { return ndk::doesOpacityVary(surfaceOpacityTraits()); }
    //! Varying opacity is used for presence threshold
    bool  doesOpacityCreatePresence() const { return ndk::doesOpacityCreatePresence(surfaceOpacityTraits()); }

    //! Is this a thin surface, allowing light transmission with no refraction
    bool  doesThinInterfaceTransmission() const { return ndk::doesThinInterfaceTransmission(surfaceOpacityTraits()); }
    //! Will light propagate into surface interior, possibly refracting
    bool  doesInteriorTransmission() const { return ndk::doesInteriorTransmission(surfaceOpacityTraits()); }

    //! Solid interior, light propagates through possibly scattering
    bool  doesSubsurfacePropagation() const { return ndk::doesSubsurfacePropagation(surfaceOpacityTraits()); }
    //! Volume interior, light propagates through volume
    bool  doesSubsurfaceVolume() const { return ndk::doesSubsurfaceVolume(surfaceOpacityTraits()); }

    //! Is surface blending required?
    bool  doesBlend() const { return ndk::doesSurfaceBlend(surfaceOpacityTraits()); }
    //! Can the surface produce 100% opacity after blending
    bool  doesBlendToFullyOpaque() const { return ndk::doesSurfaceBlendToFullyOpaque(surfaceOpacityTraits()); }
    //! Does blending require opacity (over, under)
    bool  doesBlendUseOpacity() const { return ndk::doesSurfaceBlendUseOpacity(surfaceOpacityTraits()); }

    //! Does the surface act as a cutout/holdout/matte
    bool  isCutoutSurface() const { return ndk::isCutoutSurface(surfaceOpacityTraits()); }
    //! If a cutout surface does depth and other data aovs get cut as well
    bool  doesCutoutRemoveAovs() const { return ndk::doesCutoutRemoveAovs(surfaceOpacityTraits()); }

    //----------------------------------------------------------
    // Global rendering info:

    //! World-space values are transformed into shading-space by this matrix,
    //! which is commonly just a translate.
    //!
    //! Note - Shaders that have world-space values passed as inputs from scene
    //! shader nodes need to transform those values by this matrix to put them
    //! into shading-space. Lights and objects are transformed by the renderer
    //! during setup and have this xform applied.
    const fdk::Mat4d&  worldToShadingXform() const;

    //! Inverse of worldToShadingXform, for going from shading space to world.
    const fdk::Mat4d&  shadingToWorldXform() const;

    //! Same as worldToShadingXform but single-precision for ray offset convenience.
    const fdk::Vec3f&  worldToShadingOffset() const;

    //! Shaders should output color layers + alphas.
    bool  enableColorChannels() const;
    //! Shaders should output any data channels (Z, etc)
    bool  enableDataChannels() const;

    //! Texture filter to use. If null the texture will be impulse sampled.
    DD::Image::TextureFilter*  textureFilter() const;
    void                       setTextureFilter(DD::Image::TextureFilter* filter);

    //----------------------------------------------------------

    //! Local-to-world matrix for current primitive
    const fdk::Mat4d& l2w() const;

    //! World-to-local matrix for current primitive
    const fdk::Mat4d& w2l() const;

    //! Render camera world-to-eye xform (world-space coords to camera local-space)
    const fdk::Mat4d& camWorld2Eye() const;

    //! Render camera eye-to-world xform (camera local-space coords to world-space)
    const fdk::Mat4d& camEye2World() const;

    //! Render camera origin (camera rays originate here).
    //! Same as camEye2World().getTranslation(), but cached for speed.
    const fdk::Vec3d& camP() const;

    //! Hero-view camera origin in world space - for specular copying
    const fdk::Vec3d& camHeroP() const;

    //! If enabled viewVector() uses camHeroP() instead of camP() if \a rayDepth == 0.
    //! When \a rayDepth is > 0 (ie not a camera ray) then this ignored.
    bool  useHeroCamForSpec() const;

    //----------------------------------------------------------

    //! If true, even if no lights the surface shaders should do illum calcs.
    bool  globalLightingEnabled() const;

    //! If true do direct-illumination calculations.
    bool  globalDirectLightingEnabled() const;

    //! If true handle indirect-illumation calculations.
    bool  globalIndirectLightingEnabled() const;

    //! If true do shadow occlusion for direct-illumination.
    bool  globalShadowingEnabled() const;

    //! Adds light globally to the diffuse (albedo) surface response of surface shaders
    //! which respect this shading global.
    const fdk::Vec3f&  ambientLightingBias() const;

    //! List of all active lights. If \a lightingEnabled is false an empty list is returned.
    const SlrLightInfoList&  allLights() const;

    //! List of lights enabled for this renderable, always a subset of allLights().
    //! Each SlrPrimEnabledLighting struct in this list is built for each renderable
    //! so that each renderable's illumination and shadow visibility can be unique.
    const SlrPrimEnabledLightingList&  enabledLights() const;

    //! Returns true if the enabled light can shadow this surface.
    bool  surfaceCanReceiveShadowsFor(const SlrPrimEnabledLighting& primLightingInfo) const { return primLightingInfo.receiveShadow(); }

    //! Returns true if the enabled light can shadow this surface.
    bool  surfaceCanCastShadowsFor(const SlrPrimEnabledLighting& primLightingInfo) const { return primLightingInfo.castShadow(); }

    //----------------------------------------------------------

    //! Did a camera ray create this shading context?
    bool   isCameraPath() const { return srcRtx().isCameraPath(); }

    //! Is the ray that created this shading context evaluating thin transmission?
    bool   isThinTransmittedPath() const { return srcRtx().isThinTransmittedPath(); }

    //! Did a shadow ray create this shading context?
    bool   isShadowPath() const { return srcRtx().isShadowPath(); }
    //! Did a reflected ray create this shading context?
    bool   isReflectedPath() const { return srcRtx().isReflectedPath(); }
    //! Did a refracted ray create this shading context?
    bool   isRefractedPath() const { return srcRtx().isRefractedPath(); }

    //! Index of refraction (ior) of the hit surface.
    float  indexOfRefraction() const { return _indexOfRefraction; }
    void   setIndexOfRefraction(float ior) { _indexOfRefraction = ior; }

    //! Index of refraction (ior) of the 'outside' of the hit surface. This value is
    //! automatically determined when evaluating transmission rays, usually by taking
    //! the 'dielectric priority' of the surface into account to determine which surface
    //! is more important (which can ignored, really.)
    float  outsideIndexOfRefraction() const;

    //! Which side of the geometry should be shaded. This is not about intersection testing
    //! it's whether a surface shader should be run on the front side, back side or both.
    int8_t  shadeSides() const { return _shadeSides; }
    bool    shadeBothSides() const { return (_shadeSides == ShadeSides::Both); }
    bool    shadeFrontSide() const { return (_shadeSides == ShadeSides::Front); }
    bool    shadeBackSide()  const { return (_shadeSides == ShadeSides::Back); }
    void    setShadeSides(int8_t sides) { _shadeSides = sides; }

    //----------------------------------------------------------
    // Ray intersection testing

    //! Returns true if anything is occluding the ray, including transparent objects.
    //! Does not call shaders or return information about the hit object.
    bool  trace(const fdk::Vec3f&         origin,
                const fdk::Vec3f&         dir,
                ndk::RayContext::TypeMask type = ndk::RayContext::shadowRay(),
                float                     minDist = defaultMinDistance(),
                float                     maxDist = defaultMaxDistance()) const;

    //! Returns true if anything is occluding the ray, including transparent objects.
    //! Does not call shaders or return information about the hit object.
    bool  trace(const SlrRayContext& Rtx) const;

    //! Returns true if anything is occluding the ray, filling in \a hitPoint with
    //! the nearest surface information, including hit distance and opacity.
    //! Calls presence and opacity shaders only.
    bool  trace(const SlrRayContext& Rtx, ndk::RayHitPointf& hitPoint) const;

    //! Find the 'thin' transmission factor for all surfaces between Rtx.minT and Rtx.maxT,
    //! which is most commonly used for direct lighting shadow casting. Returns true on
    //! trivial 0% transmission (100% occlusion.)
    //! See SlrIntegratorContext::getThinTransmissionFactor() for more info.
    bool  getThinTransmissionFactor(const SlrRayContext& Rtx,
                                    fdk::Vec3f&          transmissionFactor) const;

    //! Distance from \a Pg to the nearest not-occluded polygon edge for the current primitive
    //! (squared). If faceEdgesOnly is true then only the outside edged of the polygon face are
    //! tested against, otherwise the method checks all interior polygon edges (usually
    //! triangles).
    //! 
    //! Note - returns the closest distance *squared*. This is done to avoid an expensive sqrt()
    //! which the caller has the option of doing, but often it's cheaper to square the threshold
    //! value being compared to.
    float getPolygonEdgeDistanceSquared(const bool faceEdgesOnly) const;

    //----------------------------------------------------------

    //! Current output screen coords
    const fdk::Vec2i&  pixelXY() const;
    //! Current output subpixel coord inside pixelXY, where [0 0] is the center of the pixel
    //! and [-1 -1] is the lower-left corner.
    const fdk::Vec2f&  subpixelXY() const;

    //! Current pixel subsample/camera-ray index
    int32_t  sampleIndex() const;
    int32_t  primaryRayIndex() const { return sampleIndex(); }

    //! Current presence threshold 0..1. If a hit surface's presence is below this value then
    //! it's considered to have no presence, not a hit, and the ray continues. This value will
    //! change randomly after each presence test.
    float    presenceThreshold() const;

    // Conveniences for testing output pixel coords and sample index in shaders
    bool  isDebugSample() const;
    bool  isDebugPixel() const;
    bool  isPixel(int32_t px, int32_t py) const;
    bool  isPixelX(int32_t px) const;
    bool  isPixelY(int32_t py) const;

    //-------------------------------------------------------
    // Built-in intrinsic shading parameters:

    //! Returns a 'fake' stereo view vector built from the camHeroP, or the negated
    //! ray direction depending on context's stereo mode and \a rayDepth. When \a rayDepth > 0
    //! (not a camera ray) then the inverted srcRtx().direction is returned.
    //!
    //! For camera rays using the camHeroP camera ray makes specular calculations identical
    //! for all stereo cameras, visually 'flattening' the specular onto the surface.
    const fdk::Vec3f&   viewVector() const;

    //!< Distance of this context's P from srcRtx().origin.
    float               distance() const;
    //!< Linear-projected distance (classic Z). Only valid if camera ray.
    float               Zlinear() const;

    //! Displaced shading point in common shading space where lighting is queried.
    const fdk::Vec3f&     Ps() const;
    //! Geometric point (intersection) in common shading space.
    const fdk::Vec3f&     Pg() const;

    //! P differential for dyDir ray direction. ('dx' is the ray cone x-axis, not screen-space 'x')
    const fdk::Vec3f&   dPgdx() const;
    //! P differential for dyDir ray direction. ('dy' is the ray cone y-axis, not screen-space 'y')
    const fdk::Vec3f&   dPgdy() const;

    //! P surface u-space partial derivative, not normalized.
    const fdk::Vec3f&   dPgdu() const;
    //! P surface v-space partial derivative, not normalized.
    const fdk::Vec3f&   dPgdv() const;
    //! P surface z-space partial derivative, not normalized. (for volumes)
    const fdk::Vec3f&   dPgdw() const;

    //! Displaced shading point in world space where lighting is queried (common shading space offset by worldToShadingXform().)
    fdk::Vec3f          PWs() const;
    //! Geometric point (intersection) in world space (common shading space offset by worldToShadingXform().)
    fdk::Vec3f          PWg() const;

    //! Geometric position at motion sample start
    const fdk::Vec3f&  PgStart() const;
    //! Geometric position at motion sample end
    const fdk::Vec3f&    PgEnd() const;

    //! Motion vector forward, only created on demand by calling generateMotionVectorAovs().
    const fdk::Vec2f&  PgMotionFwd() const;
    //! Motion vector backwards, only created on demand by calling generateMotionVectorAovs().
    const fdk::Vec2f&  PgMotionBwd() const;

    //! Barycentric triangle uvw coordinates at srcRtx intersection.
    //! For a triangle with corners (p0 p1 p2) w=1 is along edge p0->p1, u=1 is along edge p1->p2,
    //! and v=1 is along edge p0->p2.
    //! If v or w are outside 0..1 then P is outside the bounds of the triangle and w in invalid. This
    //! happens when uvw is calculated for ray offsets dxDir & dyDir. They will always intersect the
    //! triangle plane but can often be outside the triangle's perimeter.
    const fdk::Vec3f& triUvw() const;

    //! Shading normal, normalized
    const fdk::Vec3f&     Ns() const;
    //! Geometric surface normal, normalized
    const fdk::Vec3f&     Ng() const;
    //! Shading tangent, normalized
    const fdk::Vec3f&     Ts() const;

    //! Ns() but facing toward 'view' vector (ray direction) so that backside surfaces can
    //! shade correctly.
    fdk::Vec3f  NsFaceForward() const;
    //! Ng() but facing toward 'view' vector (ray direction) so that backside surfaces can
    //! shade correctly.
    fdk::Vec3f  NgFaceForward() const;

    //! Return the vector facing toward the 'view' vector (ray direction) so that backside
    //! surfaces can shade correctly. Negates the vector if it's facing away otherwise
    //! vector is returned unchanged.
    fdk::Vec3f  faceForward(const fdk::Vec3f vector) const;

    //! Surface 2D parametric coordinates of P
    //! Note - Parametric UVs are required for dPdu & dPdv and stable bump mapping, etc,
    //! and these are not neccessarily 'texture' UVs. For example texture coords can be projected
    //! onto a surface which can break the parametric-ness of the UVs along the surface. To avoid
    //! this we keep them logically separated to make sure shading operations like bump mapping
    //! are not adversely impacted.
    //! When not projecting texture coords the value of uv() and st() will likely be identical.
    const fdk::Vec2f&     uv() const;
    //! Parametric-uv differential for dxDir ray direction
    const fdk::Vec2f&  dUVdx() const;
    //! Parametric-uv differential for dyDir ray direction
    const fdk::Vec2f&  dUVdy() const;

    //! Surface Z parametric coordinate of P (for volumes)
    float                uvw() const;
    //! Parametric-w differential for dxDir ray direction
    float             dUVWdx() const;
    //! Parametric-w differential for dyDir ray direction
    float             dUVWdy() const;

    //! Note - presence contributes to pixel coverage weight, it does -not- influence opacity!
    //! 1.0 = solid, 0.0 = no object. Fractional values indicate multiple 0% or 100% samples
    //! have been combined together, usually due to filtering a texture.
    float              presenceWeight() const;

    //! Rgb Opacity for colored transparency and thin-transmission.
    const fdk::Vec3f&  Oi() const;
    //! Float version returns Oi.x (assumes all Oi elements are same value.)
    float              opacity() const;

    //-------------------------------------------------------
    // Built-in shading layers:

    //! Texture st(uv) coordinate (with w divided out)
    const fdk::Vec2f&     st() const;
    //! Texture coord st differential for dxDir ray direction
    const fdk::Vec2f&  dStdx() const;
    //! Texture coord st differential for dyDir ray direction
    const fdk::Vec2f&  dStdy() const;
    //! Texture coord st partial derivative in parametric-u direction, not normalized
    const fdk::Vec2f&  dStdu() const;
    //! Texture coord st partial derivative in parametric-u direction, not normalized
    const fdk::Vec2f&  dStdv() const;

    //! Interpolated vertex color + alpha-transparency.
    const fdk::Vec4f&     Cf() const;
    const fdk::Vec4f&   rgba() const;
    const fdk::Vec3f&    rgb() const;
    float              alpha() const;
    float                  r() const;
    float                  g() const;
    float                  b() const;
    float                  a() const;

    // Predefined writable contribution shading layers for convenience use in surface shaders.
    // A shader can write values to these always-available layers and they can be routed to
    // output AOVs.

    //! Surface albedo rgb reflectance.
    fdk::Vec3f&  albedoColor() const;

    //! Surface diffuse scatter lobe contribution.
    fdk::Vec3f&  diffuseLobe() const;

    //! Surface specular reflection lobe contribution.
    fdk::Vec3f&  specularLobe() const;

    //! Surface transmission lobe contribution.
    fdk::Vec3f&  transmissionLobe() const;

    //! Surface emission lobe contribution.
    fdk::Vec3f&  emissionLobe() const;

    //! Surface accumulated shadow color.
    fdk::Vec3f&  shadowColor() const;


    //-------------------------------------------------------
    // Aov Handling

    //! Retrieve value from Aov, if valid. If not valid returned value is 0,0,0.
    fdk::Vec3f  getAov(const SlrAovLayer& aov) const;

    //! Copy Aov values from this shading context's shading channels into the output Pixel, and
    //! call any custom Aov handlers for them to copy their values to the output Pixel.
    void  aovsCopy(DD::Image::Pixel& outputPixel) const;

    //! Copy all Aov channels from pixel A to pixel B. If the Aov is set to mode MERGE_PREMULT_BLEND
    //! then the A channel is premulted by A.coverage() before copying to B. Note that opacity is not
    //! included in this premult!
    void  aovsAReplaceB(const DD::Image::Pixel& A, DD::Image::Pixel& B) const;

    //! Merge UNDER the Aov channels from pixel A under the matching pixel B channels, if the Aov can
    //! be blended. If the Aov is set to mode MERGE_PREMULT_BLEND then the A channel is premulted
    //! by A.coverage() while merging UNDER B. Note that opacity is not included in this premult!
    //!
    //! Aovs set to non-blend modes (min, max, etc) have that function applied and are NOT UNDERed.
    //!
    //! When \a sampleIndex is 0 then aovsAReplaceB() is called and nothing more is done.
    void  aovsAUnderB(int32_t sampleIndex, float Balpha, const DD::Image::Pixel& A, DD::Image::Pixel& B) const;
#if kDDImageVersionMajorNum <= 17
    mFnDeprecatedInNuke17("Use aovsAUnderB(int, float Balpha, ) instead.")
    void  aovsAUnderB(int32_t sampleIndex, const DD::Image::Pixel& A, DD::Image::Pixel& B, float Balpha) const;
#else
#error "Remove the above code block in Nuke 18 as they are deprecated in Nuke 17."
#endif

    //! Merge OVER all Aov channels from pixel A over the matching pixel B channels, if the Aov can
    //! be blended. If the Aov is set to mode MERGE_PREMULT_BLEND then the A channel is premulted
    //! by A.coverage() while merging OVER B. Note that opacity is not included in this premult!
    //!
    //! Aovs set to non-blend modes (min, max, etc) have that function applied and are NOT OVERed.
    //!
    //! When \a sampleIndex is 0 then aovsAReplaceB() is called and nothing more is done.
    void  aovsAOverB(int32_t sampleIndex, const DD::Image::Pixel& A, DD::Image::Pixel& B) const;

    //! Create the motion vector values for the Aov layer Chan_MotionFwd.
    //! This should only be called on demand to reduce render cost by checking the IntegratorContext
    //! motionVectorsRequested() value.
    void  createMotionVectorAovs(DD::Image::Pixel& out, bool force = false) const;

    //----------------------------------------------------------

    // Retrieve a shading layer value by Channel index. The Channel index references
    // the first element in the value and the elements are assumed to be packed in order in
    // the array.
    // Shading layer channels are arbitrary data channels shared between shaders through the
    // SlrShadingContext and are separate from the AOV channels the renderer outputs.
    // See the PrimAttrib accessor methods for reading geometry primvars.

    int32_t               getIntShadingLayer(ndk::Channel chan, int32_t def_val) const;
    float               getFloatShadingLayer(ndk::Channel chan, float   def_val) const;
    double             getDoubleShadingLayer(ndk::Channel chan, double  def_val) const;
    const fdk::Vec2f&   getVec2fShadingLayer(ndk::Channel startChan, const fdk::Vec2f& def_val) const;
    const fdk::Vec3f&   getVec3fShadingLayer(ndk::Channel startChan, const fdk::Vec3f& def_val) const;
    const fdk::Vec4f&   getVec4fShadingLayer(ndk::Channel startChan, const fdk::Vec4f& def_val) const;
                
    Dual2f          getDual2fShadingLayer(const Dual2Chans& chans, float  def_val) const;
    Dual2d          getDual2dShadingLayer(const Dual2Chans& chans, double def_val) const;
    Dual2Vec2f  getDual2Vec2fShadingLayer(const Dual2Chans& chans, const fdk::Vec2f& def_val) const;
    Dual2Vec3f  getDual2Vec3fShadingLayer(const Dual2Chans& chans, const fdk::Vec3f& def_val) const;
    Dual2Vec4f  getDual2Vec4fShadingLayer(const Dual2Chans& chans, const fdk::Vec4f& def_val) const;

    //----------------------------------------------------------

    // Retrieve a primitive attribute (primvar) value by Channel index. These are functionally the
    // same as calling the ShadingLayer versions because the underlying prim data, if it's available,
    // is automatically copied into the ShadingContext's shading layers.
    //
    // The Channel index references the first element in the value and the elements are assumed to
    // be packed in order in the array. Primitive attributes are retrieved from the primitive once
    // a surface is hit and the SlrShadingContext is filled in.
    // These values are usually read only since a shader does not have access to the raw primitive
    // data, but if required they can be set via the setShadingLayer() methods using the primvar's
    // channel indices. Warning, writing to primvar channels can lead to unpredictable results!

    int32_t               getIntPrimAttrib(ndk::Channel chan, int32_t def_val) const { return getIntShadingLayer(chan, def_val); }
    float               getFloatPrimAttrib(ndk::Channel chan, float   def_val) const { return getFloatShadingLayer(chan, def_val); }
    double             getDoublePrimAttrib(ndk::Channel chan, double  def_val) const { return getDoubleShadingLayer(chan, def_val); }
    const fdk::Vec2f&   getVec2fPrimAttrib(ndk::Channel startChan, const fdk::Vec2f& def_val) const { return getVec2fShadingLayer(startChan, def_val); }
    const fdk::Vec3f&   getVec3fPrimAttrib(ndk::Channel startChan, const fdk::Vec3f& def_val) const { return getVec3fShadingLayer(startChan, def_val); }
    const fdk::Vec4f&   getVec4fPrimAttrib(ndk::Channel startChan, const fdk::Vec4f& def_val) const { return getVec4fShadingLayer(startChan, def_val); }

    Dual2f          getDual2fPrimAttrib(const Dual2Chans& chans, float  def_val) const { return getDual2fShadingLayer(chans, def_val); }
    Dual2d          getDual2dPrimAttrib(const Dual2Chans& chans, double def_val) const { return getDual2dShadingLayer(chans, def_val); }
    Dual2Vec2f  getDual2Vec2fPrimAttrib(const Dual2Chans& chans, const fdk::Vec2f& def_val) const { return getDual2Vec2fShadingLayer(chans, def_val); }
    Dual2Vec3f  getDual2Vec3fPrimAttrib(const Dual2Chans& chans, const fdk::Vec3f& def_val) const { return getDual2Vec3fShadingLayer(chans, def_val); }
    Dual2Vec4f  getDual2Vec4fPrimAttrib(const Dual2Chans& chans, const fdk::Vec4f& def_val) const { return getDual2Vec4fShadingLayer(chans, def_val); }


    //----------------------------------------------------------
    // Texture sampling calls:

    //! Use st() and its xy derivatives dStdu() and dStdv() to sample the incoming image.
    //! The st coord 0,0 is mapped to the lower-left corner of the image's format, and 1,1
    //! is mapped to the upper-right corner.
    //!
    //! For texture sampling it's much faster to provide a DD::Image::Sampler rather than
    //! a raw Iop so Tile caching is properly leveraged, however the Sampler must not be
    //! shared between shading threads or its tile cache may crash.
    void   sample(DD::Image::Sampler*          sampler,
                  const DD::Image::ChannelSet& get,
                  DD::Image::Pixel&            out) const;

    //! Same but for sampling a single channel.
    float  sample(DD::Image::Sampler* sampler, DD::Image::Channel chan) const;

    //! Same as sample(Sampler*) but with overrides for st & derivatives currently in context.
    void   sample(DD::Image::Sampler*          sampler,
                  const Dual2Vec2f&            st,
                  const DD::Image::ChannelSet& get,
                  DD::Image::Pixel&            out) const;

    //! Same but for sampling a single channel.
    float  sample(DD::Image::Sampler* sampler, const Dual2Vec2f& st, DD::Image::Channel chan) const;

    //--------------------------------

    //! Slower versions using a raw Iop.
    void   sample(DD::Image::Iop*              texture,
                  const DD::Image::ChannelSet& get,
                  DD::Image::Pixel&            out) const;

    //! Same but for sampling a single channel.
    float  sample(DD::Image::Iop* texture, DD::Image::Channel chan) const;

    //! Same as sample(Iop*) but with overrides for st & derivatives currently in context.
    void   sample(DD::Image::Iop*              texture,
                  const Dual2Vec2f&            st,
                  const DD::Image::ChannelSet& get,
                  DD::Image::Pixel&            out) const;

    //! Same but for sampling a single channel.
    float  sample(DD::Image::Iop* texture, const Dual2Vec2f& st, DD::Image::Channel chan) const;


    //------------------------------------------------------------------------
    // Surface shading:

    //! Run the surface shader for this context, if valid, filling this context's
    //! \a shadedOutput() Pixel with the results.
    //!
    //! This method first calls surfaceShader->surfaceShader() then overwrites the shaded
    //! output Pixel's Z (Chan_Z), Zf (Chan_DeepFront), Zb (Chan_DeepBack) and coverage
    //! (Chan_Coverage) channels with this context's values. This stops surface shaders
    //! from adversely affecting the Z location and coverage weight of the surface
    //! sample.
    //!
    //! If \a applyPresenceWeight is true then the context's presenceWeight() value
    //! is used to attenuate (multiply down) only the -color- channels of the shaded
    //! output Pixel, leaving the data channels are unaffected (note that this uses
    //! the integrator's \a shaderChannelSet()). Shaded output \a coverage() is
    //! copied from presenceWeight().
    //!
    //! Note that Z() will always be ray distance and not Zlinear, while Zf() and Zb()
    //! will contain Zlinear. coverage() is copied from presenceWeight() as at this
    //! point in the shading pipeline the sample's 'pixel coverage' is simply whether
    //! it has presence or not. See presenceWeight() for more info.
    //!
    //! If the surface isCutoutSurface() then all the shaded color channels will be
    //! left zeroed however coverage, opacity and depth are left as-is so they can
    //! be used in layering.
    void  getShadedSurfaceOutput(bool applyPresenceWeight = true);

    //! Final output, or shaded data for this surface sample, typically produced
    //! by a surface shader called by \a getShadedSurfaceOutput(). This Pixel contains
    //! all the output channel data including data AOVs.
    //!
    //! This Pixel is fully read/write and an integrator or filter is free to change
    //! its contents as needed, even affected the Z and coverage values. However
    //! changing these channels may adversely affect the renderer's output layering
    //! logic.
    //!
    //! These output channels can be layered together with other surface samples to
    //! produce a final 'flat' output pixel, or can be kept separate as deep sample
    //! data.
    //!
    //! Note - this data is only valid while this shading context exists so it
    //! must be copied to the final Iop or DeepOp output, typically in an engine
    //! method according to that engine's rules. For example, channels written into
    //! which the Iop/DeepOp engine is unaware of may be lost.
    DD::Image::Pixel&  shadedOutput() const;


    //------------------------------------------------------------------------
    // Thread-safe scratch Pixels to be used during ray-tracing & shading.
    //
    // Warning - since these are shared variables they should only be
    // used at the end of a shader chain, for example when sampling
    // textures or light values. If passed up a shader graph there's
    // a good chance of conflict with another shader attempting to use
    // the same var.

    //! Per-thread context - for sampling texture map Iops - defaults to RGBA
    //! Note - has InterestRatchet set so should sample faster.
    DD::Image::Pixel&  textureColor() const;
    //! Per-thread context - for Connection sampling calls - defaults to RGBA
    DD::Image::Pixel&  bindingColor() const;

    //! Per-ray - for LightShader evaluation - defaults to RGB
    DD::Image::Pixel&  illumColor() const;
    //! Per-ray - for VolumeShader color evaluation - defaults to RGB
    DD::Image::Pixel&  volumeColor() const;
    //! Per-ray - or surface shader calls to retrieve indirect results - defaults to RGB
    DD::Image::Pixel&  indIllumColor() const;


    //------------------------------------------------------------------------
    //!
    void printParamData(std::ostream&) const;

  protected:
    //! Member vars left intentionally uninitialized for performance reasons.
    SlrShadingContext() = default;

    friend class ThreadContext;
    friend class StxBundle;
    friend class RenderEngine;
    friend class TraceableFaceTopology;
    uint8_t                   _shadeSides;              //!< Which sides to shade
    float                     _indexOfRefraction;       //!< Index of refraction of the hit surface
    DD::Image::TextureFilter* _textureFilter;           //!< Global texture filter to use. May be null!
    friend DDImage_API std::ostream& operator << (std::ostream&, const SlrShadingContext&);
  };

  using SlrShadingContextPtrList = std::vector<SlrShadingContext*>;


  //-----------------------------------------------------------------------------


  //! \class StxBundle
  //! Stores a list of StxShadingContext generated by a ray trace call.
  class DDImage_API StxBundle
  {
  public:
    using ListT = std::vector<class ShadingSample*>;

    //! Releases all the shading contexts in the list.
    ~StxBundle();

    //! Is the bundle bound to valid data?
    bool     isValid() const { return (_sttx != nullptr); }

    //! Returns true is bundle is not empty.
    explicit operator bool() const { return (!empty() && _sttx); }

    int32_t  size() const { return _list.size(); }
    bool     empty() const { return _list.empty(); }

    // Read/write accessors. No range checking or null testing performed!
    SlrShadingContext&  operator [] (int32_t i);
    SlrShadingContext&  operator [] (int32_t i) const;

    //! Sort the shading contexts from near to far distance where the first in the list is nearest.
    void  sort(bool nearToFar = true) { (nearToFar) ? sortNearToFar() : sortFarToNear(); }

    //! Sort the shading contexts from near to far distance where the first in the list is nearest.
    void  sortNearToFar();

    //! Sort the shading contexts from far to near distance where the first in the list is farthest.
    void  sortFarToNear();

    //! Retrieve the shading context at index \a i. No range checking or null testing performed!
    //! If the shading reference is empty (null) then an empty context is returned.
    //! Warning, accessing most of the empty context's methods may cause a crash, so check that it
    //! isValid() if in doubt.
    SlrShadingContext&  getShadingContext(int32_t i) const;

    //! Move/extract the shading context reference out of this bundle, transferring ownership of it
    //! to \a toBundle and setting the old reference to null.
    //! Note this does -NOT- change the size of the list, for performance reasons.
    void  moveShadingContextToBundle(int32_t i, StxBundle& toBundle);

    //! Same but for layering stacks, which accumulate the opacity masks.
    void  moveShadingContextToStack(int32_t i, StxLayeringStack& toStack);

    //! Sets size of bundle to 0 after releasing all the shading contexts in the bundle.
    //! Use this if you want to reuse the same bundle in another trace call.
    void  clear();

    //------------------------------------------------------------------------

    //! Iterator support
    struct DDImage_API iterator
    {
      explicit iterator(ListT::iterator it) noexcept : _it(it) {}

      SlrShadingContext&  operator *() noexcept;
      bool operator == (const iterator& b) const noexcept { return (_it == b._it); }
      bool operator != (const iterator& b) const noexcept { return (_it != b._it); }
      iterator& operator++() noexcept { ++_it; return *this; }

      explicit operator void*() noexcept { return &(*_it); } // for debugging

      iterator(const iterator&) = delete;
      void operator=(const iterator&) = delete;

    private:
      ListT::iterator _it;
    };

    using const_iterator = iterator;

    iterator        begin();
    const_iterator  begin() const;
    iterator        end();
    const_iterator  end() const;

    //----------------------------------------------------------

    friend DDImage_API std::ostream& operator << (std::ostream& o, const StxBundle&);

  protected:
    friend class ThreadContext;
    friend class SlrIntegratorContext;
    friend struct SlrRayContext;
    friend class RenderEngine;
    class ThreadContext* _sttx;
    ListT                _list;
    StxBundle(ThreadContext*);
    StxBundle(const StxBundle&) = delete;
    void operator=(const StxBundle&) = delete;
  };


  //-----------------------------------------------------------------------------


  //! A stack of surface shading contexts along a single ray path which can be layered
  //! together to form a flattened result.
  class DDImage_API StxLayeringStack : public StxBundle
  {
  public:
    //! Default ctor makes an invalid, empty bundle.
    StxLayeringStack();
    //! Shading stack owned by a ray.
    explicit StxLayeringStack(const SlrRayContext&);

    //! Sets size of bundle to 0 after releasing all the shading contexts in the bundle
    //! and resets the global parameters.
    //! Use this if you want to reuse the same bundle in another trace call.
    void reset();

    //! Ray index for all shading contexts in stack.
    int32_t  rayIndex() const;

    //! 
    SlrShadingContext&  addHitPoint(const ndk::RayHitPointf& hitPoint);

    //! OpacityTraitsMask of all the surfaces OR'd together.
    ndk::OpacityTraitsMask  opacityOrMask() const { return _opacityMaskOR; }

    //! OpacityTraitsMask of all the surfaces AND'd together.
    ndk::OpacityTraitsMask  opacityAndMask() const { return _opacityMaskAND; }

    //! All shading contexts are opaque-solid, that is we cannot see through the first surface
    //! and if desired can skip all other surfaces.
    bool  areAllHitsOpaqueSolid() const;

    //! If false then only the first surface matters since we should never see through it. If
    //! true then all the surfaces need to be blended, and if doAllHitsBlendToOpaque() is also
    //! true then blending can be done front-to-back.
    bool  doAllHitsBlend() const;

    //! If true all the surfaces do 'standard' blending that can result in 1.0 opacity. If false
    //! then at least one of the surfaces needs to do 'non-standard' blending (like min, max, plus)
    //! and blending should be done back-to-front.
    bool  doAllHitsBlendToOpaque() const;

    //! Does at least one of the surfaces act as a cutout/holdout/matte?
    bool  anyCutoutSurfaces() const;
    //! If there's a cutout surface does depth and other data aovs get cut as well?
    bool  anyCutoutsRemoveAovs() const;

    friend DDImage_API std::ostream& operator << (std::ostream& o, const StxLayeringStack& b);


  protected:
    friend class ThreadContext;
    friend class SlrIntegratorContext;
    friend struct RayData;
    struct RayData*        _owner;
    ndk::OpacityTraitsMask _opacityMaskOR;
    ndk::OpacityTraitsMask _opacityMaskAND;
    StxLayeringStack(ThreadContext*, struct RayData*);
    StxLayeringStack(const StxLayeringStack&) = delete;
    void operator=(const StxLayeringStack&) = delete;
  };


  //-----------------------------------------------------------------------------


  //! \class StxLayeringStackBundle
  //! Stores a list of references to StxLayeringStacks generated by the integrator.
  //! This bundle does not manage the memory of the stacks and only stores pointers
  //! to them.
  class DDImage_API StxLayeringStackBundle
  {
  public:
    using ListT = std::vector<StxLayeringStack*>;
    StxLayeringStackBundle();

    //! Returns true is bundle is not empty.
    explicit operator bool() const { return !empty(); }

    int32_t  size() const { return _list.size(); }
    bool     empty() const { return _list.empty(); }
    void     reserve(int32_t n) { _list.reserve(n); }

    //! Adds a reference the layering stack in ray \a Rtx to the bundle.
    void  addRayLayeringStack(const SlrRayContext& Rtx) { _list.push_back(&Rtx.layeringStack()); }

    //!
    void  addStack(StxLayeringStack& stack) { _list.push_back(&stack); }

    // Read/write accessors. No range checking or null testing performed!
    StxLayeringStack&  operator [] (int32_t i);
    StxLayeringStack&  operator [] (int32_t i) const;

    //! Clears the size of the bundle to zero, does not release any allocations.
    void  clear() { _list.clear(); }

    //------------------------------------------------------------------------

    //! Iterator support
    struct DDImage_API iterator
    {
      explicit iterator(ListT::iterator it) noexcept : _it(it) {}

      StxLayeringStack&  operator *() noexcept { return *(*_it); }
      bool operator == (const iterator& b) const noexcept { return (_it == b._it); }
      bool operator != (const iterator& b) const noexcept { return (_it != b._it); }
      iterator& operator++() noexcept { ++_it; return *this; }

      explicit operator void*() noexcept { return &(*_it); } // for debugging

      iterator(const iterator&) = delete;
      void operator=(const iterator&) = delete;

    private:
      ListT::iterator _it;
    };

    using const_iterator = iterator;

    iterator        begin();
    const_iterator  begin() const;
    iterator        end();
    const_iterator  end() const;

    //----------------------------------------------------------

    friend DDImage_API std::ostream& operator << (std::ostream& o, const StxLayeringStackBundle&);

  protected:
    friend class ThreadContext;
    friend class SlrIntegratorContext;
    friend struct SlrRayContext;
    ListT _list;

    StxLayeringStackBundle(const StxLayeringStackBundle&) = delete;
    void operator=(const StxLayeringStackBundle&) = delete;
  };


  //-----------------------------------------------------------------------------

  inline std::ostream& operator << (std::ostream& o, const Dual2f& b)
  {
    o << "(v" << b.val << " dx" << b.dx << " dy" << b.dy << ")";
    return o;
  }
  inline std::ostream& operator << (std::ostream& o, const Dual2d& b)
  {
    o << "(v" << b.val << " dx" << b.dx << " dy" << b.dy << ")";
    return o;
  }
  inline std::ostream& operator << (std::ostream& o, const Dual2Vec2f& b)
  {
    o << "(v" << b.val << " dx" << b.dx << " dy" << b.dy << ")";
    return o;
  }
  inline std::ostream& operator << (std::ostream& o, const Dual2Vec3f& b)
  {
    o << "(v" << b.val << " dx" << b.dx << " dy" << b.dy << ")";
    return o;
  }
  inline std::ostream& operator << (std::ostream& o, const Dual2Vec4f& b)
  {
    o << "(v" << b.val << " dx" << b.dx << " dy" << b.dy << ")";
    return o;
  }
  inline std::ostream& operator << (std::ostream& o, const Dual2Chans& b)
  {
    o << "(v=" << (int)b.val << " dx=" << (int)b.dx << " dy=" << (int)b.dy << ")";
    return o;
  }

  inline std::ostream& operator << (std::ostream& o, const ShadingLayerReq& b)
  {
    if (!b.empty()) {
      o << b.name << "[" << b.nChans << "]";
    }
    else {
      o << b.name << "<empty>";
    }
    return o;
  }

} // namespace slr


#endif
