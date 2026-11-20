// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

// This is automatically-generated code.
// You may add custom code between the --(BEGIN CUSTOM CODE)-- and --(END CUSTOM CODE)-- markers.
// Code added anywhere else will be destroyed when the code is generated.

#ifndef USG_GEOM_TOKENS_H
#define USG_GEOM_TOKENS_H

#include "usg/api.h"

#include "usg/base/Token.h"

namespace usg {
  struct USG_API TokensDef {
    Token::Lazy excludes{"excludes"};
    Token::Lazy expansionRule{"expansionRule"};
    Token::Lazy explicitOnly{"explicitOnly"};
    Token::Lazy expandPrims{"expandPrims"};
    Token::Lazy expandPrimsAndProperties{"expandPrimsAndProperties"};
    Token::Lazy includeRoot{"includeRoot"};
    Token::Lazy includes{"includes"};
    Token::Lazy membershipExpression{"membershipExpression"};
    struct {
      Token::Lazy APISchemaBase{"APISchemaBase"};
      Token::Lazy ClipsAPI{"ClipsAPI"};
      Token::Lazy CollectionAPI{"CollectionAPI"};
      Token::Lazy ModelAPI{"ModelAPI"};
      Token::Lazy Typed{"Typed"};
    } types;
    // --(BEGIN CUSTOM CODE)--
    // --(END CUSTOM CODE)--
  };

  extern USG_API TokensDef Tokens;
}

#endif
