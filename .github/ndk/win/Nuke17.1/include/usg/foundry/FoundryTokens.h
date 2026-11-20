// Copyright (c) 2025 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef USG_FNUSDSHIM_FOUNDRYTOKENS_H
#define USG_FNUSDSHIM_FOUNDRYTOKENS_H

#include "usg/api.h"
#include "usg/base/Token.h"

namespace usg {
  struct USG_API FoundryTokensDef {
    Token::Lazy fnSubEngine{"fnSubEngine"};
    Token::Lazy engineArgs{"engine:args"};
    Token::Lazy engineName{"engine:name"};
    struct {
      Token::Lazy FnSubEngineAPI{"FnSubEngineAPI"};
    } types;
  };

  extern USG_API FoundryTokensDef FoundryTokens;
}
#endif
