
// Copyright (c) 2019 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DD_Image_LinkKnobI_H
#define DD_Image_LinkKnobI_H

#include "DDImage/DDImage_API.h"

namespace DD {
  namespace Image {

    //! This is for internal use only.
    //! Used by the LinkKnob, knob that represents a soft link to another knob.
    //! For LinkKnobs, data storage and presentation is retrieved from the linked knob.
    class DDImage_API LinkKnobI
    {
    public:
      virtual ~LinkKnobI() = 0;
    };

  }
}

#endif
