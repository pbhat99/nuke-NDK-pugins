// Copyright (c) 2026 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef USG_NUKE_NUKETOKENS_H
#define USG_NUKE_NUKETOKENS_H

#include "usg/api.h"
#include "usg/base/Token.h"

namespace usg::nuke {
struct USG_API NukeTokensDef {
  Token::Lazy useDistantLight{"useDistantLight"};
  Token::Lazy useDefaultLights{"useDefaultLights"};
  Token::Lazy useDomeLight{"useDomeLight"};
  Token::Lazy domeLightTexturePath{"domeLightTexturePath"};
  struct {
    Token::Lazy NukeViewerSettings{"NukeViewerSettings"};
  } types;
};

extern USG_API NukeTokensDef NukeTokens;
}  // namespace usg::nuke

#endif  // USG_NUKE_NUKETOKENS_H
