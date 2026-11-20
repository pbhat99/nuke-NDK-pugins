// Menu.h
// Copyright (c) 2013 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_Menu_h
#define DDImage_Menu_h

#include "DDImage/DDImage_API.h"

#include "DDImage/MenuItemData.h" // TODO : forward declare

namespace DD {
  namespace Image {

    /*  Array of these describes a popup menu that can be made to appear by
       calling ViewerContext::menu(). */

    /*! \class DD::Image::Menu

        Describes a pop-up menu. Currently this can only be used in a
        ViewerContext::menu() call to make it pop up and select an item, which
        then modifies the ViewerContext::event().
     */
    class DDImage_API Menu
    {
    public:
      enum Style {
        NO_STYLE,
        ITALIC,
        OBLIQUE,
      };
      int deflt;
      void* implementation;
      Menu() { implementation = nullptr;
               deflt = 0; }

      /*! Add a new item to the menu. The text may contain '/' characters
          to indicate a submenu hierarchy. The \a shortcut is an 'or' of
          ViewerContext::state() bit flags and ViewerContext::key() values to indicate
          what shift keys need to be held down and what key is a shortcut.
          Use 0 for no shortcut. The \a event is an integer that is passed
          to a Knob handle callback if the user picks an item off the menu.
       */
      void add(const char* text, int shortcut, int event);

      /*! Add a horizontal dividing line to the menu. */
      void add_divider();

      /*! \fn void Menu::set_default(int event)
          The item returning \a event is made the default. When the user pops
          up the menu the cursor will be pointing at this item. Calling
          ViewerContext::clear_is_click() will make a mouse click pick that item
          rather than just popping up the menu and waiting for another click.
       */
      void set_default(int event) { deflt = event; }

      /*! The item returning \a event is enabled so the user can pick it. */
      void enable(int event);

      /*! The item returning \a event is disabled so the user cannot pick it
          and it is grayed out. */
      void disable(int event);

      /*! The item returning \a event is disabled so the user cannot pick it
          and is made completely invisible, as though it was not there. To
          get it back call enable() or disable().
       */
      void hide(int event);

      /*! Same as enable() */
      void show(int event);

      /*! The item returning \a event has the given opaque pointer set to some 
       * additional data. */
      void set_data(int event, MenuItemData *data);

      /*! The item returning \a event might have opaque data, this is returned.
       * If no data was assigned using set_data(), it will return NULL. */
      const MenuItemData* get_data(int event);

      /*! The item returning \a event can have a style, for instance you might want
       * the item to appear oblique or italic. This sets the given item to that style.
       */
      void set_style(int event, enum Style style);
    };

  } // namespace Image
} // namespace DD

#endif // DDImage_Menu_h

// Copyright (c) 2013 The Foundry Visionmongers Ltd.  All Rights Reserved.
