// Copyright (c) 2011 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef SCENE_VIEW_KNOBI_H
#define SCENE_VIEW_KNOBI_H

#include <vector>
#include <string>

namespace DD
{
  namespace Image
  {
    class Menu;
    class Knob;

    static const char* const kSceneViewKnobName = "scene_view";
    static const char* kDefaultNodeList[] = {
      "-------          ",
      nullptr
    };

    enum class SceneNodeType: unsigned int {
      eInvalid,
      eGeometry,
      eCamera,
      eAxis
    };

    /**
     * Abstract base class for SceneView Knob-specific methods.
     */
    class SceneView_KnobI
    {

    public:

      enum CallbackReason
      {
        eCustomMenuOptionSelected = 0,
        eItemNameChanged,
        eItemMoved,
        eItemAdded,
        eItemRemoved
      };

      enum SelectionMode
      {
        eSelectionModeToggle = 0,   // default. Selections are toggled on and off, with an extra selection column for togglin selections.
        eSelectionModeHighlight     // Selection is made by highlighting a particular row. 
      };

      typedef std::vector<std::string>  ItemList;
      typedef std::vector<unsigned int> IndexList;
      typedef const std::vector<int>    WidgetEventCallbackParam;

      /** callback type passed from an op
          returns with the callback reason, the knob the callback was invoked for, plus two custom int and a const char* parameters. 

          @Param CallbackReason - enumerated value giving the reason for the callback invocation
          @param Knob - The knob invoking the callback
          @param WidgetEventCallbackParam - a vector of integer values giving context information for the callback.
                                            The values in this parameter are dependent upon the CallbackReason:
                                            eCustomMenuOptionSelected - The first element will be the index of the custom menu item selected.
                                            eItemNameChanged - The first element will be the index of the item that has changed
                                            eItemMoved - The first element will be the index that the selection is being moved to,
                                                         The 2nd - n+1th elements are the indices of the n elements being moved
                                                         (n can be obtained by the vector size()-1)
          @param const char* - a string giving context informtaion for the callback (can be NULL)
                                            The values in this parameter are dependent upon the CallbackReason:
                                            eCustomMenuOptionSelected - NULL
                                            eItemNameChanged - The new name of the item
                                            eItemMoved - NULL
       */
      typedef void (*WidgetEventCallback)(CallbackReason, Knob*, WidgetEventCallbackParam&, const char*);

      /** set the menu items on the knob to the given list.  if you want this
         to persist you must use the SAVE_MENU flag on this knob. */
      virtual void menu(const ItemList& e) = 0;

      /** augments the menu items on the knob with the given list.  if you want this
         to persist you must use the SAVE_MENU flag on this knob. */
      virtual void addItems(const ItemList& e) = 0;

      /** removes from the knob all menu items contained in the given list.
          if you want this to persist you must use the SAVE_MENU flag on this knob. */
      virtual void removeItems(const ItemList& e) = 0;

      /** get the menu items names set to the knob. */
      virtual const ItemList& getItemNames() const = 0;

      /** get the number of items in the knob */
      virtual size_t getItemCount() const = 0;

      /** get the list of menu items on the knob */
      virtual const ItemList& menu() const = 0;

      /** get the list of selected items on the knob */
      virtual void getSelectedItems(IndexList& items) = 0;

      /** set the list of selected items on the knob. Note that unless you are in eSelectionModeToggle mode, only the first item will be selected */
      virtual void setSelectedItems(const IndexList& items) = 0;

      /** set list of items imported into this knob **/
      virtual void setImportedItems(const IndexList& items) = 0;

      /** get list of items imported into this knob **/
      virtual void getImportedItems(IndexList& items) = 0;

      virtual void viewAllNodes(bool view)  = 0;

      /** whether to automatically select all imported items */
      virtual void autoSelectItems(bool autoSelect) = 0;

      /** automatically expand the scene view to the first multi-child object, 
      if it exists **/ 
      virtual void autoExpand() = 0;

      /** set the title of the nodelist column **/
      virtual void setColumnHeader(const std::string& title) = 0;

      /** get the item currently being highlighted **/
      virtual std::string getHighlightedItem() const = 0;

      /** return a vector of all item strings which are currently highlighted **/
      virtual void getHighlightedItems(ItemList& itemList) const = 0;

      /** return a vector of all item indices which are currently highlighted **/
      virtual void getHighlightedItems(IndexList& itemList) const = 0;

      /** register a callback to get notifications about changes to the knob **/
      virtual void registerWidgetEventCallback(WidgetEventCallback callback, Knob* knob) = 0;

      /** add a custom right-click menu to knob (Must be a QMenu-derived object) **/
      virtual void addCustomRightClickMenu(DD::Image::Menu* menu) = 0;

      /** sets the knob's selection mode **/
      virtual void setSelectionMode(SelectionMode mode) = 0;

      /** gets the knob's selection mode **/
      virtual SelectionMode getSelectionMode() const = 0;

      /** set drag and drop behavior on or off **/
      virtual void setDragAndDrop(bool enable) = 0;

      /** set editing behavior on or off **/
      virtual void setEditable(bool enable) = 0;

      virtual ~SceneView_KnobI()
      {
      }
    };

  }
}

#endif
