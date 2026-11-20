// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

/*! \file noise.h

    Noise generation functions built into the DDImage
    library. These are not in the DD::Image namespace because they
    resemble the functions in shader languages.

 */

#ifndef DDImage_noise_h
#define DDImage_noise_h

#include "DDImage_API.h"

namespace DD
{

  namespace Image
  {

    DDImage_API double noise(double x);
    DDImage_API double noise(double x, double y);
    DDImage_API double noise(double x, double y, double z);
    DDImage_API double noise(double x, double y, double z, double w);
    DDImage_API double fBm(double x, double y, double z,
                           int octaves, double lacunarity, double gain);
    DDImage_API double turbulence(double x, double y, double z,
                                  int octaves, double lacunarity, double gain);
    DDImage_API double p_random(int x);
    DDImage_API double p_random(int x, int y);
    DDImage_API double p_random(int x, int y, int z);
    DDImage_API double p_random(double x);
    DDImage_API double p_random(double x, double y);
    DDImage_API double p_random(double x, double y, double z);

  } // namespace Image

} // namespace DD

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
