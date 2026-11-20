// Copyright (c) 2024 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef NDK_GEO_UTILS_DRAW_H
#define NDK_GEO_UTILS_DRAW_H

#include "ndk/api.h"

#include <cstdint> // for uint32_t

namespace ndk {

  //! Flags are defined as uints vs. enums so they can be OR'd & AND'd together.
  using DisplayOptionsMask = uint32_t;

  static constexpr DisplayOptionsMask   defaultDisplay() { return 0x00000000; }


  //! Options for drawing object icons.
  struct DisplayOptions
  {
    DisplayOptionsMask  mask;
  };


} // namespace ndk

#endif
