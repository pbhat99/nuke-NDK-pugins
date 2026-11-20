// Copyright (c) 2024 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef DDIMAGE_KNOBMIXINS_H
#define DDIMAGE_KNOBMIXINS_H

#include "DDImage/Knobs.h"

namespace DD
{
  namespace Image
  {
    //! A class which provides common UI for Snapping Geometry/Pivot point
    //! Create one of these and call the knobs method to make the knobs.
    //! The knob_changed function should be called as this ensures that the old
    //! snap menu is hidden on show panel (if it's present).
    class DDImage_API SnapKnobs {
      public:
        enum class SnapOperation
        {
          GeoTo,
          PivotTo
        };

        SnapKnobs(const std::string& knobNamePrefix="");
        void knobs(Knob_Callback f);
        int knob_changed(Knob* k, Op* op);
        
        void enable(bool enable);
      
      private:
        void updateUI();
        void setOperation(SnapOperation operation);

      private:
        std::string _snapOperationKnobName;
        std::string _translateKnobName;
        std::string _rotateKnobName;
        std::string _scaleKnobName;
        std::string _pivotToBoundingBoxKnobName;
        std::string _pivotToSelectionKnobName;
        std::string _geoToSelectionKnobName;
        std::string _pivotToSelectionAniKnobName;
        std::string _geoToSelectionAniKnobName;
        std::string _customKnobName;

        std::string _legacySnapMenuKnobName;
        std::string _legacySnapMenuLblKnobName;
        std::string _axisUseMatrixKnobName;

        int _snapOperation = static_cast<int>(SnapOperation::GeoTo);
        bool _translate = true;
        bool _rotate = false;
        bool _scale = false;

        Knob* _snapOperationKnob = nullptr;
        Knob* _pivotToBoundingBoxKnob = nullptr;
        Knob* _pivotToSelectionKnob = nullptr;
        Knob* _pivotToSelectionAniKnob = nullptr;
        Knob* _geoToSelectionKnob = nullptr;
        Knob* _geoToSelectionAniKnob = nullptr;
        Knob* _scaleKnob = nullptr;
        Knob* _translateKnob = nullptr;
        Knob* _rotateKnob = nullptr;
        Knob* _customKnob = nullptr;

        bool _internallyEnabled = true;
        bool _enabled = true;
    };
  }
}

#endif
