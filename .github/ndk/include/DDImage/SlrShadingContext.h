//------------------------------------------------------------------------------
// DDImage/SlrShadingContext.h
//
// Copyright (c) 2021 The Foundry Visionmongers Ltd.  All Rights Reserved.
//------------------------------------------------------------------------------

#ifndef SLR_SHADINGCONTEXT_H
#define SLR_SHADINGCONTEXT_H

#include "DDImage/SlrPrimInfo.h" // for SlrLightInfoList
#include "DDImage/SlrAovLayer.h"

#include "ndk/geo/render/RayContext.h"
#include "ndk/base/Channel.h"

#include "DDImage/ChannelSet.h"
#include "DDImage/Pixel.h"
#include "DDImage/Thread.h" // for ThreadId

namespace DD {
  namespace Image {
    class Iop;
    class Pixel;
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
  class SlrShadingData;

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
    float          t;                 //!< Offset of time in motionStep
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
  static constexpr ndk::Channel  Chan_MotionFwd = 72; // Vec2f - forward motion-vectors
  static constexpr ndk::Channel  Chan_MotionBwd = 74; // Vec2f - backwards motion-vectors

  //------------------------------

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

    bool isAssigned() const { return (val != ndk::Chan_Empty); }
    bool empty() const { return !isAssigned(); }
  };

  // Built-in parameter Dual2Chans definitions:

  static constexpr Dual2Chans  Dual2Chans_Empty = {0, 0, 0};
  static constexpr Dual2Chans  Dual2Chans_Pg  = {12/*Chan_Pg*/ , 15/*Chan_dPgdx*/ , 18/*Chan_dPgdy*/ }; // Vec3f
  static constexpr Dual2Chans  Dual2Chans_UV  = {45/*Chan_UV*/ , 47/*Chan_dUVdx*/ , 49/*Chan_dUVdy*/ }; // Vec2f parametric-uv
  static constexpr Dual2Chans  Dual2Chans_UVw = {51/*Chan_UVW*/, 52/*Chan_dUVWdx*/, 53/*Chan_dUVWdy*/}; // float parametric-w (for volumes)
  static constexpr Dual2Chans  Dual2Chans_St  = {54/*Chan_St*/ , 56/*Chan_dStdx*/ , 58/*Chan_dStdy*/ }; // Vec2f texture coord

  //-----------------------------------------------------------------------------


  /*! This class is intended to be passed up a SlrShader tree.
  */
  class DDImage_API SlrShadingContext
  {
  public:
    ndk::RayContext     srcRtx;               //!< Ray that created this context
    //
    uint16_t            rayDepth;             //!< Ray depth that created this context where 0 indicates a camera, or primary ray
    uint16_t            diffuseDepth;
    uint16_t            reflectionDepth;
    uint16_t            refractionDepth;
    //
    SlrSurfaceShader*   surfaceShader;        //!< Current surface shader being evaluated
    SlrGeometryShader*  displacementShader;   //!< Current displacement shader being evaluated, if any
    SlrVolumeShader*    atmosphereShader;     //!< Current atmospheric volume shader being evaluated, if any

  public:
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
    //! Absolute time of the context
    fdk::TimeValue  time() const;

    //! Sample info of the context containing the sample time, motion step,
    //! motion step offset, etc.
    const SampleInfo&  sampleInfo() const;

    //! Thread id from current SlrThreadContext.
    DD::Image::ThreadId  threadId() const;

    //! Integrator context that created this shading context.
    SlrIntegratorContext&  Itx() const;

    //! Sampling random number generator to use for this subpixel sample.
    ndk::SamplingRng&  samplingRng() const;

    //! Ray id of \a srcRtx
    int32_t   rayId() const { return srcRtx.id; }

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
    // Global rendering info:

    //! World-space values are transformed into shading-space by this matrix,
    //! which is commonly just a translate.
    //!
    //! Note - Shaders that have world-space values passed as inputs from scene
    //! shader nodes need to transform those values by this matrix to put them
    //! into shading-space. Lights and objects are transformed by the renderer
    //! during setup and have this xform applied.
    const fdk::Mat4d&  globalSceneXform() const;

    //! Same as _globalSceneXform but single-precision for ray offset convenience.
    const fdk::Vec3f&  globalSceneOffset() const;

    //! Shaders should output color layers + alphas.
    bool  enableColorChannels() const;
    //! Shaders should output any data channels (Z, etc)
    bool  enableDataChannels() const;

    //! Should textures sample? This may be disabled for camera rays when shading
    //! a pre-rendered scene with pixel containing the surface albedo.
    bool  textureSamplingEnabled() const { return _textureSamplingEnabled; }
    void  setTextureSampling(bool enabled) { _textureSamplingEnabled = enabled; }
    void  enableTextureSampling() { setTextureSampling(true); }
    void  disableTextureSampling() { setTextureSampling(false); }

    //! Texture filter to use. If null the texture will be impulse sampled.
    DD::Image::TextureFilter*  textureFilter() const;
    void                       setTextureFilter(DD::Image::TextureFilter* filter);

    //! If true, even if no lights the surface shaders should do illum calcs.
    bool  lightingEnabled() const;

    //! If true do direct-illumination calculations.
    bool  directLightingEnabled() const;

    //! If true handle indirect-illumation calculations.
    bool  indirectLightingEnabled() const;

    //! If true do shadow occlusion for direct-illumination.
    bool  shadowingEnabled() const;

    //! Adds light globally to the diffuse (albedo) surface response of surface shaders
    //! which respect this shading global.
    const fdk::Vec3f&  ambientLightingBias() const;

    //! If enabled viewVector() uses camHeroP() instead of camP() if \a rayDepth == 0.
    //! When \a rayDepth is > 0 (ie not a camera ray) then this ignored.
    bool  useHeroCamForSpec() const;


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

    //! List of active lights. If \a lightingEnabled is false an empty list is returned.
    const SlrLightInfoList&  lights() const;

    //----------------------------------------------------------

    //! Did a camera ray create this shading context?
    bool   isCameraPath() const { return srcRtx.isCameraPath(); }

    //! Is the ray that created this shading context evaluating thin transmission?
    bool   isThinTransmittedPath() const { return srcRtx.isThinTransmittedPath(); }

    //! Did a shadow ray create this shading context?
    bool   isShadowPath() const { return srcRtx.isShadowPath(); }
    //! Did a reflected ray create this shading context?
    bool   isReflectedPath() const { return srcRtx.isReflectedPath(); }
    //! Did a refracted ray create this shading context?
    bool   isRefractedPath() const { return srcRtx.isRefractedPath(); }

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

    //! Surface blending order assigned during Integrator StxBundle contruction. The default surface
    //! shader blend method checks this order when deciding to use Under or Over blend math.
    //! The direction of blending also affects which blending modes can be used effectively since
    //! many on the 'non-standard' modes like min/max cannot be correctly performed when blending
    //! front-to-back. A surface shader is free to implement whatever blending math it wants to,
    //! but there's no guarantee the result will be correct for all blending orders unless this
    //! value is taken into consideration.
    int8_t  blendingOrder() const { return _blendingOrder; }
    bool    blendingOrderFrontToBack() const { return (_blendingOrder == BlendingOrder::FrontToBack); }
    bool    blendingOrderBackToFront() const { return (_blendingOrder == BlendingOrder::BackToFront); }
    bool    blendingOrderDisabled()    const { return (_blendingOrder == BlendingOrder::Disabled); }
    void    setBlendingOrder(int8_t order) { _blendingOrder = order; }

    //----------------------------------------------------------
    // Ray intersection testing

    //! Start a new RayContext bundle making it the current bundle and returning a reference
    //! to it. Note that only one bundle may be active at a time since each bundle must be a
    //! contiguous section of RayContexts in the cache.
    //! See SlrIntegratorContext for more info.
    SlrRayBundle&  startNewRayBundle(ndk::RayContext::TypeMask rayType) const;

    //! Make \a rayBundle be the active one that intersection testing is done on and store
    //! in. If the bundle has rays in it the active ray will be set to the first in the
    //! bundle.
    //! This is not required if startNewRayBundle() was just called.
    //! See SlrIntegratorContext for more info.
    void  selectActiveRayBundle(const SlrRayBundle& rayBundle) const;

    //! Access the currently active RayBundle for this thread which can then be added to. Note
    //! that only one bundle may be current at a time.
    //! See SlrIntegratorContext for more info.
    SlrRayBundle&  activeRayBundle() const;

    //! End ray bundle, releasing all the ray, intersection and shading info created for that
    //! bundle and changing the current bundle to the previous one in the list.
    //! See SlrIntegratorContext for more info.
    SlrRayBundle&  endRayBundle() const;


    //! Returns true if anything is occluding the ray, including transparent objects.
    //! Does not call shaders or return information about the hit object.
    bool  intersect(const ndk::RayContext& Rtx = ndk::RayContext()) const;

    //! Intersect the scene with the RayContext \a Rtx, returning true if there was a hit
    //! within \a Rtx's distance range, and filling in \a hitStx with the information
    //! about the hit surface.
    //! See SlrIntegratorContext::getNearestIntersection() for more info.
    bool  getNearestIntersection(const ndk::RayContext& Rtx,
                                 ndk::RayHitPointf&     hitPoint) const;

    //! Find the 'thin' transmission factor for all surfaces between Rtx.minT and Rtx.maxT,
    //! which is most commonly used for direct lighting shadow casting. Returns false on
    //! trivial 0% transmission (100% occlusion.)
    //! See SlrIntegratorContext::getThinTransmissionFactor() for more info.
    bool  getThinTransmissionFactor(const ndk::RayContext& Rtx,
                                    fdk::Vec3f&            transmissionFactor) const;

    /*! Distance from \a P to the nearest not-occluded polygon edge in screen-space
        for the current primitive.
        All triangles generated from the tessellation of the original SlrPrimInfo
        are tested, so the method checks all polygon edges.
        Returns when the first edge distance is under \a minDist.
    */
    float getPolygonEdgeDistance(float minDist) const;

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
    //! (not a camera ray) then the inverted srcRtx direction is returned.
    //!
    //! For camera rays using the camHeroP camera ray makes specular calculations identical
    //! for all stereo cameras, visually 'flattening' the specular onto the surface.
    const fdk::Vec3f&   viewVector() const;

    //!< Distance of this context's P from srcRtx origin.
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

    //! Displaced shading point in world space where lighting is queried (common shading space offset by globalSceneXform().)
    fdk::Vec3f          PWs() const;
    //! Geometric point (intersection) in world space (common shading space offset by globalSceneXform().)
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

    //! Ns() but facing toward 'view' vector (srcRtx.direction) so that backside surface
    //! can shade correctly.
    fdk::Vec3f  NsFaceForward() const;
    //! Ng() but facing toward 'view' vector (srcRtx.direction) so that backside surface
    //! can shade correctly.
    fdk::Vec3f  NgFaceForward() const;


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

    //! Merge UNDER all Aov channels from pixel A under the mathcing pixel B channels. If the Aov is
    //! set to mode MERGE_PREMULT_BLEND then the A channel is premulted by A.coverage() while merging
    //! UNDER B. Note that opacity is not included in this premult!
    void  aovsAUnderB(int32_t sampleIndex, const DD::Image::Pixel& A, DD::Image::Pixel& B, float Balpha) const;

    //! Merge OVER all Aov channels from pixel A under the mathcing pixel B channels. If the Aov is
    //! set to mode MERGE_PREMULT_BLEND then the A channel is premulted by A.coverage() while merging
    //! OVER B. Note that opacity is not included in this premult!
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

    /*! Use st() and the derivatives of st() to sample the incoming image.
        The st coord 0,0 is mapped to the lower-left corner of the image's format,
        and 1,1 is mapped to the upper-right corner.

        For texture sampling it's much faster to provide a DD::Image::Sampler
        rather than a raw Iop so Tile caching is properly leveraged.
    */
    void sample(DD::Image::Sampler*          sampler,
                const DD::Image::ChannelSet& get,
                DD::Image::Pixel&            out) const;

    //! Same as sample(Sampler*) but with overrides for st & derivatives currently in context.
    void sample(DD::Image::Sampler*          sampler,
                const Dual2Vec2f&            v,
                const DD::Image::ChannelSet& get,
                DD::Image::Pixel&            out) const;

    //! Slower version using a raw Iop.
    void sample(DD::Image::Iop*              texture,
                const DD::Image::ChannelSet& get,
                DD::Image::Pixel&            out) const;
    //! Same as sample(Iop*) but with overrides for st & derivatives currently in context.
    void sample(DD::Image::Iop*              texture,
                const Dual2Vec2f&            v,
                const DD::Image::ChannelSet& get,
                DD::Image::Pixel&            out) const;


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
    //! Per-ray - for LightShader shadow transmission evaluation - defaults to RGB (no depth!)
    DD::Image::Pixel&  shadowColor() const;
    //! Per-ray - for VolumeShader color evaluation - defaults to RGB
    DD::Image::Pixel&  volumeColor() const;
    //! Per-ray - or surface shader calls to retrieve indirect results - defaults to RGB
    DD::Image::Pixel&  indIllumColor() const;


  public:
    //!
    void printParamData(std::ostream&) const;

  protected:
    //! Member vars left intentionally uninitialized for performance reasons.
    SlrShadingContext() = default;

    friend class SlrThreadContext;
    friend class StxBundle;
    friend class SlrThreadContext;
    friend class RenderEngine;
    uint8_t                   _blendingOrder;           //!< Surface blending order
    uint8_t                   _shadeSides;              //!< Which sides to shade
    float                     _indexOfRefraction;       //!< Index of refraction of the hit surface
    bool                      _textureSamplingEnabled;  //!< Whether shaders should sample their textures
    DD::Image::TextureFilter* _textureFilter;           //!< Global texture filter to use. May be null!
    friend DDImage_API std::ostream& operator << (std::ostream&, const SlrShadingContext&);
  };

  using SlrShadingContextList = std::vector<SlrShadingContext>;


  //-----------------------------------------------------------------------------


  class DDImage_API StxBundle
  {
  public:
    //! Default ctor makes an invalid, empty bundle.
    StxBundle();

    //! Is the bundle bound to valid data?
    bool     isValid() const { return (_data != nullptr); }

    //! Bundle index in global StxBundle list.
    int32_t  index() const { return _index; }
    //! Original sample count, usually the ray count. size() should always be <= this number.
    int16_t  nSamples() const { return _samples; }

    int16_t  blendingOrder() const { return _order; }
    bool  blendingOrderFrontToBack() const { return (_order == BlendingOrder::FrontToBack); }
    bool  blendingOrderBackToFront() const { return (_order == BlendingOrder::BackToFront); }
    bool  blendingOrderDisabled()    const { return (_order == BlendingOrder::Disabled); }

    //! Set blending order of the bundle and all the shading contexts currently in the bundle.
    //! Shading contexts added afterwards inherit the order of the bundle.
    void  setBlendingOrder(BlendingOrder order);
    void  setBlendingOrderFrontToBack() { setBlendingOrder(BlendingOrder::FrontToBack); }
    void  setBlendingOrderBackToFront() { setBlendingOrder(BlendingOrder::BackToFront); }
    void  setBlendingOrderDisabled() { setBlendingOrder(BlendingOrder::Disabled); }

    //! Starting SlrShadingContext index in global list.
    int32_t  startIndex() const { return _start; }
    //! Inclusive-end SlrShadingContext index in global list. (ie i=startIndex(); i < endIndex())
    int32_t  endIndex() const { return (_start + _size); }
    //! Total number of ShadingContexts in bundle.
    int32_t  size() const { return _size; }

    SlrShadingContext&        operator [] (int i);
    const SlrShadingContext&  operator [] (int i) const;

    //! Returns the first shading context in the bundle. Caution - if this bundle is empty the returned
    //! shading context may be from a previous bundle!
    SlrShadingContext&  front();
    //! Returns the last shading context in the bundle. Caution - if this bundle is empty the returned
    //! shading context may be from a previous bundle!
    SlrShadingContext&  back();

    //! Iterator support
    struct DDImage_API iterator
    {
      iterator(SlrShadingData* p) noexcept : _ptr(p) {}

      SlrShadingContext&  operator * () noexcept;

      bool operator == (const iterator& b) const noexcept { return (_ptr == b._ptr); }
      bool operator != (const iterator& b) const noexcept { return (_ptr != b._ptr); }

      iterator& operator++() noexcept;
      iterator  operator++(int) noexcept;

    private:
      SlrShadingData* _ptr;
    };

    using const_iterator = iterator;

    iterator        begin();
    const_iterator  begin() const;
    iterator        end();
    const_iterator  end() const;

    //! Sort the shading contexts from near to far distance where the first in the list is nearest.
    void sort();

    //! Sort the shading contexts from far to near distance where the first in the list is farthest.
    void sortFarToNear();

    //! Sets size of bundle to 0 and resizes list to start of bundle (retaining contents of prior bundles.)
    void clear();

    friend DDImage_API std::ostream& operator << (std::ostream& o, const StxBundle& b);


  private:
    friend class SlrThreadContext;
    friend class RayData;
    class RayData* _data; //!< Per-ray intersection data
    int32_t   _index;     //!< Index of this bundle in parent bundle list
    int16_t   _samples;   //!< Original sample count, _size should always be <= this number.
    int16_t   _order;     //!< Bundle blending order (when bundle represents a single ray intersections)
    int32_t   _start;     //!< Absolute starting index in shading data cache list
    int32_t   _size;      //!< Number of ShadingContexts in bundle

    StxBundle(class RayData*, int32_t bundleIndex, int32_t nSamples);
  };

  using StxBundleList = std::vector<StxBundle>;


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
