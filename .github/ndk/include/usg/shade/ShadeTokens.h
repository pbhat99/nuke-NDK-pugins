// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

// This is automatically-generated code.
// You may add custom code between the --(BEGIN CUSTOM CODE)-- and --(END CUSTOM CODE)-- markers.
// Code added anywhere else will be destroyed when the code is generated.

#ifndef USG_SHADE_SHADETOKENS_H
#define USG_SHADE_SHADETOKENS_H

#include "usg/api.h"
#include "usg/base/Token.h"
namespace usg {
  namespace shade {
    struct USG_API ShadeTokensDef {
      Token::Lazy binding{ "binding" };
      Token::Lazy id{ "id" };
      Token::Lazy infoId{ "info:id" };
      Token::Lazy infoImplementationSource{ "info:implementationSource" };
      Token::Lazy outputsDisplacement{ "outputs:displacement" };
      Token::Lazy outputsSurface{ "outputs:surface" };
      Token::Lazy outputsVolume{ "outputs:volume" };
      Token::Lazy sourceAsset{ "sourceAsset" };
      Token::Lazy sourceCode{ "sourceCode" };
      struct {
        Token::Lazy ConnectableAPI{ "ConnectableAPI" };
        Token::Lazy CoordSysAPI{ "CoordSysAPI" };
        Token::Lazy Material{ "Material" };
        Token::Lazy MaterialBindingAPI{ "MaterialBindingAPI" };
        Token::Lazy ChildMaterialAPI{ "LookAPI" };
        Token::Lazy NodeDefAPI{ "NodeDefAPI" };
        Token::Lazy NodeGraph{ "NodeGraph" };
        Token::Lazy Shader{ "Shader" };
      } types;

      struct {
        Token::Lazy allPurpose{ "" };
        Token::Lazy full{ "full" };
        Token::Lazy preview{ "preview" };
      } purposes;
    };
      // --(BEGIN CUSTOM CODE)--
      struct USG_API MaterialAPITokensDef {
        Token::Lazy materialBinding{ "materialBinding" };
        Token::Lazy materialBind{ "materialBind" };
      };
      extern USG_API MaterialAPITokensDef MaterialAPITokens;
      // --(END CUSTOM CODE)--

    extern USG_API ShadeTokensDef ShadeTokens;
  }
}
#endif