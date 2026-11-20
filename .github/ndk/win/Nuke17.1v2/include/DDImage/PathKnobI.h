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

      //! Check if an error is set
      virtual bool inError() const = 0;

      //! Check if a warning is set
      virtual bool inWarning() const = 0;

      //! Get the error message
      virtual std::string errorMessage() const = 0;

      //! Show a error message in the knob
      virtual void showError(const std::string& message) = 0;

      //! Show a warning message in the knob
      virtual void showWarning(const std::string& message) = 0;

      //! Clear the error and warning messages
      virtual void clearErrorsAndWarnings() = 0;

      enum class InputStageMode { NodeInput = 0, File, None };
      virtual void setInputStageMode(InputStageMode mode) = 0;

      //! Set the node input number to retrieve the stage from.
      //! Only used in InputStageMode::NodeInput mode.
      virtual void setInputStageIndex(int index) = 0;

      //! Set the name of the file path knob to retrieve the stage from.
      //! Only used in InputStageMode::File mode.
      //! Default is 'file'.
      virtual void setInputStageFileKnobName(const std::string knobName) = 0;

      //! Sets the filter string used in the popup dialog to filter the prims.
      //! For example 'filter:"Type:Camera"'.
      virtual void setScenegraphFilter(const std::string& filter) = 0;
    };
  }
}

#endif
