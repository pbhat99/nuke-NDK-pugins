// CurveKnobI.h
// Copyright (c) 2012 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_CurveKnobI_h
#define DDImage_CurveKnobI_h

#include "DDImage/DDImage_API.h"
#include "DDImage/ElementI.h"

namespace DD
{
  namespace Image
  {
    class OutputContext;

    class DDImage_API CurveKnobI
    {
      public:
        typedef ElementI::ElementPtr ElementPtr;
      protected:
        CurveKnobI();
      private:
        CurveKnobI(const CurveKnobI&);
      public:
        virtual ~CurveKnobI();
        virtual ElementPtr rootElement() = 0;
        virtual ElementPtr toElement(const char* curvePath) = 0; 
        virtual void evaluate(const OutputContext &) = 0;
    };
  }
}

#endif

// Copyright (c) 2012 The Foundry Visionmongers Ltd.  All Rights Reserved.
