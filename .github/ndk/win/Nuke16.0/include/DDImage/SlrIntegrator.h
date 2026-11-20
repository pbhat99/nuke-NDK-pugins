// Copyright (c) 2023 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef SLR_INTEGRATOR_H
#define SLR_INTEGRATOR_H

#include "DDImage/Pixel.h"
#include "DDImage/Knob.h"
#include "DDImage/Knobs.h"
#include "DDImage/PluginObject.h"
#include "DDImage/SlrSurfaceShader.h" // convenience for SlrIntegrator plugins
#include "DDImage/SlrRayBundle.h"
#include "DDImage/SlrAovLayer.h"

#include "ndk/geo/camera/LensProjection.h"

#include "fdk/math/types.h" // for Vec2fList

#include <unordered_map>

namespace DD {
  namespace Image {
    class Op;
  }
}
namespace ndk {
  class DeepPixel;
}

namespace slr
{
  class SlrThreadContext;
  class SlrShadingContext;
  class SlrIntersectionInfo;

  using RenderableWeightMap = std::unordered_map<uint32_t, float>;

  //-----------------------------------------------------------------------------


  /*! This class is passed to an SlrIntegrator from the renderer pixel loop for each pixel
      being rendered. It contains all the camera rays being shot for a pixel so the integrator
      could perform spatial weighting since each ray origin is at a known relative subpixel
      location inside the pixel.
  */
  class DDImage_API SlrIntegratorContext
  {
  public:
    const fdk::Vec2i&  pixelXY() const { return _pixelXY; }
    int32_t            pixelX() const { return _pixelXY.x; }
    int32_t            pixelY() const { return _pixelXY.y; }

    bool  isPixel(int32_t x, int32_t y) const { return (x == _pixelXY.x && y == _pixelXY.y); }
    bool  isPixelX(int32_t x) const { return (x == _pixelXY.x); }
    bool  isPixelY(int32_t y) const { return (y == _pixelXY.y); }

    bool  isDebugPixel() const;
    bool  isDebugSample() const;

    const DD::Image::Format&  renderFormat() const;
    const DD::Image::Box&     renderRegion() const;

    //! Special projection modes. Default SceneCamera is standard lens projection.
    SlrEngineContext::RenderProjectionMode  renderProjectionMode() const;

    //----------------------------------------------------------

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

    //! Deep option - ignore all channels of samples where alpha is 0.
    //! Integrators can respect this global option to remove deep samples that do not
    //! significantly contribute to a deep pixel.
    //! Note that deep matte samples will likely not have zero alpha and so should not
    //! be affected by this option (ie the cutout alpha value is non-zero.)
    bool  deepIgnoreZeroAlphaSamples() const;

    //----------------------------------------------------------

    //! Camera projection function renderer is using.
    const ndk::LensProjectionFunction&  cameraLensFunc() const;

    //! List of per motion sample camera contexts.
    const ndk::CameraSampleList&  cameraMotionSamples() const;

    //! Per motion sample camera context. Note that \a motionIndex is clamped to the
    //! last motion sample.
    //! Note that camera transforms are in shading-space so the camera translation
    //! from the frame0 motion sample has been removed. For example if the camera
    //! translation is [0 0 10] then the sample will be at [0 0 0] and the Z=+10
    //! offset will be in globalSceneXform().
    const ndk::Camera::Sample&  cameraMotionSample(size_t motionIndex) const;

    //! Screen coordinate transformation info.
    const ndk::ProjectionImageContext&  cameraImageContext() const;

    //!
    uint16_t  rayMaxDepth() const;
    uint16_t  rayDiffuseMaxDepth() const;
    uint16_t  rayReflectionMaxDepth() const;
    uint16_t  rayRefractionMaxDepth() const;

    //----------------------------------------------------------

    //! Random number seed for current pixel.
    int32_t  pixelSeed() const { return _pixelSeed; }

    //! Per-subpixel sampling random number generator.
    ndk::SamplingRng&      subpixelRng(int32_t i) const;

    //!
    const fdk::Vec2fList&  subpixelXYs() const { return _subpixelXYs; }
    //! Returns the subpixel XY offset coordinate inside pixelXY where 0,0 is the center of the
    //! pixel, -1,-1 is the pixel left/bottom edge, and +1,+1 is the pixel right/top edge.
    const fdk::Vec2f&      subpixelXY(int32_t i) const { return _subpixelXYs[i]; }

    //!
    int32_t                primaryRayBundleIndex() const { return _primaryRayBundle; }

    //! Number of primary/camera ray samples.
    int32_t                nPrimaryRays() const { return _nPrimaryRays; }
    //! Same as nPrimaryRays.
    int32_t                nSamples() const { return nPrimaryRays(); }

    //! Per primary ray sample info including time, motion step index and step offset t.
    const SampleInfoList&  sampleInfoList() const { return _primaryRaySampleInfoList; }
    const SampleInfo&      sampleInfo(size_t i) const { return _primaryRaySampleInfoList[i]; }

    //! Time value for a primary ray sample.
    fdk::TimeValue         time(size_t i) const { return _primaryRaySampleInfoList[i].time; }

    //!
    const fdk::FloatList&  presenceThresholds() const { return _presenceThresholds; }
    float                  presenceThreshold(size_t i) const { return _presenceThresholds[i]; }

    //! Convenience map for tracking pixel coverage weight of renderables during integration of
    //! subpixel samples together. Typically the \a renderableIndex() from a SlrShadingContext
    //! is used as the key for the map, but other indices could be used.
    //! Picking the 'dominant' or 'best' surface sample for a data Aov output is usually a
    //! matter of finding the nearest renderable surface with the greatest coverage regardless
    //! if it is actually the closest surface. For example if the closest surface has 0.1
    //! coverage but the next closest surface has 0.9 coverage, the 0.9 one wins out since
    //! it is the 'dominant' surface in the pixel.
    RenderableWeightMap&   renderablesCoverageMap() { return _renderablesCoverageMap; }

    //----------------------------------------------------------

    //! Background for final integrated result.
    //! The output pixel of the integrator is expected to incorporate the background into its
    //! final result.
    //! This is provided as a separate value as an integrator may need this value first in
    //! the integration vs. compositing the integrated result at the end.
    const DD::Image::Pixel&  bgPixel() const { return _bgPixel; }

    //! Final integrated & flattened result.
    DD::Image::Pixel&  outputPixel() const;

    //! Are we outputting deep pixel data?
    bool             deepOutputEnabled() const;

    //! Final integrated deep result. Only valid when deep output is enabled.
    ndk::DeepPixel*  outputDeepPixel() const;

    //! Are there active Aov outputs?
    bool  haveAovOutputs() const;

    //! Motion vector output has been requested. This is only true if the render controls
    //! have a valid output channel routing for the motion vector data, to save the cost
    //! of calculating them.
    bool  motionVectorsOutputRequested() const;

    //! List of shading data Aovs to output. These Aovs are copied directly from
    //! SlrShadingContext shading data and are faster to access.
    const SlrAovLayerList&  aovShadingLayersList() const;

    //! List of custom-handler Aovs to output. These Aovs have a custom SlrAovLayer::Handler
    //! that must be called individually and is thus slower than iterating across the
    //! aovShadingLayers list.
    const SlrAovLayerList&  aovCustomLayersList() const;

    //! The map of Aov names to SlrAovLayer indices.
    const SlrAovIndexMap&   aovLayerMap() const;

    //! Find an AovLayer by name.
    const SlrAovLayer*      findAovLayer(const char* name) const;

    //----------------------------------------------------------

    //! True if there's at least one active ray bundle.
    bool  haveActiveRayBundle() const;

    //! True if there's at least one active ray bundle and an active ray in that bundle.
    bool  haveActiveRayData() const;

    //! Start a new RayContext bundle of camera rays making it the active bundle and returning
    //! a reference to it.
    //! If \a nRaysPresize is > 0 then the bundle is pre-sized to that number of rays.
    SlrRayBundle&  startNewCameraRayBundle(ndk::RayContext::TypeMask rayType,
                                           uint32_t                  nRaysPresize = 0) const;

    //! Make \a rayBundle be the active one that intersection testing is done on and store
    //! in. If the bundle has rays in it the active ray will be set to the first in the
    //! bundle.
    //! This is not required if startNewRayBundle() was just called.
    void  selectActiveRayBundle(const SlrRayBundle& rayBundle) const;

    //! Access the currently active RayContext bundle which can be added to. Note that only
    //! one bundle may be active at a time.
    SlrRayBundle&  activeRayBundle() const;

    //! Index of active bundle. 0 is the default bundle that always exists.
    int32_t  activeRayBundleIndex() const;

    //! End ray bundle, releasing all the ray, intersection and shading info created for that
    //! bundle and changing the current bundle to the previous one in the list. If this is the
    //! first bundle nothing happens as there always needs to be one default bundle.
    //! Returns the new current ray bundle.
    SlrRayBundle&  endRayBundle() const;

    //----------------------------------------------------------

    //! Intersect a bundle of rays with the closest surfaces to the ray origins, filling in
    //! shading contexts. There will only be one shading context created per ray, so the size
    //! of the StxBundle will never exceed nRays. If the rays are a coherent set the hits can
    //! be described as a 'wavefront' pushed out from the ray origins along the ray directions.
    //!
    //! Building surface shading contexts requires evaluating additional surface properties so
    //! is more costly than getting the raw geometric hit info.
    //!
    //! Note that \a stxBundle is -not- cleared before adding hits to it.
    //!
    //! Returns the number of hits added to stxBundle.
    int32_t  getNearestSurfaceIntersections(const SlrShadingContext& srcStx,
                                            const ndk::RayContext*   rtxArray,
                                            int32_t                  nRays,
                                            StxBundle&               stxBundle) const;

    //! Intersect a bundle of rays with the closest surfaces to the ray origins, filling in
    //! geometric hit infos. There will only be one hit info created per ray, so the size
    //! of the hit list will never exceed nRays. If the rays are a coherent set the hits can
    //! be described as a 'wavefront' pushed out from the ray origins along the ray directions.
    //!
    //! Note - \a hitList -is- cleared before adding the hits.
    void  getNearestIntersections(const SlrShadingContext& srcStx,
                                  const ndk::RayContext*   rtxArray,
                                  int32_t                  nRays,
                                  ndk::RayHitPointfList&   hitList) const;

    //! Intersect a single RayContext \a Rtx with the closest surface with presence to \a Rtx's
    //! origin within its distance range, and filling in \a hitPoint with the information about
    //! the hit object.
    //!
    //! This is a convenience method and may not be as efficient as using
    //! getNearestSurfaceIntersections() since finding the closest surface with presence means
    //! checking all other surfaces along the ray path, so the cost of getting surface
    //! information for all the surfaces is marginally more than just getting one.
    //!
    //! Returns false if there was no intersection.
    bool  getNearestIntersection(const SlrShadingContext& srcStx,
                                 const ndk::RayContext&   Rtx,
                                 ndk::RayHitPointf&       hitPoint) const;

    //----------------------------------------------------------

    //! Intersect a single ray with all surfaces, filling in the ray hit points list in this
    //! SlrIntegratorContext. Retrieve the hit points via hit point list methods like getRayHitPoint()
    //! and nRayHitPoints().
    //!
    //! This is the most expensive intersection method as all intersectable surfaces are retrieved,
    //! even potentially hidden ones, and building surface shading contexts requires evaluating
    //! additional surface properties.
    //!
    //! Note that the active surface intersection bundle is -not- cleared before adding hits to it.
    void  getAllIntersections(const SlrShadingContext& srcStx,
                              const ndk::RayContext&   Rtx) const;

    //! Intersect a single ray with the all surfaces, filling in geometric hit infos.
    //! Note - \a hitList is -not- cleared before adding the hits.
    void  getAllIntersections(const SlrShadingContext& srcStx,
                              const ndk::RayContext&   Rtx,
                              ndk::RayHitPointfList&   hitList) const;

    //----------------------------------------------------------

    //! Find the 'thin' transmission factor for all surfaces between Rtx.minT and Rtx.maxT,
    //! which is most commonly used for direct lighting occlusion (shadow) testing.
    //! Only materials with the thinInterfaceTransmission() trait enabled are fully evaluated
    //! for opacity (transmission amount,) otherwise only the material's presence is considered
    //! to determine the solidity factor. If the material's presence is derived from its opacity
    //! then opacity will be calculated as a byproduct but will not contribute to the material's
    //! occlusion factor unless thinInterfaceTransmission() is enabled.
    //!
    //! Returns false on trivial 0% transmission, or a 100% occluded path.
    //!
    //! \a transmissionFactor is a Vec3 so that colorized shadows can be supported. It will contain
    //! the total occlusion from all surfaces and volumes along the ray path up to 100%
    //! occlusion or Rtx.maxT, whichever comes first.
    //!
    //! Transmission factor is only valid between 0..1, or 0%..100% transmission, so usually this
    //! factor is a weight multiplied against a light's emission to determine the total amount of
    //! light reaching srcStx.
    //!
    //! Note that thin transmission is an efficiency to speed up direct shadow calculations
    //! without needing to calculate expensive object qualities like scatter, refraction, etc.
    //! During thin transmission path evalation the ray is never bent so a single set of
    //! intersections between ray minT and maxT is all that is ever considered.
    bool  getThinTransmissionFactor(const SlrShadingContext& srcStx,
                                    const ndk::RayContext&   Rtx,
                                    fdk::Vec3f&              transmissionFactor) const;

    //! Perform shading, etc to calculate the final output result for all provided scene rays.
    void  getPathIllumination(const SlrShadingContext&     srcStx,
                              const SlrRayBundle&          rays,
                              const DD::Image::ChannelSet& get,
                              DD::Image::Pixel&            out);

    //! Perform transmission shading on the surface, assuming \a entryStx represents the entry
    //! interface pointer of the object. If the object is marked as double-sided then the entry
    //! interface is assumed to also be the exit interface. In this case there's no possibility
    //! of object overlap resolution via dielectric priority so the surface is interpreted as
    //! warping surface with no thickness.
    //! If single-sided the surface is assumed to have a back surface that terminates the
    //! interior of the surface.
    void  getPathTransmission(const SlrShadingContext&     surfaceStx,
                              const SlrRayBundle&          entryRays,
                              const DD::Image::ChannelSet& get,
                              DD::Image::Pixel&            out);

    //----------------------------------------------------------

    //! Intersect 2d UV points with surface UV coords, filling in geometric hit infos.
    //! Size of uvArray must match size of \a rays bundle - no check is performed!
    //! There will only be one hit info created per ray/UV, so the size of the hit list will never
    //! exceed the size of the rays bundle.
    //!
    //! Note that the active surface intersection bundle is -not- cleared before adding hits to it.
    void  getUVIntersections(const SlrShadingContext& srcStx,
                             const SlrRayBundle&      rays,
                             const fdk::Vec2f*        uvArray) const;

    //----------------------------------------------------------

    //! Retrieve the hit point count from the last ray cast done by this IntegratorContext.
    int32_t  nRayHitPoints() const;

    //! Retrieve a hit point from the last ray cast from 0..nRayHitPoints(). No range checking is
    //! done - the index is assumed to be within nRayHitPoints().
    const ndk::RayHitPointf&  getRayHitPoint(int32_t rayHitIndex = 0) const;

    //! Sort the hits in desired direction. Ray hit index 0 is either nearest or farthest after sorting.
    void  sortRayHits(bool nearToFar = true) const;

    //! Ray hit index 0 is nearest after sorting.
    void  sortRayHitsNearToFar() const { sortRayHits(true); }

    //! Ray hit index 0 is farthest after sorting.
    void  sortRayHitsFarToNear() const { sortRayHits(false); }

    //! OpacityTraitsMask of all the hit points OR'd together.
    ndk::OpacityTraitsMask  rayHitPointsOpacityOrMask() const;

    //! OpacityTraitsMask of all the hit points AND'd together.
    ndk::OpacityTraitsMask  rayHitPointsOpacityAndMask() const;

    //! All hits are opaque-solid, that is we cannot see through the first surface and if
    //! desired can skip all the other hits.
    bool  areAllHitsOpaqueSolid() const;

    //! If false then only the first hit matters since we should never see through it. If true
    //! then all the hits need to be blended, and if doAllHitsBlendToOpaque() is also true then
    //! blending can be done front-to-back.
    bool  doAllHitsBlend() const;
    //! If true all the hits do 'standard' blending that can result in 1.0 opacity. If false
    //! then at least one of the hits needs to do 'non-standard' blending (like min, max, plus)
    //! and blending should be done back-to-front.
    bool  doAllHitsBlendToOpaque() const;

    //----------------------------------------------------------
    // Ray bundle surface intersection results management

    //! Clear the active surface intersection bundle collection in preparation for creating
    //! a series of new surface intersections. Usually at the spawn of a new ray bundle to
    //! store the resulting intersections from each ray in the bundle.
    //!
    //! Same as calling activeSurfaceIntersectionBundleGroup().clear().
    void  startActiveSurfaceIntersectionBundleGroup() const;

    //! Add the currently selected surface intersection bundle to the active surface intersection
    //! bundle collection.
    //! Do this -after- calling a ray intersection method to store the results of each ray bundle.
    void  addSurfaceIntersectionsToActiveGroup() const;

    //! Current active surface intersection bundle collection that can be used by an integrator
    //! to build up a list of StxBundles generated by startNewSurfaceIntersectionBundle().
    //! 
    //! Note - there is only one of these for each RayBundle.
    StxBundleList&  activeSurfaceIntersectionBundleGroup() const;

    //---------------------

    //! Start a new surface intersection bundle stored in the currently active ray data cache.
    //! Use this when doing ad-hoc intersection testing with temp RayContexts or when building
    //! a list of bundles where all the bundles need to remain valid.
    StxBundle&  startNewSurfaceIntersectionBundle() const;

    //! Access the currently selected surface intersection bundle.
    //! Change the selected bundle via SlrRayBundle::selectActiveRay(), startNewSurfaceIntersectionBundle(),
    //! and endShadingIntersectionBundle().
    //!
    //! Note - this will return an invalid, junk StxBundle if selectActiveRayBundle() has never
    //! been called and if used may cause a crash.
    StxBundle&  activeSurfaceIntersectionBundle() const;

    //! Removes the last added bundle from the active list. Will never remove the first bundle
    //! in the active list.
    void  endSurfaceIntersectionBundle() const;

    //! Set blending order of the active surface intersection bundle. Do this before adding
    //! shading contexts as they inherit the bundle's blending order. Alternatively you can set
    //! it on the activeSurfaceIntersectionBundle() after all shading contexts have been added.
    void  setSurfaceIntersectionBundleBlendingOrder(BlendingOrder mode) const;
    void  setSurfaceIntersectionBundleBlendingOrderFrontToBack() const;
    void  setSurfaceIntersectionBundleBlendingOrderBackToFront() const;
    void  setSurfaceIntersectionBundleBlendingOrderDisabled() const;

    //! Construct and add a new shading context in the current StxBundle from the RayHitPoint for
    //! rayHitIndex. This assumes you've started a new bundle and are incrementing its current size.
    //! The new shading context will inherit the blending order of the current bundle.
    //! No range checking is done on rayHitIndex!
    //! srcStx is the shading context from the last hit, or the camera.
    SlrShadingContext&  addNewShadingContextToSurfaceIntersectionBundle(const SlrShadingContext& srcStx,
                                                                        int32_t                  rayIndex = 0) const;


    //------------------------------------------------------------------------
    // Thread-safe scratch Pixels to be used during ray-tracing & shading.
    //
    // Warning - since these are shared variables they should only be used by an integrator
    // at the start of a shader chain, for example when calling a surface shader or
    // accumulating. If passed up a shader graph there's a good chance of conflict with a
    // shader attempting to use the same var.

    //! Per-ray - for surface shader calls - defaults to RGBA.
    //! Warning - this will be invalid memory if there's no active ray data!
    DD::Image::Pixel&  activeRaySurfaceColor() const;

    //! Per-ray - for layering multiple surfaces along the same depth segment - defaults to RGBA.
    //! Warning - this will be invalid memory if there's no active ray data!
    DD::Image::Pixel&  activeRayLayeredColor() const;

    //------------------------------

    //! Thread-context color accumulator Pixel - for accumulating/blending samples together - defaults to RGBA.
    //! This is a convenience for Integrators to use.
    DD::Image::Pixel&  accumColor() const;

    //! Thread-context data Aov accumulator Pixel, usually selected from the nearest sample with largest coverage.
    //! Defaults to Chan_Z.
    //! This is a convenience for Integrators to use.
    DD::Image::Pixel&  accumData() const;

    //!
    ndk::DeepPixel&  accumDeepPixel() const;

    //------------------------------

    //! Global set of all channels that need alpha premult.
    const DD::Image::ChannelSet&  shaderChannelSet() const;
    //! Global set of all channels surface shaders produce.
    const DD::Image::ChannelSet&  surfaceChannelSet() const;
    //! Global set of data channels that don't want blending with alpha (vectors, Z, normals, etc)
    const DD::Image::ChannelSet&  dataChannelSet() const;

    //! Global set of requested channels that Slr2 needs to produce.
    const DD::Image::ChannelSet&  requestedOutChannelSet() const;
    //! Global set of requested channels that need to be premultiplied for alpha compositing.
    const DD::Image::ChannelSet&  requestedOutColorChannelSet() const;
    //! Global set of requested Aov data channels that don't need to be premulted/composited.
    const DD::Image::ChannelSet&  requestedOutDataChannelSet() const;

    // ChannelPack versions:
    const DD::Image::ChannelPack&  shaderChannelPack() const;
    const DD::Image::ChannelPack&  surfaceChannelPack() const;
    const DD::Image::ChannelPack&  dataChannelPack() const;
    const DD::Image::ChannelPack&  bgChannelPack() const;
    const DD::Image::ChannelPack&  requestedOutChannelPack() const;
    const DD::Image::ChannelPack&  requestedOutColorChannelPack() const;
    const DD::Image::ChannelPack&  requestedOutDataChannelPack() const;

    //----------------------------------------------------------

    const SlrAovChannelMap&  aovUnpremultByCoverageChannelMap() const;
    const SlrAovChannelMap&  aovUnpremultByAlphaChannelMap() const;


  protected:
    friend class SlrThreadContext;
    friend class RenderEngine;
    SlrThreadContext*   _sttx;
    fdk::Vec2i          _pixelXY;                   //!< Current output pixel screen coord
    //
    int32_t             _pixelSeed;                 //!< Assigned random seed for current output pixel
    int32_t             _primaryRayBundle;          //!< Index of the bundle containing all the camera rays for current output pixel
    int32_t             _nPrimaryRays;              //!< Number of primary rays in bundle
    fdk::Vec2fList      _subpixelXYs;               //!< All the randomized subpixel uv coords for current output pixel, one per primary ray
    SampleInfoList      _primaryRaySampleInfoList;  //!< All the randomized sample times and motion step info, one per primary ray
    fdk::FloatList      _presenceThresholds;        //!< All the randomized presence thresholds, one per primary ray
    //
    RenderableWeightMap _renderablesCoverageMap;    //!< To track pixel coverage weight of renderables during subpixel sample integration
    DD::Image::Pixel    _bgPixel;                   //!< Background pixel channel data for current output pixel

    SlrIntegratorContext(SlrThreadContext*);
  };


  //-----------------------------------------------------------------------------


  /*! \class DD::Image::SlrIntegrator
  */
  class DDImage_API SlrIntegrator : public DD::Image::PluginObject
  {
  private:
    //! Disabled copy constructor.
    SlrIntegrator(const SlrIntegrator&) = delete;
    //! Disabled assignment operator.
    SlrIntegrator& operator = (const SlrIntegrator&) = delete;


  public:
    static constexpr std::string_view pluginCategory = "SlrIntegrators";  //!< Category can be used to organize plugins within a subdirectory.
    static constexpr std::string_view pluginPrefix   = "slr";             //!< The name to use as the plugin name prefix.
    static constexpr std::string_view pluginSuffix   = "Integrator";      //!< The name to use as the plugin name suffix.

    //-----------------------------------------------------------------
    /*! This structure creates a subclass of SlrIntegrator, possibly from loading a plugin file
        or a compiled-in Description.
    */
    class DDImage_API Description : public DD::Image::PluginObject::Description
    {
    public:
      //! Constructor method definitions used for 'build()' methods in plugins.
      typedef SlrIntegrator* (*IntegratorBuilder)(void);
      IntegratorBuilder builderFunc;  //!< Call this to construct an SlrIntegrator object.

    public:
      //! SlrIntegrator specific builder.
      Description(const char*       integratorClassName,
                  IntegratorBuilder integratorBuilder);

      //! Prefix and suffix strings prepended and appended to pluginClass() string to
      //! construct plugin filenames.
      const std::string_view&  classPrefix() const override { return SlrIntegrator::pluginPrefix; }
      const std::string_view&  classSuffix() const override { return SlrIntegrator::pluginSuffix; }

      //! Name of the SlrIntegrator class without any leading prefix or or trailing suffix.
      const char*  integratorClass() const { return pluginClass().data(); }

      //! Full name of plugin with prefix and suffix.
      const std::string&  integratorPluginName() const { return _pluginName; }
    };
    //-----------------------------------------------------------------


  protected:
    //! Default implementation combines the channels from all inputs.
    virtual void  _validateIntegrator(double time, int32_t nPixelSamples);


  public:
    //!
    SlrIntegrator();

    //! Must have a virtual destructor
    virtual ~SlrIntegrator() {}

    //! Returns the class name, must implement.
    virtual const char*  integratorClass() const = 0;

    //! Create a SlrIntegrator instance from a integrator class name. Calling code takes ownership of returned pointer.
    static SlrIntegrator*  create(const SlrIntegrator::Description& integratorDescription);
    static SlrIntegrator*  create(const std::string_view& integratorClassName);

    //! Find plugins matching this class' prefix and suffix. See PluginObject::FindPlugins() for more info.
    static const fdk::StringList& FindPlugins(bool findBuiltins,
                                              bool findExternals,
                                              bool forceRefresh = false);

  public:

    //!
    virtual void knobs(DD::Image::Knob_Callback f) {}

    //! Perform any custom knob changed callback logic.
    //! \a callAgain is usually passed from the Op::knob_changed() method that owns
    //! this plugin, setting to 1 if the knob wants to be handled again.
    //! Default implementation does nothing and returns \a callAgain unchanged.
    virtual int knob_changed(DD::Image::Knob* knob, int callAgain = 0) { return callAgain; }


    //!
    //! Calls subclass _validateIntegrator().
    void validateIntegrator(double time, int32_t nPixelSamples);


    //! Generate an output Pixel result given a list of source camera rays via the \a Itx
    //! SlrIntegratorContext's outputPixel().
    //! These are the final output floats copied into the renderer Iop's output channels in its
    //! engine (or deep engine) method.
    //!
    //! The default implmentation of this method takes a list of camera rays and does intersection
    //! testing with each, creating a set of hit intersection SlrShadingContexts(Stxs) from the
    //! -nearest- intersections. That's passed to the SlrIntegrator subclass' \a integrate() method
    //! which will typically perform shading and combine the results together to form the final
    //! output Pixel result.
    //!
    //! An integrator subclass can replace this default behavior to create custom calculations
    //! from the camera rays, or completely ignore them altogether. An example of this would
    //! be placing debug information in the output Pixel floats.
    //!
    //! Note that \a camStx is an -empty- Stx provided by the renderer and intended only for
    //! spawning the hit SlrShadingContexts. It does not contain any valid surface information
    //! since it represents the camera's origin and not a surface. However \a camStx could contain
    //! a valid atmosphereShader representing the volume that the camera origin is inside and the
    //! current Ior refractive index.
    //! \a camRtxBundle is the list of RayContexts generated from the camera.
    virtual void  integrateRays(SlrIntegratorContext&    Itx,
                                const SlrShadingContext& camStx,
                                const SlrRayBundle&      camRtxBundle) const;

    //! Default implementation creates pseudo-camera rays matching the count in \a camRtxBundle
    //! then calls integrate() on this Integrator.
    virtual void  integrateRaysUvUnwrap(SlrIntegratorContext&    Itx,
                                        const SlrShadingContext& camStx,
                                        const SlrRayBundle&      camRtxBundle,
                                        const fdk::Vec2f&        tileOffset = fdk::Vec2f(0.0f),
                                        const fdk::Vec2f&        tileScale = fdk::Vec2f(1.0f)) const;


    //! Perform shading, etc to calculate the final output Pixel result.
    //! An integrator will typically perform shading on each hit intersection then combine the results
    //! together to form the final output Pixel result.
    //!
    //! \a samplesStxBundleGroup contains StxBundles where each one contains a list of hit intersection
    //! SlrShadingContexts(Stxs) produced by the integrateRays() method. Most often bundles will only
    //! contain a single intersection Stx from the nearest object hit
    //!
    //! Must implement.
    virtual void  integrate(SlrIntegratorContext&         Itx,
                            const StxBundleList&          samplesStxBundleGroup,
                            const DD::Image::ChannelPack& getColorChannels,
                            const DD::Image::ChannelPack& getDataChannels,
                            DD::Image::Pixel&             out,
                            ndk::DeepPixel*               deepOut = nullptr) const = 0;



    //!
    void print(const char*   prefix,
               std::ostream& o) const;
    friend std::ostream& operator << (std::ostream& o, const SlrIntegrator& b) { b.print("", o); return o; }


  }; // SlrIntegrator

  using SlrIntegratorList = std::vector<SlrIntegrator*>;
  using SlrIntegratorMap  = std::unordered_map<std::string, SlrIntegrator*>;


} // namespace slr


#endif
