// Copyright (c) 2018 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DD_Image_ScriptKnobI_H
#define DD_Image_ScriptKnobI_H

#include "DDImage/DDImage_API.h"

namespace DD {
  namespace Image {

    class DDImage_API ScriptKnobI
    {
    public:

      enum KeyboardModifiers
      {
        NoModifier           = 0,
        ShiftModifier        = 1 << 1,
        ControlModifier      = 1 << 2,
        AltModifier          = 1 << 3
      };

      virtual int getKeyboardModifiers() const = 0;

      virtual void setFixedWidth(int width) = 0;
      virtual void setFixedHeight(int height) = 0;
      virtual void forceWidth(bool v) = 0;
      virtual void forceHeight(bool v) = 0;

      virtual const char* command() const = 0;
      virtual void command(const char* v) = 0;

      virtual ~ScriptKnobI();
    };
  }
}

#endif  // DD_Image_ScriptKnobI_H
