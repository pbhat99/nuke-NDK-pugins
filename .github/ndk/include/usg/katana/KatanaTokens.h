// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

// This is automatically-generated code.
// You may add custom code between the --(BEGIN CUSTOM CODE)-- and --(END CUSTOM CODE)-- markers.
// Code added anywhere else will be destroyed when the code is generated.

#ifndef USG_KATANA_KATANATOKENS_H
#define USG_KATANA_KATANATOKENS_H

#include "usg/api.h"
#include "usg/base/Token.h"

namespace usg {
  namespace katana {
    struct USG_API KatanaTokensDef {
      Token::Lazy geometryCenterOfInterest{ "geometry:centerOfInterest" };
      struct {
        Token::Lazy KatanaLightAPI{ "KatanaLightAPI" };
      } types;
      // --(BEGIN CUSTOM CODE)--
      // --(END CUSTOM CODE)--
    };

    extern USG_API KatanaTokensDef KatanaTokens;
  }
}
#endif
