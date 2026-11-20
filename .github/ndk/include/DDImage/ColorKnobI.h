// Copyright (c) 2018 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DD_Image_ColorKnobI_H
#define DD_Image_ColorKnobI_H

#include "DDImage/DDImage_API.h"

namespace DD {
  namespace Image {

    /*! The interface for color knobs. Both, three and four component color knobs implement it. */
    class DDImage_API ColorKnobI
    {
    public:
      virtual ~ColorKnobI();

      /*! set the default value of panel dropped state */
      virtual void setPanelDefaultDroppedState(bool v) = 0;
    };

  }
}

#endif /* DD_Image_ColorKnobI_H */
