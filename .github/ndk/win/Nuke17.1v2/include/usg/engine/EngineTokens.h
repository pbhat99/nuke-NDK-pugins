// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef USG_ENGINE_TOKENS_H
#define USG_ENGINE_TOKENS_H

#include "usg/engine/api.h"
#include "usg/base/Token.h"

namespace usg {

  struct USG_ENGINE_API EngineTokensDef {
    struct Targets {
      Token defineGeometry{"define-geometry"};
      Token modifyValues{"modify-values"};
      Token editStage{"edit-stage"};
      Token affectStereo{"affect-stereo"};
    } target;
  };

  extern USG_ENGINE_API EngineTokensDef EngineTokens;

}
#endif
