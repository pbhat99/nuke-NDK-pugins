// Copyright (c) 2019 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DD_Image_PythonCustomKnob_H
#define DD_Image_PythonCustomKnob_H

#include "DDImage/DDImage_API.h"

#include <string>
#include <vector>

namespace DD {
  namespace Image {

    class DDImage_API PythonCustomKnobI
    {
    public:
      PythonCustomKnobI();
      virtual ~PythonCustomKnobI();

      virtual bool hasGui() const = 0;
    };

  }
}

#endif
