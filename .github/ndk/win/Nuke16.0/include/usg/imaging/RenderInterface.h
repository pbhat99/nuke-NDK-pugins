// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef USG_IMAGING_RENDERINTERFACE_H
#define USG_IMAGING_RENDERINTERFACE_H

#include "usg/base/ArgSet.h"
#include "usg/geom/Attribute.h"

#include "fdk/math/Mat4.h"
#include "fdk/math/Box2.h"

#include <memory>

namespace usg
{

  //--------------------------------------------------------------------------------------------------

  class RenderInterfaceHandle;    //!< Wrapper abstraction for implementation type
  class RenderOutputLayerHandle;  //!< Wrapper abstraction for implementation type
  class SourceCodeGenerator;

  using StageRef = std::shared_ptr<class Stage>;

  //!
  class USG_API RenderInterface
  {
  public:
    //-------------------------------------------------------------

    using DataFormat = uint8_t;

    // UNorm8 - unsigned normalized byte representing a float between 0 and 1:
    static DataFormat  FormatUNorm8;
    static DataFormat  FormatUNorm8Vec2;
    static DataFormat  FormatUNorm8Vec3;
    static DataFormat  FormatUNorm8Vec4;
    // SNorm8 - signed normalized byte representing a float between -1 and 1:
    static DataFormat  FormatSNorm8;
    static DataFormat  FormatSNorm8Vec2;
    static DataFormat  FormatSNorm8Vec3;
    static DataFormat  FormatSNorm8Vec4;
    //
    static DataFormat  FormatFloat16;
    static DataFormat  FormatFloat16Vec2;
    static DataFormat  FormatFloat16Vec3;
    static DataFormat  FormatFloat16Vec4;
    //
    static DataFormat  FormatFloat32;
    static DataFormat  FormatFloat32Vec2;
    static DataFormat  FormatFloat32Vec3;
    static DataFormat  FormatFloat32Vec4;
    //
    static DataFormat  FormatInt16;
    static DataFormat  FormatInt16Vec2;
    static DataFormat  FormatInt16Vec3;
    static DataFormat  FormatInt16Vec4;
    //
    static DataFormat  FormatUInt16;
    static DataFormat  FormatUInt16Vec2;
    static DataFormat  FormatUInt16Vec3;
    static DataFormat  FormatUInt16Vec4;
    //
    static DataFormat  FormatInt32;
    static DataFormat  FormatInt32Vec2;
    static DataFormat  FormatInt32Vec3;
    static DataFormat  FormatInt32Vec4;
    //
    static DataFormat  FormatFloat32UInt8;  //!< Depth-stencil format
    //
    static DataFormat  FormatInvalid;
    static DataFormat  FormatCount;

    //-------------------------------------------------------------

    //!
    class USG_API OutputLayer
    {
    public:
      //! Default ctor builds an empty (invalid) OutputLayer.
      OutputLayer();
      //! Copy constructors
      OutputLayer(const OutputLayer&);
      OutputLayer& operator = (const OutputLayer&);
      OutputLayer(const RenderOutputLayerHandle&);

      // Dtor in implementation required for std::unique_ptr to work
      virtual ~OutputLayer();


      //! True if RenderOutputLayerHandle is valid (has a valid object binding.)
      bool      isValid() const;
      operator  bool()    const { return this->isValid(); }


      //! Resolve the buffer so that reads reflect the latest writes.
      void   resolve();

      //! Map(bind) the buffer for reading.
      void*  map();
      //! Unmap(unbind) the buffer. It is no longer safe to read from the buffer.
      void   unmap();

      //-------------------------------------------------------
      // Note - the accessor methods below do not check for a
      //        valid object binding. If called and not valid
      //        they will likely crash. Check isValid() first!
      //-------------------------------------------------------

      //! Return whether the buffer is currently mapped(bound) by anybody.
      bool     isMapped() const;
      //! Is the buffer converged (whether the renderer is still adding samples or not).
      bool     isConverged() const;

      int32_t     width() const;
      int32_t     height() const;
      int32_t     nChans() const;
      DataFormat  dataFormat() const;
      //! Derived from DataFormat.
      int32_t     bytesPerChan() const;
      //! Derived from DataFormat.
      bool        isSignedData() const;
      //! Derived from DataFormat.
      bool        isFloatData() const;


    protected:
      std::unique_ptr<RenderOutputLayerHandle, void(*)(RenderOutputLayerHandle*)> _pImpl;
    };

    //-------------------------------------------------------------

    struct RenderPluginDesc
    {
      Token       id;
      std::string displayName;
      int         priority;
    };

    //! Return the list of all loaded plugin names.
    static const std::vector<RenderPluginDesc>&  rendererPlugins();

    //! Checks whether the renderer plugin exists.
    static bool rendererPluginExists(Token id, RenderPluginDesc* desc = nullptr);

    //-------------------------------------------------------------

  public:
    // Subclasses may want to use their own handle creator by passing false.
    RenderInterface(bool createHandle = true);

    // Dtor in implementation required for std::unique_ptr to work
    virtual ~RenderInterface();

    //! True if RenderInterface is valid (has a valid object binding.)
    bool              isValid() const;
    operator          bool()    const { return this->isValid(); }

    //-------------------------------------------------------------

    //! Identifier token for this renderer interface - should be unique!
    const Token&  rendererId() const { return _rendererId; }

    //-------------------------------------------------------------

    //!
    ArgSet&  rendererArgs() { return _rendererArgs; }

    //! Initialize the renderer interface, copying the provided args.
    //! This method will then call the virtual subclass method _initRenderer().
    void  initRenderer(const ArgSet& args);

    //! Returns true if the render substructures are allocated and initialized.
    //! Note - check this before calling any accessor methods!
    bool  rendererInitialized() const;

    //-------------------------------------------------------------

    /// -------------------------------------------------------
    /// Render Settings API

    struct RenderSettingsDescriptor
    {
      Token key;
      std::string name;
      Value defaultValue;
    };
    using RenderSettingsDescriptorVector = std::vector<RenderSettingsDescriptor>;

    //! Returns the available render settings
    const RenderSettingsDescriptorVector& renderSettings() const { return _rendererSettings; }

    //! Sets the render setting if it exists
    void setRenderSetting(const Token& key, const Value& v);

    //! Returns whether the key identifies a render setting
    bool isRenderSetting(const Token& key) const;

    //! Returns the render setting for key. If it doesn't exist, returns an empty descriptor
    const RenderSettingsDescriptor& getRenderSetting(const Token& key) const;

    //! Sets the refine level for subdiv prims.
    void setRefineLevel(int level) { _refineLevel = level; }

    //! Sets whether the rendering should be done progressively.
    void setProgressiveRendering(bool progressiveRendering);

    //! Enables/disables presentation to the screen.
    void setEnablePresentation(bool enable);

    //! Sets the AOVs which will be rendered to the viewport if presentation is enabled.
    void setPresentationOutput(const Token& name);

    /// -------------------------------------------------------
    /// Lighting API

    //! Enables/disables scene lighting.
    void setEnableLighting(bool enabled) { _enableLighting = enabled; }

    //! Returns whether scene lighting is enabled or disabled.
    bool isLightingEnabled() const { return _enableLighting; }

    /// -------------------------------------------------------
    /// Camera and Framing API
    
    //!
    void setCameraFraming(const fdk::Box2i& displayWindow,
                          const fdk::Box2i& dataWindow,
                          float             pixelAspectRatio = 1.0f);

    //! Set the view and projection matrices for the free camera.
    //! Note: The projection matrix must be pre-adjusted for the window policy.
    void setFreeCameraMatrices(const fdk::Mat4d& viewMatrix,
                               const fdk::Mat4d& projectionMatrix);


    //! Set pixel w/h of the OutputLayers.
    void setOutputLayerSize(int32_t width,
                            int32_t height);

    /// -------------------------------------------------------
    /// Shadow API

    //! Turns the shadow task on or off.
    void setEnableShadows(bool enabled) { _enableShadows = enabled; }

    //! Returns whether the shadow task is on or off.
    bool isShadowsEnabled() const { return _enableShadows; }

    /// -------------------------------------------------------
    /// Material API

    //! Return the MaterialSource generator to use for source type \a sourceType.
    virtual const SourceCodeGenerator*  getSourceCodeGenerator(const Token& sourceType) const;


    //! Enables/disables using scene materials.
    void setEnableMaterials(bool enabled) { _enableSceneMaterials = enabled; }

    //! Returns whether scene materials are enabled/disabled.
    bool isMaterialEnabled() const { return _enableSceneMaterials; }

    void setDisplayRender(const bool displayRender) { _displayRender = displayRender; }
    bool isDisplayRender() const { return _displayRender; }

    void setDisplayProxy(const bool displayProxy) { _displayProxy = displayProxy; }
    bool isDisplayProxy() const { return _displayProxy; }

    void setDisplayGuides(const bool displayGuides) { _displayGuides = displayGuides; }
    bool isDisplayGuides() const { return _displayGuides; }

    void setFlatShading(bool enable) { _flatShading = enable; }
    bool isFlatShading() const { return _flatShading; }

    //-------------------------------------------------------------

    //! Start the engine execution of the currently assigned rendering tasks.
    //! If fullyConverge = true the method will loop until isConverged() = true
    //! before returning.
    void  executeTasks(bool fullyConverge = false);

    //! Is the buffer converged (whether the renderer is still adding samples or not).
    bool  isConverged() const;

    //-------------------------------------------------------------

    //! Set the list of named OutputLayers to be rendered.
    void  setOutputLayers(const TokenArray& names);

    //! Retrieve a named OutputLayer, if it exists. If not the returned OutputLayer object is invalid.
    //! Warning, no internal state checking performed, assumes rendererInitialized() returned true!
    OutputLayer  getOutputLayer(const Token& name) const;

    //! Set the identifier name for the scene
    void setSceneName(const std::string& sceneName) { _sceneName = sceneName; }
    //! Get the identifier name for the scene
    const std::string& getSceneName() const { return _sceneName; }


  protected:
    //! Subclasses can do any custom initializations in this method.
    virtual bool _initRenderer(const Token& rendererId);


  protected:
    friend class RenderInterfaceHandle;
    std::unique_ptr<RenderInterfaceHandle, void(*)(RenderInterfaceHandle*)> _pImpl;
    Token                                  _rendererId;       //!< Renderer identifier string
    ArgSet                                 _rendererArgs;     //!< Arguments to renderer
    std::vector<RenderSettingsDescriptor>  _rendererSettings; //!< Arguments to renderer
    std::string                            _sceneName;        //!< Identifier name for scene

    // Render task params:
    bool       _showViewerGeometry = true;   //!< True if we show the ViewerContext geometry, otherwise show from inputs
    bool       _flatShading = false;         //!< Materials are not lit, they are constant-shaded colors & textures
    bool       _enableLighting = true;       //!<
    bool       _enableSceneMaterials = true; //!<
    bool       _enableShadows = false;       //!<
    bool       _displayRender = true;        //!< True if we are showing render prims
    bool       _displayProxy = false;        //!< True if we are showing proxy prims
    bool       _displayGuides = false;       //!< True if we are showing guide prims
    //
    float      _pointSize = 3.0f;
    fdk::Vec4f _pointColor{1.0f, 0.0f, 0.0f, 1.0f};
    //
    int        _refineLevel = 0;             //!< The refine level for subdiv prims.
  };


} // usg

#endif
