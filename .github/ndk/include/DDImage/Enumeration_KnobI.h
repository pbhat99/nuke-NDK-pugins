// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef ENUMERATION_KNOBI_H
#define ENUMERATION_KNOBI_H

#include <vector>
#include <string>

namespace DD
{
  namespace Image
  {

    /**
     * Abstract base class for Enumeration Knob-specific methods.
     */
    class Enumeration_KnobI
    {

    public:
      /** set the menu items on the knob to the given list.  if you want this
         to persist you must use the SAVE_MENU flag on this knob. */
      virtual void menu(const std::vector<std::string>& e, bool maintainSelection = true) = 0;

      /** get the list of menu items on the knob */
      virtual const std::vector<std::string>& menu() const = 0;

      /** get the index of the currently selected item */
      virtual int getSelectedItemIndex() const = 0;

      /** get the currently selected menu item */
      virtual std::string getSelectedItemString() const = 0;

      /** The selected menu item can contain tab-separated content to signify a different display string
          to the internal menu item option. This will return the internal menu item option only.
      */
      std::string getItemValueString() const;

      /** The selected menu item can contain tab-separated content to signify a different display string
          to the internal menu item option. This will return the internal menu item option only.
      */
      virtual std::string getItemValueString(int itemIndex) const = 0;

      /** The following function can be used to check the error state on Enumeration knobs.
          If we have the EXACT_MATCH_ONLY flag set, any attempt to set an enumeration knob by
          name to an invalid value will result in the knob being in an Error state.
          This function can be used to get the error value (i.e. the value that was attempted to set.
          Returns NULL if no error.
        */
      virtual const char* getError() const { return nullptr;}

      virtual ~Enumeration_KnobI()
      {
      }

      /** Set whether the menu item at the given index should be usable or greyed out. */
      virtual void setEnabled(int itemIndex, bool enabled) {};
    };


    inline std::string Enumeration_KnobI::getItemValueString() const { return getItemValueString(getSelectedItemIndex()); }

  }
}

#endif
