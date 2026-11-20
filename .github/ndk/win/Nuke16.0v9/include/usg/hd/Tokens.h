// Copyright (c) 2025 The Foundry Visionmongers Ltd. All Rights Reserved.

#pragma once

#include "usg/api.h"
#include "usg/base/Token.h"

namespace usg::hd {
  USG_API const TokenArray& LightPrimTypeTokens();
    struct USG_API HdTokensDef {
      Token::Lazy rendererInstanceName{ "rendererInstanceName" };
      struct {
        Token::Lazy Disabled{ "disabled" };
        Token::Lazy SRGB{ "sRGB" };
        Token::Lazy OpenColorIO{ "openColorIO" };
      } colorCorrection;
      struct {
        Token::Lazy coordSysBinding{ "coordSysBinding" };
        Token::Lazy primvars{ "primvars" };
        Token::Lazy purpose{ "purpose" };
        Token::Lazy visibility{ "visibility" };
        Token::Lazy xform{ "xform" };
      } flatten;
    };

    extern USG_API HdTokensDef HdTokens;
} // namespace usg::hd
