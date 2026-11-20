// Copyright (c) 2011 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_RGB_h
#define DDImage_RGB_h

namespace DD {
  namespace Image {

    /// Convert an RGB linear triple to a luma value, using 
    /// factors from ITU Recommendation BT.709 (rec709)
    static inline float y_convert_rec709(float r, float g, float b)
    {
      return r * 0.2125f + g * 0.7154f + b * 0.0721f;
    }

  }
}

#endif
