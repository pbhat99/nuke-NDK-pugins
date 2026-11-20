// Copyright (c) 2025 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef USD_SCENE_GRAPH_KNOBI_H
#define USD_SCENE_GRAPH_KNOBI_H

#include "DDImage/Knobs.h"

#include <map>

namespace usg
{
  class Path;
  using PathSet = std::set<Path>;

  class Stage;
  using StageRef = std::shared_ptr<Stage>;

  class Layer;
  using LayerRef = std::shared_ptr<Layer>;
}

namespace DD::Image
{
  using PathMap = std::map<usg::Path, bool>;

  /**
    * Abstract base class for USD Scenegraph Knob-specific methods.
    */
  class USDSceneGraph_KnobI
  {

  public:

    // Set the USD stage to be shown in the 3D scene graph.
    virtual void setStage(const usg::StageRef& stage) = 0;

    // Get the USD stage.
    virtual usg::StageRef getStage() = 0;

    // Set the layer to be used by the scene graph.
    virtual void setScenegraphLayer(const usg::LayerRef& layer) const = 0;

    // Several functions for getting the current state of the scene graph tree.
    virtual const PathMap& getLoadedPaths() const = 0;
    virtual const PathMap& getVisibilityOverridesPaths() const = 0;
    virtual const PathMap& getActiveOverridesPaths() const = 0;
    virtual bool  getPrimVisible(const usg::Prim& prim) const = 0;
    virtual bool  getPrimActive(const usg::Prim& prim) const = 0;

    // Get and clear pending payload loads
    virtual usg::PathSet consumePendingPayloadLoads() = 0;

    // Clear the overrides in the scenegraph.
    virtual void clearOverrides() = 0;

    // Display a popup dialog window with USD scene graph for users to be able to 
    // select payloads and other options to edit the stage before any 3D data gets loaded.
    virtual void showWindowPopup() = 0;

    // Check if the window popup is currently showing.
    virtual bool isWindowPopupOpen() = 0;

    virtual ~USDSceneGraph_KnobI() {};
  };
}

#endif
