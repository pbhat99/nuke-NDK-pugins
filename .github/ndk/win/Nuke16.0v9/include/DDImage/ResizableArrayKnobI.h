// Copyright (c) 2010 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DD_Image_ResizableArrayKnobI_H
#define DD_Image_ResizableArrayKnobI_H

#include <vector>
#include <string>

#include "DDImage/DDImage_API.h"

namespace DD {
  namespace Image {

    class OutputContext;

    class ResizableArrayKnobI
    {
    public:

      //! Set item as visible in the curve editor
      virtual void addItemToCurveEditor(int index) = 0;
      
      //! Check the item's curve editor visibility
      virtual bool visibleInCurveEditor(int index) const = 0;

      //! Remove item's curve editor visibility
      virtual void removeItemFromCurveEditor(int index) = 0;

      //! Set item as visible in the dope sheet
      virtual void addItemToDopeSheet(int index) = 0;

      //! Check the item's dope sheet visibility
      virtual bool visibleInDopeSheet(int index) const = 0;

      //! Remove item's dope sheet visibility
      virtual void removeItemFromDopeSheet(int index) = 0;

      //! Override an item's curve editor/dope sheet label
      virtual void setItemLabel(int index, const std::string& name) = 0;

      //! Get an item's curve editor/dope sheet label
      virtual std::string getItemLabel(int index) const = 0;

      virtual ~ResizableArrayKnobI()
      {
      }
    };
  }
}

#endif
