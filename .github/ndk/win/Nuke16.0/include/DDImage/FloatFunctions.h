// FloatFunctions.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_FloatFunctions_h
#define DDImage_FloatFunctions_h

#ifndef _WIN32
#include <stdint.h>
#endif

#ifdef _WIN32
#include <float.h>
#else
#include <math.h>
#endif

/*!
 */

namespace DD
{
  namespace Image
  {
    /// Accepts float or double.
    template <class T, class V>
    inline bool IsEqualFloat(T val1, T val2)
    {
      union FloatBitter {
        volatile V uint;
        volatile T flt;
      };

      FloatBitter fb1, fb2;
      fb1.flt = val1;
      fb2.flt = val2;

      return fb1.uint == fb2.uint;
    }

    inline bool IsEqual(float val1, float val2)
    {
      return IsEqualFloat<float, unsigned int>(val1, val2);
    }

    inline bool IsEqual(double val1, double val2)
    {
#ifndef _WIN32
      typedef uint64_t UInt64Type;
#else
      typedef unsigned __int64 UInt64Type;
#endif      
      return IsEqualFloat<double, UInt64Type>(val1, val2);
    }

    inline bool IsNonZero(float val1)
    {
      return !IsEqual(val1, 0.0f);
    }
    inline bool IsNonZero(double val1)
    {
      return !IsEqual(val1, 0.0);
    }
  }
}  // end of namespace DD::Image

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
