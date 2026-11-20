// Copyright (c) 2026 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef NDK_BASE_COLORTEMPERATURE_H
#define NDK_BASE_COLORTEMPERATURE_H

#include "ndk/api.h"

#include "fdk/math/Vec3.h"

namespace ndk {
  //! Converts a color temperature (in Kelvin) to an RGB color.
  fdk::Vec3f NDK_API ColorTemperatureAsRgb(float colorTemperature);
}

#endif
