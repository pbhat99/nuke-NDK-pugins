// Copyright (c) 2010 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DD_Image_TextKnobI_H
#define DD_Image_TextKnobI_H

#include <string>

#include "DDImage/DDImage_API.h"

namespace DD {
  namespace Image {

    //! Interface to provide facility for setting the text of a static text knob
    //! To use in Op subclasses. Create a knob using Named_Text_knob() and retrieve
    //! the knob using Op::knob("knob_name"). Then dynamic_cast to a Text_KnobI
    //! and call text("some value") on it.
    class DDImage_API Text_KnobI
    {
    public:
      virtual ~Text_KnobI();

      virtual const std::string& text() const = 0;
      virtual void text(std::string& text) = 0;
    };

  }
}

#endif
