// Copyright (c) 2011 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef TRANSFORM2DKNOBI_H
#define TRANSFORM2DKNOBI_H

#include "DDImage/Matrix4.h"

namespace DD
{
  namespace Image
  {
    class Knob;
    class OutputContext;

    // specifies the type of control points the knob contains
    class DDImage_API Transform2d_KnobI
    {
    public:

      enum KNOB_ID
      {
        TRANSLATE,
        ROTATE,
        SCALE,
        // SKEW knob has been deprecated. Please use SKEWX instead
        SKEW,
        CENTER,
        GAUGE,
        SKEWX,
        SKEWY,
        SKEWORDER
      };

      virtual ~Transform2d_KnobI();
      
      //! Return the Knob this interface is on. Implementations should just "return this;"
      virtual Knob* baseKnob() = 0;

      virtual Knob* translateKnob() = 0;
          
      virtual Knob* rotateKnob() = 0;
      
      virtual Knob* scaleKnob() = 0;

      // Skew knob has been deprecated. Please use skewXKnob instead
      //virtual Knob* skewKnob() = 0;
      
      virtual Knob* skewXKnob() = 0;

      virtual Knob* skewYKnob() = 0;

      virtual Knob* skewOrderKnob() = 0;

      virtual Knob* centerKnob() = 0;
      
      virtual Knob* gaugeKnob() = 0;

      virtual void setAutoKey( bool autoKey ) = 0;

      /**
       * this should help identify critical moments when a single knob
       * may set its auto key to false in order to perform reset or any other sensitive operation that may trigger auto key
       */
      virtual bool isKnobAutoKeying( KNOB_ID knob ) = 0;

      //! Return the transform of all constituent controls, at an optional OutputContext.
      virtual void matrix(Matrix4& m, const OutputContext& context) const = 0;

    };    
  }
}

#endif // TRANSFORM2DKNOBI_H
