// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef DDIMAGE_PATHKNOBI_H
#define DDIMAGE_PATHKNOBI_H

#include "DDImage/DDImage_API.h"

#include <string>

namespace DD
{
  namespace Image
  {
    //! Abstract base class for a single Path knob
    class DDImage_API Path_KnobI
    {
    public:
      virtual ~Path_KnobI() = default;

      //! Show a error message in the knob
      virtual void showError(const std::string& message) = 0;

      enum class InputStageMode { NodeInput = 0, File, None };
      virtual void setInputStageMode(InputStageMode mode) = 0;
      virtual void setInputStageIndex(int index) = 0;
      virtual void setInputStageFileKnobName(const std::string knobName) = 0;

      virtual void setScenegraphFilter(const std::string& filter) = 0;
    };
  }
}

#endif
