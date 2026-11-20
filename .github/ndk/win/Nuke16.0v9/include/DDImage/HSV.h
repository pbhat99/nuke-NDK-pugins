// HSV.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

/*! \file HSV.h

   Conversion between RGB and HSV and HSL.

   The Hue is given in the range 0-1, with 0 being red. Multiply by
   360 to get the hue reported by other software. If the hue cannot
   be determined (due to a gray color) then h is set to 0 and true
   is returned.

   The colors may be outside the 0-1 range, this will result in
   saturation and value outside the 0-1 range. This may not convert
   bi-directionally without change.

   The RGB values may need to be converted using a DD::Image::LUT to
   the sRGB space before these functions will return values that match
   other software.

 */

#ifndef DDImage_HSV_h
#define DDImage_HSV_h

#include "DDImage_API.h"

namespace DD
{
  namespace Image
  {

    bool DDImage_API RGBtoHSV(float r, float g, float b,
                              float& h, float& s, float& v);
    void DDImage_API HSVtoRGB(float h, float s, float v,
                              float& r, float& g, float& b);
    bool DDImage_API RGBtoHSL(float r, float g, float b,
                              float& H, float& S, float& L);
    void DDImage_API HSLtoRGB(float H, float S, float L,
                              float& r, float& g, float& b);
    void DDImage_API HSVtoHSL(float h, float s, float v,
                              float& H, float& S, float& L);
    void DDImage_API HSLtoHSV(float H, float S, float L,
                              float& h, float& s, float& v);


  }
}

#endif
