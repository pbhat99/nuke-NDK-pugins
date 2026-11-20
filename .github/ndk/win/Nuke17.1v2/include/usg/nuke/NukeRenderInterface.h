// Copyright (c) 2026 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef NUKE_RENDER_INTERFACE_H
#define NUKE_RENDER_INTERFACE_H

#include <optional>

#include "usg/imaging/RenderInterface.h"

namespace usg {
class NukeRenderInterfaceHandle;
class ShaderDesc;
class ShaderDescGroup;
class MaterialSource;

using ShaderDescConstPtrList = std::vector<const ShaderDesc*>;

//! The render interface implementation for Nuke which exposes renderer controls to the application.
class USG_API NukeRenderInterface : public RenderInterface
{
public:
  class GLContext;
  using GLContextSharedPtr = std::shared_ptr<GLContext>;

  static constexpr const char* kViewerInstanceNamePrefix = "NukeHydra2Viewer";

  explicit NukeRenderInterface();

  //! Convert a DataFormat enumeration to the appropriate GL data format values.
  static void getGLBufferFormat(const DataFormat& inFormat, uint32_t* outGLFormat, uint32_t* outGLType, uint32_t* outGLInternalFormat = nullptr);

  //! Get the current GL context.
  static GLContextSharedPtr getCurrentGLContext();

  //! Get the shared GL context.
  static GLContextSharedPtr getSharedGLContext();

  //! Make the given GL context current.
  static void makeCurrentGLContext(const GLContextSharedPtr& glcontext);

  //! Starts up the Hydra GL instance. Call this before using the GLContext.
  virtual void initHydraGL();

  //! Configure the render task parameters (lighting, materials, AOVs, render tags, etc.).
  //! Should be called after the renderer is initialised and before rendering.
  void configureScene() const;

  //! Set the named output layers to render, caching the names so they are
  //! restored by configureScene() after scene index rebuilds.
  void setOutputLayers(const TokenArray& names) const;

  //! Initialise the Hydra 2.0 imaging scene index pipeline at the given root path.
  //! This creates the stage scene index and associated filtering scene indices.
  void initImagingSceneIndices(const Path& path) const;

  //! Bind a USD stage to the stage scene index. The Hydra engine will render the stage's
  //! contents. Call initImagingSceneIndices() first.
  void setStage(const StageRef& usgStage) const;

  //! Set the current frame time on the stage scene index for animation playback.
  void setFrameTime(fdk::TimeValue frame) const;

  //! Apply any pending edits from the USD stage to the scene index pipeline.
  //! Call this after modifying the stage to ensure the render reflects the latest state.
  void applyStagePendingUpdates() const;

  //! Execute the rendering tasks. If the renderer is not initialised this is a no-op.
  //! Uses progressive rendering settings from the base RenderInterface.
  void executeRenderer() const;

  //! Query the render delegate for supported AOV token names.
  //! Returns a list of AOV name strings (e.g. "color", "depth", "normal", etc.)
  //! that the current render delegate can produce.
  std::vector<std::string> getSupportedAovTokens() const;

  //! Set which AOV to display in the viewport. The AOV name should be one of the
  //! strings returned by getSupportedAovTokens(). Passing "color" reverts to the
  //! default beauty display.
  void setDisplayAov(const std::string& aovName);

  //! Reloads the given texture resources and re-dirties all overridden material prims
  void reloadTextures(const fdk::StringSet& textures) const;

  //! Update the default camera light's orientation to match the camera's view direction
  void setCameraLightTransform(const fdk::Mat4f& inverseViewMatrix) const;

  //! Set the active render settings prim path so production render delegates read
  //! settings from the correct USD RenderSettings prim.
  void setActiveRenderSettingsPrimPath(const Path& path) const;

  //! Set the active render product prim path so production render delegates only collect RenderVars
  //! from that product.
  void setActiveRenderProductPrimPath(const Path& path) const;

  //! Set the paths to include in the render collection. This is used to filter which prims are
  //! rendered by the Hydra delegate.
  void setRenderCollectionPaths(std::optional<PathArray> paths) { _renderCollectionPaths = std::move(paths); }

protected:
  friend class NukeRenderInterfaceHandle;

private:
  //! Determines the paths to render in the Hydra delegate.
  std::optional<PathArray> _renderCollectionPaths;
};
}  // namespace usg

#endif  // NUKE_RENDER_INTERFACE_H
