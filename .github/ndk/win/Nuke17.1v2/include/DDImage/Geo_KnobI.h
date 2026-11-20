// Copyright (c) 2010 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DD_Image_Geo_KnobI_H
#define DD_Image_Geo_KnobI_H

#include "DDImage/DDImage_API.h"

#include "DDImage/Matrix4.h"

namespace DD
{
  namespace Image
  {
    class Knob;
    class Axis_KnobI;

    class DDImage_API Geo_KnobI
    {
    protected:
      Knob* _knob;
      Axis_KnobI* _localMatrixSource;
    public:
      Geo_KnobI(Knob* knob) : _knob(knob), _localMatrixSource(nullptr) { }
      virtual ~Geo_KnobI() { }
      
      Matrix4 localMatrix() const;

      virtual void setMatrixSource(Axis_KnobI* source) { _localMatrixSource = source; }
    };

  }
}

#endif
