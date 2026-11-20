// Copyright (c) 2024 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef SLR_RENDERENGINECONTEXT_H
#define SLR_RENDERENGINECONTEXT_H

#include "DDImage/SlrShadingContext.h"
#include "DDImage/Iop.h"
#include "DDImage/DeepOp.h"
#include "DDImage/GeometryProviderI.h"

#include "usg/geom/Stage.h"

#include <atomic>

namespace DD {
  namespace Image {
    class Sampler;
    class DeepOutputPlane;
  }
}
namespace ndk {
  class Channel;
  class ChannelManager;
}

namespace slr
{
  class SlrEngineContext;
  class SlrFilterOp;
  struct SlrPrimInfo;
  class SlrSurfaceMaterial;
  struct SlrLightInfo;
  class SlrLightMaterial;
  class SlrIntegrator;
  class SlrShader;

  using SlrFilterOpList = std::vector<SlrFilterOp*>;

  //----------------------------------------------------------------------------------

  /*! \class slr::SlrRenderOp

      Op base class for ScanlineRender nodes.
  */
  class DDImage_API SlrRenderOp : public DD::Image::Iop
                                , public DD::Image::DeepOp
                                , public DD::Image::GeometryProviderI
  {
  public:
    explicit SlrRenderOp(Node*);

    //! Return the node input to use for the render scene source.
    //! -1 indicates an internal scene source with no input connection.
    //! Must implement.
    virtual int32_t  sceneInput() const = 0;

    //------------------------------------------------------------

    //! Retrieve the SlrRenderOp the renderEngineContext() is coming from.
    //! Defaults to 'this'.
    virtual SlrRenderOp*  renderEngineOp() { return this; }

    //! Retrieve the render engine context this SlrRenderOp provides, if any.
    virtual SlrEngineContext*  renderEngineContext() = 0;

    // Number of active render threads engine is currently using.
    virtual uint32_t  activeRenderThreadCount() const { return _activeRenderThreads; }

    //------------------------------------------------------------

    //! Increment the requestVersion and configure any globals.
    virtual void  renderEngineRequestSetup() = 0;

    void  _open() override;
    void  _close() override;

    //------------------------------------------------------------
    // From Iop:
    void  _request(int x, int y, int r, int t, DD::Image::ChannelMask outChannels, int count) override;
    void  engine(int y, int x, int r, DD::Image::ChannelMask outChannels, DD::Image::Row& outRow) override;

    //------------------------------------------------------------
    // From DeepOp:
    Op*  op() override { return this; }
    void  getDeepRequests(DD::Image::Box box, const DD::Image::ChannelSet& outChannels, int count, std::vector<DD::Image::RequestData>& requests) override;
    bool  doDeepEngine(DD::Image::Box box, const DD::Image::ChannelSet& outChannels, DD::Image::DeepOutputPlane& outDeepPlane) override;

    //----------------------------------------------------------------------------
    // From GeometryProviderI:
    Op*  getGeometryProviderOp() override { return this; }

    //! GeometryProviderI: Allow subclasses to gain access to sibling functions.
    GeometryProviderI*  asGeometryProvider() override { return this; }

    //! Cast to a GeometryProviderI interface. This is much cheaper and safer than using dynamic_cast.
          GeometryProviderI*  geometryProvider()       override { return asGeometryProvider(); }
    const GeometryProviderI*  geometryProvider() const override { return const_cast<SlrRenderOp*>(this)->asGeometryProvider(); }

    // Pass to sceneInput() by default.
    fdk::Hash  geometryComposeState(const ndk::NodeEvalContext& engineGraphContext) override;
    fdk::Hash  geometryEditVersionState(const ndk::NodeEvalContext& engineGraphContext) override;
    bool       geometryStateVaries(const ndk::NodeEvalContext& engineGraphContext) override;

    //! Return an empty layer, SlrRenderOp cannot provide one.
    usg::LayerRef  buildGeometryLayerForContext(const ndk::NodeEvalContext& engineGraphContext,
                                                bool                        appendTo,
                                                const fdk::TimeValueSet&    sampleTimes = fdk::TimeValueSet()) override;

    // Pass to sceneInput() by default.
    bool  canProvideGeometryStage() const override;

    // Pass to sceneInput() by default.
    void  buildGeometryStageForContext(usg::StageRef&              stage,
                                       const usg::ArgSet&          requestArgs,
                                       const ndk::NodeEvalContext& engineGraphContext,
                                       const fdk::TimeValueSet&    additionalTimes = fdk::TimeValueSet()) override;


  protected:
    std::atomic<uint32_t> _activeRenderThreads;   //!< Number of engine calls currently being processed
  };


  //----------------------------------------------------------------------------------
  //----------------------------------------------------------------------------------


  class DDImage_API SlrThreadContext
  {
  public:
    SlrThreadContext();
    virtual ~SlrThreadContext();

    //! Thread id.
    DD::Image::ThreadId  threadId() const;

    //! Thread index in a linear list of shading engine threads, guaranteed to be unique and never reused
    //! for another thread Id during the same engine session (between open() and close()).
    //!
    //! Note that a render engine thread works on multiple output pixels & lines during a single engine
    //! session so while \a threadIndex is not reused the thread itself is reused over and over again.
    uint32_t  threadIndex() const;
  };


  //----------------------------------------------------------------------------------
  //----------------------------------------------------------------------------------


  /*! 
  */
  class DDImage_API SlrEngineContext
  {
  public:
    //! Any special projection modes are in this list.
    enum RenderProjectionMode
    {
      SceneCamera = 0,    //!< Standard camera render mode
      UvUnwrap,           //!< UV map unwrap (uv bake)
    };


  public:
    //! Pass in parent SlrRenderOp that owns the context.
    explicit SlrEngineContext(SlrRenderOp* iop);

    //! Returns the SlrRenderOp Iop/DeepOp that owns this context.
    SlrRenderOp*  renderOp() const { return _renderOp; }

    /*! Checks if parent renderOp has abort flag was set.
        Use this in loops to check if the user has changed the GUI and
        the render should be stopped.
    */
    bool  renderAborted() const;

    //! Version number incremented each time scene is re-rendered (re-validated)
    int32_t  renderVersion() const { return _renderVersion; }

    int32_t  validateVersion() const { return _validateVersion; }

    //! Version number incremented each time renderer is requested
    int32_t  requestVersion() const { return _requestVersion; }

    //! Special rasterization modes - may ignore camera lens.
    RenderProjectionMode  renderProjectionMode() const { return _renderProjectionMode; }

    //! Get the Stage containing the scene geometry being rendered.
    virtual usg::StageRef  stage() const = 0;

    //!
    virtual int32_t             nRenderablePrims() const = 0;
    //!
    virtual const SlrPrimInfo&  renderablePrim(int32_t i) const = 0;

    //!
    virtual int32_t             nRenderableLights() const = 0;
    //!
    virtual const SlrLightInfo& renderableLight(int32_t i) const = 0;

    //!
    virtual int32_t             nInstancers() const = 0;
    //!
    virtual const SlrPrimInfo&  instancerPrim(int32_t i) const = 0;


    //! The list of material family names to connect material outputs, in priority order.
    virtual const usg::TokenArray&  materialFamilies() const = 0;

    //! Find the SlrSurfaceMaterial bound to a material path, if any.
    virtual SlrSurfaceMaterial*  getSlrPrimInfoMaterial(const usg::Path& material_path) const = 0;

    //! Assign a SlrSurfaceMaterial to a path. If an existing material exists for the path it's deleted and replaced.
    virtual void  assignSlrPrimInfoMaterial(SlrPrimInfo&        primInfo,
                                            SlrSurfaceMaterial* slr_material) = 0;


    //! Find the SlrLightMaterial bound to a material path, if any.
    virtual SlrLightMaterial*    getSlrLightInfoMaterial(const usg::Path& material_path) const = 0;

    //! Assign a SlrLightMaterial to a path. If an existing material exists for the path it's deleted and replaced.
    virtual void  assignSlrLightInfoMaterial(SlrLightInfo&     lightInfo,
                                             SlrLightMaterial* slr_material) = 0;

    //! Overall lighting-is-on state
    virtual bool  globalLightingEnabled() const = 0;

    //! Overall shadowing-is-on state
    virtual bool  globalShadowingEnabled() const = 0;

    //----------------------------------------------------------------------------
    // Global scene info:

    //! World-space values are transformed into common shading space by this matrix,
    //! which is typically just a global translate by the camera origin.
    //!
    //! Just a translation is fine because there is no need to remove the camera's
    //! rotations from the scene. The main requirement is to move the scene close
    //! to the origin (0,0,0) so that single-precision floats are adequate for
    //! ray tracing precision. The rotations of the camera are immaterial.
    //!
    //! Important note - Shaders that have world-space values passed as inputs
    //! from scene shader nodes need to transform those values by this matrix to
    //! put them into shading-space. Lights and objects are transformed by the
    //! renderer during setup and have this xform already applied.
    virtual const fdk::Mat4d&  worldToShadingXform() const = 0;

    //! Inverse of worldToShadingXform, for going from shading space to world.
    virtual const fdk::Mat4d&  shadingToWorldXform() const = 0;

    //! Like worldToShadingXform but single-precision for ray offset convenience.
    virtual const fdk::Vec3f&  worldToShadingOffset() const = 0;


    //----------------------------------------------------------------------------
    // Primvars derived from shader requests and primvars provided by gPrims:


    //! Retrieves the -first- Channel index assigned to the named primvar layer
    //! in the shading channel manager. The shader should have declared its desire
    //! to sample GPrim primvars via SlrShader::requestedGeometryPrimvars().
    //! 
    //! Note that \a primvarName is the name of the primitive source attribute
    //! -WITHOUT- the leading 'primvars:' namespace. For example 'displayColor', not
    //! 'primvars:displayColor' which is the full name of the source attribute.
    //!
    //! An SlrShader should call this from _requestResources() method.
    //! 
    //! The channel count is required to make sure the primvar layer name matches the
    //! expected channel and corresponding data type size. ie 2 channels for a Vec2 or
    //! double, 3 channels for a Vec3, and 4 channels for a Vec4.
    //! 
    //! Returns ndk::Chan_Empty if the retrieval fails, which can happen if no GPrims
    //! provide the named primvar or the matched layer does not have the required
    //! channel count. Shaders should be prepared to handle this failure case by
    //! falling back to default values.
    //! 
    //! Some primvars are automatically mapped from the original source primvar name
    //! to a predefined layer & channel. For example 'displayColor' is mapped to layer
    //! 'rgb' and Chan_R, while 'displayOpacity' is mapped to layer 'alpha' and
    //! Chan_A.
    virtual ndk::Channel  retrievePrimvarLayer(const usg::Token& primvarName,
                                               int32_t           nChans) const = 0;
    //! Same as retrievePrimvarLayer but returns a Dual2Chans struct with the
    //! dx/dy differential channels assigned, if they exist, otherwise they are set
    //! to ndk::Chan_Empty.
    virtual Dual2Chans    retrieveDual2PrimvarLayer(const usg::Token& primvarName,
                                                    int32_t           nChans) const = 0;

    //! Retrieves the -first- Channel index assigned to the named writable shading layer
    //! in the shading channel manager. The shader should have declared its desire
    //! to write into these layers via SlrShader::requestedWritableShadingLayers().
    //! 
    //! An SlrShader should call this from _requestResources() method.
    //! 
    //! The channel count is required to make sure the layer name matches the expected
    //! channel and corresponding data type size. ie 2 channels for a Vec2 or double,
    //! 3 channels for a Vec3, and 4 channels for a Vec4.
    //! 
    //! Returns ndk::Chan_Empty if the retrieval fails, which can happen if the shading
    //! layer name was not requested or the matched layer does not have the required
    //! channel count. Shaders should be prepared to handle this failure case by
    //! falling back to default values.
    virtual ndk::Channel  retrieveShadingLayer(const usg::Token& layerName,
                                               int32_t           nChans) const = 0;
    //! Same as retrieveShadingLayer but returns a Dual2Chans struct with the
    //! dx/dy differential channels assigned, if they exist, otherwise they are set
    //! to ndk::Chan_Empty.
    virtual Dual2Chans    retrieveDual2ShadingLayer(const usg::Token& layerName,
                                                    int32_t           nChans) const = 0;


    //----------------------------------------------------------------------------

    //! Can the renderable Gprim provide the named primvar with the required number
    //! of channels? If so it returns the primvar's interpolation type, otherwise it
    //! returns Invalid_Rate.
    //!
    //! Note that \a primvarName is the name of the primitive source attribute
    //! -WITHOUT- the leading 'primvars:' namespace. For example 'displayColor', not
    //! 'primvars:displayColor' which is the full name of the source attribute.
    //!
    //! \a nPrimvarChans is required to make sure the primvar layer name matches the
    //! expected channel and corresponding data type size. ie 2 channels for a Vec2 or
    //! double, 3 channels for a Vec3, and 4 channels for a Vec4.
    virtual PrimvarInterpolation  gprimProvidesPrimvar(const SlrPrimInfo& primInfo,
                                                       const usg::Token&  primvarName,
                                                       int32_t            nPrimvarChans) const = 0;


    //----------------------------------------------------------------------------

    //! Request a texture DD::Image::Sampler for an Iop and a set of channels.
    //! An SlrShader should call this from its implemented _requestResources() method,
    //! then during its implemented _initializeForShadingThread() method create
    //! a DD::Image::Sampler by calling createTextureSampler().
    //!
    //! After all shaders have done their requests the resulting texture reference will
    //! contain the combined channels being handled by the same Iop/Sampler source.
    virtual void  requestTextureSampler(DD::Image::Iop*              iop,
                                        const DD::Image::ChannelSet& requestedChannels) = 0;

    //! Create and return a texture DD::Image::Sampler for the Iop source. This is
    //! normally called from an SlrShader's _initializeForShadingThread() method
    //! to create a Sampler for the shading thread, stored in the SlrShaderThreadData
    //! associated with that SlrShader.
    //!
    //! Important - the calling SlrShader must take ownership of the Sampler allocation
    //! and its deletion. See SlrShader::_initializeForShadingThread() for more info.
    virtual DD::Image::Sampler*  createTextureSampler(DD::Image::Iop* iop) const = 0;

    //----------------------------------------------------------------------------

    //! Intersected output color layers of all active surface materials. This will typically
    //! include the light material color outputs as the light AOVs must flow through the
    //! surface materials.
    virtual const DD::Image::ChannelSet&  surfaceMaterialOutputColorChannels() const = 0;

    //! Intersected output 'data' layers of all active surface materials. This will typically
    //! include the light material data outputs as the light AOVs must flow through the
    //! surface materials.
    virtual const DD::Image::ChannelSet&  surfaceMaterialOutputDataChannels() const = 0;

    //! Intersected output color layers of all active light materials.
    virtual const DD::Image::ChannelSet&  lightMaterialOutputColorChannels() const = 0;

    //! Intersected output 'data' layers of all active light materials.
    virtual const DD::Image::ChannelSet&  lightMaterialOutputDataChannels() const = 0;


    //----------------------------------------------------------------------------

    //! List of Aovs to output.
    virtual const SlrAovLayerList&  aovLayerList() const = 0;

    //! List of shading data Aovs to output. These Aovs are copied directly from
    //! SlrShadingContext shading data and are faster to access.
    virtual const SlrAovLayerList&  aovShadingLayersList() const = 0;

    //! List of custom-handler Aovs to output. These Aovs have a custom SlrAovLayer::Handler
    //! that must be called individually and is thus slower than iterating across the
    //! aovShadingLayers list.
    virtual const SlrAovLayerList&  aovCustomLayersList() const = 0;

    //! The map of Aov names to SlrAovLayer indices.
    virtual const SlrAovIndexMap&   aovLayerMap() const = 0;

    //! Find an AovLayer by name.
    virtual const SlrAovLayer*      findAovLayer(const char* name) const = 0;


    //----------------------------------------------------------------------------

    //! Default integrator provided by the renderer.
    virtual SlrIntegrator*  defaultIntegrator() const = 0;

    //! Call this from a rendering Iop::_open() or a SlrFilterOp::_open().
    //! If this is not called then the Render engine may crash.
    virtual void  engineOpen() = 0;

    //! Does this renderer have a background input source? If true \a getBgSource() should be
    //! called to retrieve this source pixel data, otherwise the bg should be assumed to be
    //! empty and filled with whatever values are correct for the output channels.
    virtual bool  hasBgSource() const = 0;

    //! Retrieve the background pixel data for a pixel region. Normally this data comes from
    //! the renderOp()'s background input.
    //!
    //! If \a outRow is valid this method should fill the flat Row for line \a y, or erase it. Note
    //! that any additional lines between \a y and \a t should be skipped.
    //!
    //! If \a outDeep is valid this method should fill the deep xyrt region, or add 'holes'.
    //!
    //! Returns false if processing was halted due to an abort.
    //! Default implementation reads the renderOp()'s input0.
    virtual bool  getBgSource(int y, int t, int x, int r,
                              const DD::Image::ChannelSet& outChannels,
                              DD::Image::Row*              outRow,
                              DD::Image::DeepOutputPlane*  outDeep);

    //! Render/rasterize the rectangular \a renderRegion which is either a single scanline
    //! written to \a outRow or a region written to \a outDeepPlane. Normally only one of
    //! the outputs are non-null depending on the engine called in SlrRenderOp, but the
    //! renderer will produce both if desired.
    //!
    //! Returns false if processing was halted due to an abort.
    virtual bool  renderPixelRegion(SlrIntegrator*               integrator,
                                    const SlrFilterOpList&       filters,
                                    const DD::Image::Box&        renderRegion,
                                    const DD::Image::ChannelSet& outChannels,
                                    DD::Image::Row*              outRow,
                                    DD::Image::DeepOutputPlane*  outDeepPlane) = 0;

    //! Call this from a rendering Iop::_close() or a SlrFilterOp::_close().
    //! If this is not called then the Render engine may not clean up after itself or
    //! re-initialize properly.
    virtual void  engineClose() = 0;


  protected:
    virtual ~SlrEngineContext() {}
    SlrRenderOp*         _renderOp;               //!< Render Iop/DeepOp that owns this context
    int32_t              _validateVersion;        //!< Incremented each time scene is re-validated
    int32_t              _renderVersion;          //!< Incremented each time scene is re-rendered
    int32_t              _requestVersion;         //!< Incremented each time renderer is requested
    RenderProjectionMode _renderProjectionMode;   //!< Special rasterization modes - may ignore camera lens
    size_t               _numShaders;             //!< Total number of SlrShaders registered

  public:
    //! ChannelManager for shading system. For internal use only.
    virtual ndk::ChannelManager*  shadingChannelManager() = 0;

    // Add or remove a shader to/from the renderer. For internal use only.
    virtual int32_t  registerShader(SlrShader* shader) = 0;
    virtual int32_t  unRegisterShader(SlrShader* shader) = 0;
  };


} // namespace slr


#endif
