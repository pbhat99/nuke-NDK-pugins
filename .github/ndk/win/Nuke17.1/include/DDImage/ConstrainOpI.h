// Copyright (c) 2025 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef DDIMAGE_CONSTRAINOPI_H
#define DDIMAGE_CONSTRAINOPI_H

#include "DDImage/DDImage_API.h"
#include "DDImage/Op.h"

#include "fdk/math/Mat4.h"
#include "fdk/base/Time.h"

#include "ndk/geo/utils/Constrain.h"


namespace DD
{
  namespace Image
  {
    class GeomOpEngine;

    //! Implements the constrain behaviour
    class DDImage_API ConstrainOpI
    {
    public:
      ConstrainOpI(fdk::FrameValue referenceFrame, fdk::AxisDirection defaultLookAxis=fdk::AxisDirection::Z_PLUS);
      ConstrainOpI(DD::Image::Op* op, fdk::AxisDirection defaultLookAxis=fdk::AxisDirection::Z_PLUS);
      virtual ~ConstrainOpI() = default;

      void knobs(Knob_Callback f, Op* op);
      int knob_changed(Knob* k, Op* op);

      //! Returns the input label for the current constrain mode
      const char* input_label(char* buffer) const;

      const char* constraintKnobName() const;
      const char* orientAxisKnobName() const;
      const char* rotateXKnobName() const;
      const char* rotateYKnobName() const;
      const char* rotateZKnobName() const;
      const char* strengthKnobName() const;
      const char* applyTranslateKnobName() const;
      const char* applyRotateKnobName() const;
      const char* applyScaleKnobName() const;
      const char* maintainOffsetFrameKnobName() const;
      const char* maintainOffsetKnobName() const;
      const char* setMaintainOffsetFrameToCurrentKnobName() const;

      //! Returns a ConstraintMatrixParams structure populated from the knobs
      //! Does not populate xform variables as these are not obtained from the knobs.
      ndk::ConstraintMatrixParams getConstraintParametersFromKnobs(const OutputContext* context = nullptr) const;

      //! Returns the times we need from inputs given the times that have been requested.
      fdk::TimeValueSet getNeededInputTimes(const fdk::TimeValueSet& requestedTimes) const;

      //! Gets the constraint type from the constraint knob.
      ndk::ConstraintMode getConstraintType() const;

      //! Gets whether we should be maintaining the offset given the constraint type and whether
      //! the maintain offset knob value is True.
      bool constraintMaintainOffset() const;
    
      static fdk::AxisDirection GetLookAxis(int lookAxisIndex);
    
      class DDImage_API SubEngine
      {
      public:
        explicit SubEngine(GeomOpEngine* engine);

      protected:
        GeomOpEngine* _engine;
      };

    protected:
      Knob* _constraintKnob = nullptr;
      Knob* _lookAxisKnob = nullptr;
      Knob* _strengthKnob = nullptr;
      Knob* _rotateXKnob = nullptr;
      Knob* _rotateYKnob = nullptr;
      Knob* _rotateZKnob = nullptr;
      Knob* _applyTranslateKnob = nullptr;
      Knob* _applyRotateKnob = nullptr;
      Knob* _applyScaleKnob = nullptr;
      Knob* _maintainOffsetFrameKnob = nullptr;
      Knob* _maintainOffsetKnob = nullptr;
      Knob* _setMaintainOffsetFrameToCurrentKnob = nullptr;

      int _constraint = 0;
      fdk::AxisDirection _lookAxis = fdk::AxisDirection::Z_PLUS;
      bool _rotateX = true;
      bool _rotateY = true;
      bool _rotateZ = true;
      double _strength = 1.0;
      bool _applyTranslate = true;
      bool _applyRotate = true;
      bool _applyScale = false;
      bool _maintainOffset = false;
      float _maintainOffsetFrame = 0.0;
    };

  }
}

#endif
