// Copyright (c) 2024 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef SLR_RENDERENGINECONTEXT_H
#define SLR_RENDERENGINECONTEXT_H

#include "DDImage/SlrShadingContext.h"

#include "DDImage/Iop.h"
#include "usg/geom/Stage.h"

namespace DD {
  namespace Image {
    class Sampler;
  }
}
namespace ndk {
  class Channel;
  class ChannelManager;
}

namespace slr
{
  class SlrPrimInfo;
  class SlrSurfaceMaterial;
  class SlrLightInfo;
  class SlrLightMaterial;


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
    //! Pass in parent Op that owns the context
    SlrEngineContext(DD::Image::Iop* iop);

    //! Returns the Render Op that owns this context.
    DD::Image::Iop*  renderIop() const { return _renderIop; }

    /*! Checks if parent renderOp has abort flag was set.
        Use this in loops to check if the user has changed the GUI and
        the render should be stopped.
    */
    bool  renderAborted() const;

    //! Version number incremented each time scene is re-rendered (re-validated)
    int32_t  renderVersion() const { return _renderVersion; }

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

    //----------------------------------------------------------------------------
    // Global scene info:

    //! World-space values are transformed into common shading space by this matrix,
    //! which is typically just a global translate.
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
    const fdk::Mat4d&  globalSceneXform() const { return _globalSceneXform; }

    //! Same as _globalSceneXform but single-precision for ray offset convenience.
    const fdk::Vec3f&  globalSceneOffset() const { return _globalSceneOffset; }


    //----------------------------------------------------------------------------
    // Primvars derived from shader requests and primvars provided by gPrims:


    /*! Retrieves the -first- Channel index assigned to the named primvar layer
        in the shading channel manager. The shader should have declared its desire
        to sample GPrim primvars via SlrShader::requestedGeometryPrimvars().

        Note that \a primvarName is the name of the primitive source attribute
        -WITHOUT- the leading 'primvars:' namespace. For example 'displayColor', not
        'primvars:displayColor' which is the full name of the source attribute.

        An SlrShader should call this from _requestResources() method.

        The channel count is required to make sure the primvar layer name matches the
        expected channel and corresponding data type size. ie 2 channels for a Vec2 or
        double, 3 channels for a Vec3, and 4 channels for a Vec4.

        Returns ndk::Chan_Empty if the retrieval fails, which can happen if no GPrims
        provide the named primvar or the matched layer does not have the required
        channel count. Shaders should be prepared to handle this failure case by
        falling back to default values.

        Some primvars are automatically mapped from the original source primvar name
        to a predefined layer & channel. For example 'displayColor' is mapped to layer
        'rgb' and Chan_R, while 'displayOpacity' is mapped to layer 'alpha' and
        Chan_A.
    */
    virtual ndk::Channel  retrievePrimvarLayer(const usg::Token& primvarName,
                                               int32_t           nChans) const = 0;
    //! Same as retrievePrimvarLayer but returns a Dual2Chans struct with the
    //! dx/dy differential channels assigned, if they exist, otherwise they are set
    //! to ndk::Chan_Empty.
    virtual Dual2Chans    retrieveDual2PrimvarLayer(const usg::Token& primvarName,
                                                    int32_t           nChans) const = 0;

    /*! Retrieves the -first- Channel index assigned to the named writable shading layer
        in the shading channel manager. The shader should have declared its desire
        to write into these layers via SlrShader::requestedWritableShadingLayers().

        An SlrShader should call this from _requestResources() method.

        The channel count is required to make sure the layer name matches the expected
        channel and corresponding data type size. ie 2 channels for a Vec2 or double,
        3 channels for a Vec3, and 4 channels for a Vec4.

        Returns ndk::Chan_Empty if the retrieval fails, which can happen if the shading
        layer name was not requested or the matched layer does not have the required
        channel count. Shaders should be prepared to handle this failure case by
        falling back to default values.
    */
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

    /*! Request a texture DD::Image::Sampler for an Iop and a set of channels.
        An SlrShader should call this from _requestResources() method.
        The DD::Image::Sampler will be created after all shaders have done their
        requests so that the combined channels are handled by the same Sampler.
    */
    virtual void  requestTextureSampler(DD::Image::Iop*              iop,
                                        const DD::Image::ChannelSet& requested_channels) = 0;

    //! Returns a texture DD::Image::Sampler, if one exists, for the Iop source.
    virtual DD::Image::Sampler*  getTextureSampler(DD::Image::Iop* iop) const = 0;


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


  protected:
    virtual ~SlrEngineContext() {}
    DD::Image::Iop*      _renderIop;              //!< Render Iop that owns this context
    int32_t              _renderVersion;          //!< Incremented each time scene is re-rendered (re-validated)
    int32_t              _requestVersion;         //!< Incremented each time renderer is requested
    RenderProjectionMode _renderProjectionMode;   //!< Special rasterization modes - may ignore camera lens
    fdk::Mat4d           _globalSceneXform;       //!< World is transformed by this matrix
    fdk::Vec3f           _globalSceneOffset;      //!< Same as _globalSceneXform but single-precision for ray offset

  public:
    //! ChannelManager for shading system. For internal use only.
    virtual ndk::ChannelManager*  shadingChannelManager() = 0;

  };

} // namespace slr


#endif
