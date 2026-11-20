// Copyright (c) 2023 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef SLR_INTEGRATOR_H
#define SLR_INTEGRATOR_H

#include "DDImage/Pixel.h"
#include "DDImage/Knob.h"
#include "DDImage/Knobs.h"
#include "DDImage/PluginObject.h"
#include "DDImage/SlrShadingContext.h"
#include "DDImage/SlrSurfaceShader.h" // convenience for SlrIntegrator plugins
#include "DDImage/SlrRayContext.h"
#include "DDImage/SlrAovLayer.h"

#include "ndk/geo/camera/LensProjection.h"

#include "fdk/math/types.h" // for Vec2fList

#include <unordered_map>

namespace DD {
  namespace Image {
    class Op;
  }
}
namespace slr
{

  /*! This class is passed to an SlrIntegrator from the renderer pixel loop for each pixel
      being rendered. It contains all the camera rays being shot for a pixel so the integrator
      could perform spatial weighting since each ray origin is at a known relative subpixel
      location inside the pixel.
  */
  class DDImage_API SlrIntegratorContext
  {
  public:
    //! Thread memory context.
    const SlrThreadContext&  threadContext() const;

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
    const fdk::Mat4d&  worldToShadingXform() const;

    //! Inverse of worldToShadingXform, for going from shading space to world.
    const fdk::Mat4d&  shadingToWorldXform() const;

    //! Same as worldToShadingXform but single-precision for ray offset convenience.
    const fdk::Vec3f&  worldToShadingOffset() const;

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
    //! offset will be in worldToShadingXform().
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

    //----------------------------------------------------------

    //! Are there active Aov outputs? These are extra float channels provided by the
    //! shading contexts to be copied to the output samples.
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

    //! Returns true if anything is occluding the ray, including transparent objects.
    //! Does not call shaders or return information about the hit object.
    //! \a srcStx is required for sample/time info.
    bool  trace(const SlrShadingContext&  srcStx,
                const fdk::Vec3f&         origin,
                const fdk::Vec3f&         dir,
                ndk::RayContext::TypeMask type = ndk::RayContext::shadowRay(),
                float                     minDist = defaultMinDistance(),
                float                     maxDist = defaultMaxDistance()) const;

    //! Returns true if anything is occluding the ray, including transparent objects.
    //! Does not call shaders or return information about the hit object.
    //! \a srcStx is required for sample/time info.
    bool  trace(const SlrShadingContext& srcStx, const SlrRayContext& Rtx) const;

    //! Returns true if anything is occluding the ray, filling in \a hitPoint with
    //! the information about the hit object, including hit distance and opacity.
    //! Calls presence and opacity shaders only.
    bool  trace(const SlrShadingContext& srcStx,
                const SlrRayContext&     Rtx,
                ndk::RayHitPointf&       hitPoint) const;

    //! Intersect a bundle of rays with the closest surfaces to the ray origins, filling in
    //! shading contexts. There will only be one shading context created per ray, so the size
    //! of the output StxBundle will never exceed nRays. If the rays are a coherent set the hits
    //! can be described as a 'wavefront' pushed out from the ray origins along the ray
    //! directions.
    //!
    //! Building surface shading contexts requires evaluating additional surface properties so
    //! is more costly than getting the raw geometric hit info.
    //!
    //! Note that \a stxBundleOut is -not- cleared before adding hits to it.
    //!
    //! Returns the number of hits added to \a stxBundleOut.
    int32_t  getNearestSurfaceIntersections(const SlrShadingContext& srcStx,
                                            const ndk::RayContext*   rtxArray,
                                            int32_t                  nRays,
                                            StxBundle&               stxBundleOut) const;

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

    //----------------------------------------------------------

    //! Intersect a single ray with all surfaces, filling in the ray's hit point/shading context
    //! list retrievable via Rtx.hitPointStack(). The list is always cleared by this
    //! intersection method, so move any hit points you care about out of the list before calling
    //! another intersection method with the same ray.
    //!
    //! Note that even though the SlrRayContext arg is const the ray data bound to the ray is not,
    //! so while the SlrRayContext itself is not modified the bound data is.
    //!
    //! This is the most expensive intersection method as all intersectable surfaces are retrieved,
    //! even potentially hidden ones, and building surface shading contexts requires evaluating
    //! additional surface properties.
    int32_t  getAllIntersections(const SlrShadingContext& srcStx,
                                 const SlrRayContext&     Rtx) const;

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
    //! Returns true on trivial 0% transmission, or a 100% occluded path.
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
                                    const SlrRayContext&     Rtx,
                                    fdk::Vec3f&              transmissionFactor) const;

    //! Perform shading, etc to calculate the final output result for the provided scene ray.
    void  getPathIllumination(const SlrShadingContext&     srcStx,
                              const SlrRayContext**        rays,
                              int32_t                      nRays,
                              const DD::Image::ChannelSet& get,
                              DD::Image::Pixel&            out) const;

    //! Perform transmission shading on the surface, assuming \a entryStx represents the entry
    //! interface pointer of the object. If the object is marked as double-sided then the entry
    //! interface is assumed to also be the exit interface. In this case there's no possibility
    //! of object overlap resolution via dielectric priority so the surface is interpreted as
    //! warping surface with no thickness.
    //! If single-sided the surface is assumed to have a back surface that terminates the
    //! interior of the surface.
    void  getPathTransmission(const SlrShadingContext&     surfaceStx,
                              const SlrRayContext**        entryRays,
                              int32_t                      nRays,
                              const DD::Image::ChannelSet& get,
                              DD::Image::Pixel&            out) const;

    //----------------------------------------------------------

    //! Intersect a 2d UV point with surface UV coords, filling in the ray's hit point/shading context
    //! list retrievable via Rtx.hitPointStack(). \a uvDxy specifies the radius of the filter area
    //! to calculate in UV coordinate space.
    void  getUvIntersections(const SlrShadingContext& srcStx,
                             const SlrRayContext&     Rtx,
                             const fdk::Vec2f&        uvCoord,
                             const fdk::Vec2f&        uvDxy) const;

    //! Utility function to intersect a ray with the BVH bboxes. \a hitPoint will
    //! only contain info about the bbox wall geometry, so opacity, presence, etc
    //! will be at defaults.
    //! Hit point result will be in Rtx.hitPointStack().
    bool  getBboxIntersection(const SlrRayContext& Rtx, int32_t level) const;

    //------------------------------

    //! Global set of all channels surface shaders produce.
    const DD::Image::ChannelSet&  surfaceShadingChannelSet() const;
    //! Global set of all channels that need alpha blending.
    const DD::Image::ChannelSet&  shaderColorChannelSet() const;
    //! Global set of data channels that don't want blending with alpha (vectors, Z, normals, etc)
    const DD::Image::ChannelSet&  shaderDataChannelSet() const;

    //! Global set of requested channels that Slr2 needs to produce.
    const DD::Image::ChannelSet&  requestedOutputChannelSet() const;
    //! Global set of requested channels that need to be premultiplied for alpha blending.
    const DD::Image::ChannelSet&  requestedOutputColorChannelSet() const;
    //! Global set of requested Aov data channels that don't need to be premulted/blended.
    const DD::Image::ChannelSet&  requestedOutputDataChannelSet() const;

    // ChannelPack versions:
    const DD::Image::ChannelPack&  surfaceShadingChannelPack() const;
    const DD::Image::ChannelPack&  shaderColorChannelPack() const;
    const DD::Image::ChannelPack&  shaderDataChannelPack() const;
    const DD::Image::ChannelPack&  bgChannelPack() const;
    const DD::Image::ChannelPack&  requestedOutputChannelPack() const;
    const DD::Image::ChannelPack&  requestedOutputColorChannelPack() const;
    const DD::Image::ChannelPack&  requestedOutputDataChannelPack() const;

    //----------------------------------------------------------

    const SlrAovChannelMap&  aovUnpremultByCoverageChannelMap() const;
    const SlrAovChannelMap&  aovUnpremultByAlphaChannelMap() const;


  protected:
    friend class SlrIntegrator;
    friend class ThreadContext;
    friend class RenderEngine;
    class ThreadContext* _sttx;
    fdk::Vec2i      _pixelXY;                   //!< Current output pixel screen coord
    //
    int32_t         _pixelSeed;                 //!< Assigned random seed for current output pixel
    int32_t         _nPrimaryRays;              //!< Number of primary rays in bundle
    fdk::Vec2fList  _subpixelXYs;               //!< All the randomized subpixel uv coords for current output pixel, one per primary ray
    SampleInfoList  _primaryRaySampleInfoList;  //!< All the randomized sample times and motion step info, one per primary ray
    fdk::FloatList  _presenceThresholds;        //!< All the randomized presence thresholds, one per primary ray

    explicit SlrIntegratorContext(ThreadContext*);
  };


  //-----------------------------------------------------------------------------


  /*! \class DD::Image::SlrIntegrator
  */
  class DDImage_API SlrIntegrator : public DD::Image::PluginObject
  {
  public:
    static constexpr std::string_view pluginCategory = "SlrIntegrators";  //!< Category can be used to organize plugins within a subdirectory.
    static constexpr std::string_view pluginPrefix   = "slr";             //!< The name to use as the plugin name prefix.
    static constexpr std::string_view pluginSuffix   = "Integrator";      //!< The name to use as the plugin name suffix.

    //!
    SlrIntegrator();

    //! Must have a virtual destructor
    virtual ~SlrIntegrator() {}

    //! Returns the class name, must implement.
    virtual const char*  integratorClass() const = 0;

    //!
    virtual void addIntegratorKnobs(DD::Image::Knob_Callback f) {}

    //! Perform any custom knob changed callback logic.
    //! \a callAgain is usually passed from the Op::knob_changed() method that owns
    //! this plugin, setting to 1 if the knob wants to be handled again.
    //! Default implementation does nothing and returns \a callAgain unchanged.
    virtual int integratorKnobChanged(DD::Image::Knob* knob, int callAgain = 0) { return callAgain; }


    //!
    //! Calls subclass _validateIntegrator().
    void validateIntegrator(double time, int32_t nPixelSamples);


    //! Given a list of source camera rays in \a camRtxBundle generate output Pixel samples via
    //! the \a Itx SlrIntegratorContext's \a outputSamples().
    //! These are the final output floats copied into the renderer Iop's output channels in its
    //! engine (or deep engine) method.
    //!
    //! An integrator subclass can replace this default behavior to create custom calculations
    //! from the camera rays or completely ignore them altogether. An example of this would be
    //! placing debug information in the output Pixel floats.
    //!
    //! Note that each ray in \a camRtxBundle has an -empty- source shading context provided by the
    //! renderer and is intended only for spawning the hit SlrShadingContexts. They do not contain
    //! any valid surface information since they represent the camera's origin and not a surface.
    //! However each shading context could contain a valid atmosphereShader representing the volume
    //! that the camera origin is inside, plus other traits like the current Ior refractive index,
    //! and the sample info for each ray to use.
    //!
    //! \a camRtxBundle is the list of SlrRayContexts generated from the camera.
    //!
    //! \a camSampleGroups is storage for the intersected shading results, managed by the
    //! integrator system.
    //!
    //! The default implementation of this method takes a list of camera rays and does intersection
    //! testing with each, adding hit intersection SlrShadingContexts(Stxs) from the -nearest-
    //! intersections to \a camSampleGroups. That is passed to the SlrIntegrator subclass
    //! \a integrate() method which will typically perform shading and possibly combine the results
    //! together to form the final output Pixel samples.
    virtual void  integrateRays(SlrIntegratorContext&   Itx,
                                const SlrRayBundle&     camRtxBundle,
                                StxLayeringStackBundle& camSampleGroups) const;

    //! Perform shading, etc to calculate the final output Pixel result.
    //! An integrator will typically perform shading on each hit intersection then combine the results
    //! together to form the final output Pixel result in \a outputSampleBundles().
    //!
    //! \a camSampleGroups contains StxBundles created in \a integrateRays(). Each bundle is a
    //! list of hit intersection SlrShadingContexts (Stxs) and will most often bundles will only
    //! contain a single intersection Stx from the nearest object hit. A bundle can contain more when
    //! an object provides 'thin transmission' which is a non-physically accurate way to composite
    //! multiple object surfaces along a single ray, ignoring all physical effects like refraction.
    //!
    //! Must implement.
    virtual void  integrate(SlrIntegratorContext&         Itx,
                            const StxLayeringStackBundle& camSampleGroups) const = 0;


    //! Custom UV-unwrapping variant of \a integrateRays() which creates pseudo ray intersections in
    //! UV space.
    //!
    //! Default implementation creates pseudo-camera rays matching the count in \a camRtxBundle,
    //! then calls integrate() on this Integrator.
    virtual void  integrateRaysUvUnwrap(SlrIntegratorContext&   Itx,
                                        const SlrRayBundle&     camRtxBundle,
                                        StxLayeringStackBundle& camSampleGroups,
                                        const fdk::Vec2f&       tileOffset = fdk::Vec2f(0.0f),
                                        const fdk::Vec2f&       tileScale = fdk::Vec2f(1.0f)) const;


    //!
    void print(const char*   prefix,
               std::ostream& o) const;
    friend std::ostream& operator << (std::ostream& o, const SlrIntegrator& b) { b.print("", o); return o; }


  protected:
    //! Default implementation combines the channels from all inputs.
    virtual void  _validateIntegrator(double time, int32_t nPixelSamples);


  public:
    //-----------------------------------------------------------------
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


    //! Create a SlrIntegrator instance from a integrator class name. Calling code takes ownership of returned pointer.
    static SlrIntegrator*  create(const SlrIntegrator::Description& integratorDescription);
    static SlrIntegrator*  create(const std::string_view& integratorClassName);

    //! Find plugins matching this class' prefix and suffix. See PluginObject::FindPlugins() for more info.
    static const fdk::StringList& FindPlugins(bool findBuiltins,
                                              bool findExternals,
                                              bool forceRefresh = false);

    //-----------------------------------------------------------------
    //-----------------------------------------------------------------


  private:
    //! Disabled copy constructor.
    SlrIntegrator(const SlrIntegrator&) = delete;
    //! Disabled assignment operator.
    SlrIntegrator& operator = (const SlrIntegrator&) = delete;

  }; // SlrIntegrator

  using SlrIntegratorList = std::vector<SlrIntegrator*>;
  using SlrIntegratorMap  = std::unordered_map<std::string, SlrIntegrator*>;


} // namespace slr


#endif
