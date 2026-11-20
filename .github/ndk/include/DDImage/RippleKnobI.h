// Copyright (c) 2011 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef RIPPLEKNOBI_H
#define RIPPLEKNOBI_H

#include "DDImage/DDImage_API.h"

namespace DD
{
  namespace Image
  {
    
    enum RippleType
    {
      eRippleOff,
      eRippleAll,
      eRippleFromStart,
      eRippleToEnd,
      eRippleRange
    };

    struct DDImage_API Ripple_KnobInfo
    {
      Ripple_KnobInfo() : type(eRippleOff), rangeFrom(1), rangeTo(1)
      {
      }
      
      //! returns whether the passed in frame is within the ripple range contained within this
      //! struct, based on the type enum and the rangeFrom and rangeTo values
      bool isFrameInRippleRange(float frame) const;
      //! same as above, but returns false if the frame is the same as the passed in current frame
      bool isFrameInRippleRangeIgnoreCurrent(float frame, float currentFrame) const;

      RippleType  type;
      int         rangeFrom;
      int         rangeTo;
    };
  
    class DDImage_API Ripple_KnobI
    {
    public:
      virtual ~Ripple_KnobI();
          
      virtual Ripple_KnobInfo getState() = 0;      
    };
  }
}

#endif // RIPPLEKNOBI_H
