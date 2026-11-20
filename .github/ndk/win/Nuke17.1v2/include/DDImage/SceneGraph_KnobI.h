// Copyright (c) 2020 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef SCENE_GRAPH_KNOBI_H
#define SCENE_GRAPH_KNOBI_H

#include <vector>
#include <map>
#include <string>

#include "SceneItem.h"

namespace DD
{
  namespace Image
  {
    class Menu;
    class Knob;

    static const char* const kSceneGraphKnobName = "scene_graph";
    static const char* kSceneGraphDefaultNodeList[] = {
      "-------          ",
      nullptr
    };

    namespace SceneGraph {

      static const char* const kNameField = "name";
      static const char* const kTypeField = "type";

      static const std::vector<std::string> kSerialisedFields;
    }

    /**
     * Abstract base class for Scenegraph Knob-specific methods.
     */
    class SceneGraph_KnobI
    {

    public:
      /*! augments the menu items on the knob with the given list.
       * \param e collection of scene graph items to insert into the tree
       */
      virtual void addItems(const SceneItems& e, bool selected=false) = 0;

      /*! sets the items in the tree, removing any that no longer exist
       * \param e collection of scene graph items to insert into the tree
       * \param selected set this to true to overwrite the value of already existing menu items
       */
      virtual void setItems(const SceneItems& e, bool selected=false) = 0;

      /** removes from the knob all menu items contained in the given list.
          if you want this to persist you must use the SAVE_MENU flag on this knob. */
      virtual void removeItems(const std::vector<std::string>& e) = 0;

      /// remove all items in the view
      virtual void clear() = 0;

      /** get the menu items names set to the knob. */
      virtual std::vector<std::string> getItemNames() const = 0;

      /** get the menu items set to the knob. */
      virtual SceneItems getItems() const = 0;
      /// get a specific item by path
      virtual SceneItem getItem(const std::string& path) const = 0;

      /// set the selected items
      virtual void setSelectedItems(const std::vector<std::string>& items) = 0;
      /// get the selected items
      virtual std::vector<std::string> getSelectedItems(const std::string& field = DD::Image::SceneGraph::kNameField) const = 0;

      /// true if something is selected in the knob
      virtual bool hasSelection() const = 0;

      /*! Toggle viewing filtered items
      * \param view true to unfilter items listed
      */
      virtual void viewAllNodes(bool view)  = 0;

      /** set the title of the nodelist column **/
      virtual void setColumnHeader(const std::string& title) = 0;

      /** set the view of the scene graph **/
      virtual void enableListView() = 0;

      /** set the focus to the scene graph knob so tab and knob are selected **/
      virtual void setFocus() = 0;

      /** returns whether the given scenegraph is empty **/
      virtual bool isEmpty() const = 0;

      virtual ~SceneGraph_KnobI()
      {
      }
    };

  }
}

#endif
