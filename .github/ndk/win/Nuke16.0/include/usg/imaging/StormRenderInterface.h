// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef USG_IMAGING_STORMRENDERINTERFACE_H
#define USG_IMAGING_STORMRENDERINTERFACE_H

#include "fdk/base/Hash.h"
#include "usg/imaging/RenderInterface.h"

#include "fdk/base/Time.h"

namespace usg
{
  using StageRef = std::shared_ptr<class Stage>;


  //!
  class USG_API StormRenderInterface : public RenderInterface
  {
  public:
    class GLContext; // Abstract, castable to PXR_NS::GlfGLContext
    using GLContextSharedPtr = std::shared_ptr<class GLContext>;


    StormRenderInterface();

    //!
    //void  prepareRender(const DelegateParams& params);

    //!
    void  executeRenderer();

    //!
    void  finalizeRender();


    //----------------------------------------------------------

    //! Convert a DataFormat enumeration to the appropriate GL data format values.
    static void  getGLBufferFormat(const DataFormat& inFormat,
                                   uint32_t*         outGLFormat,
                                   uint32_t*         outGLType,
                                   uint32_t*         outGLInternalFormat = nullptr);

    //!
    static GLContextSharedPtr  getCurrentGLContext();
    //!
    static GLContextSharedPtr  getSharedGLContext();
    //!
    static void                makeCurrentGLContext(const GLContextSharedPtr& glcontext);


  protected:
    //! Starts up the Hydra GL instance. Call this before using the GLContext.
    virtual void  _initHydraGL();

    //!
    bool  _initRenderer(const Token& rendererId) override;

    //! Temp!
    void  _configureScene(const StageRef stage, fdk::TimeValue frame);

    //! Enables or disables the viewer camera light
    void _enableCameraLight(bool enable);

    //! Sets the viewer camera light position
    void _setCameraLightPosition(const fdk::Vec3f &pos);

    //! Notifies Hydra that the textures need to be reloaded
    void _reloadTextures(const std::vector<std::string>& textures);

  protected:
    friend class StormRenderInterfaceHandle;
    // Render delegate params:
    bool       _flipFrontFacing;              //!<
    bool       _applyRenderState;             //!<
    bool       _enableIdRender;               //!<
    bool       _enableSampleAlphaToCoverage;  //!<
    bool       _cameraLightEnabled;           //!<
    fdk::Vec3f _cameraLightPosition;          //!<
    uint64_t   _sampleCount;                  //!<
    int32_t    _drawMode;                     //!<
    fdk::Hash  _lastStageProcessHash;         //!< Previous process hash of the stage to be rendered.
  };


} // usg

#endif
