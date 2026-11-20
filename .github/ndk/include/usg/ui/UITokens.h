// Copyright (c) 2024 The Foundry Visionmongers Ltd. All Rights Reserved.

// This is automatically-generated code.
// You may add custom code between the --(BEGIN CUSTOM CODE)-- and --(END CUSTOM CODE)-- markers.
// Code added anywhere else will be destroyed when the code is generated.

#ifndef USG_UI_UITOKENS_H
#define USG_UI_UITOKENS_H

#include "usg/api.h"
#include "usg/base/Token.h"

namespace usg {
namespace ui {
  struct USG_API UITokensDef {
    Token::Lazy closed{"closed"};
    Token::Lazy minimized{"minimized"};
    Token::Lazy open{"open"};
    Token::Lazy uiDescription{"ui:description"};
    Token::Lazy uiDisplayGroup{"ui:displayGroup"};
    Token::Lazy uiDisplayName{"ui:displayName"};
    Token::Lazy uiNodegraphNodeDisplayColor{"ui:nodegraph:node:displayColor"};
    Token::Lazy uiNodegraphNodeExpansionState{"ui:nodegraph:node:expansionState"};
    Token::Lazy uiNodegraphNodeIcon{"ui:nodegraph:node:icon"};
    Token::Lazy uiNodegraphNodePos{"ui:nodegraph:node:pos"};
    Token::Lazy uiNodegraphNodeSize{"ui:nodegraph:node:size"};
    Token::Lazy uiNodegraphNodeStackingOrder{"ui:nodegraph:node:stackingOrder"};
    struct {
      Token::Lazy Backdrop{"Backdrop"};
      Token::Lazy NodeGraphNodeAPI{"NodeGraphNodeAPI"};
      Token::Lazy SceneGraphPrimAPI{"SceneGraphPrimAPI"};
    } types;
    // --(BEGIN CUSTOM CODE)--
    // --(END CUSTOM CODE)--
  };

  extern USG_API UITokensDef UITokens;
}
}
#endif
