// Copyright (c) 2025 The Foundry Visionmongers Ltd. All Rights Reserved.

#pragma once

#include "usg/imaging/RenderInterface.h"

#include "fdk/base/Hash.h"
#include "fdk/base/Time.h"

#include "usg/katana/Geolib3SceneIndexWrapper.h"

namespace usg {
  using StageRef = std::shared_ptr<class Stage>;

  //!
  class USG_API KatanaRenderInterface : public RenderInterface
  {
  public:
    class GLContext; // Abstract, castable to PXR_NS::GlfGLContext
    using GLContextSharedPtr = std::shared_ptr<class GLContext>;

    static constexpr char kViewerInstanceNamePrefix[] = "KatanaHydra2Viewer";
    static constexpr char kTsubaInstanceNamePrefix[] = "KatanaTsuba";

    KatanaRenderInterface();

    //!
    void executeRenderer();

    //----------------------------------------------------------

    //! Convert a DataFormat enumeration to the appropriate GL data format values.
    static void getGLBufferFormat(const DataFormat& inFormat, uint32_t* outGLFormat,
                                  uint32_t* outGLType, uint32_t* outGLInternalFormat = nullptr);

    //!
    static GLContextSharedPtr getCurrentGLContext();
    //!
    static GLContextSharedPtr getSharedGLContext();
    //!
    static void makeCurrentGLContext(const GLContextSharedPtr& glcontext);

    //! Some initial setup involving task controller
    void configureScene();

    //! Initialize USD imaging scene indices include a stage scene index.
    void initImagingSceneIndices(const Path& path);

    //! Set stage to the stage scene index from imaging scene indices.
    void setStage(const StageRef usgStage);

    //! Set frame time of a stage set to the stage scene index from imaging scene indices.
    void setFrameTime(fdk::TimeValue frame);

    //! This is a temp solution to respond to stage updates from a Katana viewer.
    //! Will be replaced by a scene index or a scene index filter.
    void applyStagePendingUpdates();

    //! Starts up the Hydra GL instance. Call this before using the GLContext.
    virtual void initHydraGL();

    void insertGeolib3SceneIndex(katana::Geolib3SceneIndexWrapper* inputScene,
                                 const Path& scenePathPrefix);

    void updateDrawMode(int drawMode = 0);

    void updateRenderTags();

  protected:
    //!
    bool _initRenderer(const Token& rendererId) override;

  protected:
    friend class KatanaRenderInterfaceHandle;                  //!<
  };

} // usg
