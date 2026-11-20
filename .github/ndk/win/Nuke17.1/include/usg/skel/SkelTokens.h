// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

// This is automatically-generated code.
// You may add custom code between the --(BEGIN CUSTOM CODE)-- and --(END CUSTOM CODE)-- markers.
// Code added anywhere else will be destroyed when the code is generated.

#ifndef USG_SKEL_SKELTOKENS_H
#define USG_SKEL_SKELTOKENS_H

#include "usg/api.h"
#include "usg/base/Token.h"

namespace usg {
namespace skel {
  struct USG_API SkelTokensDef {
    Token::Lazy bindTransforms{"bindTransforms"};
    Token::Lazy blendShapeWeights{"blendShapeWeights"};
    Token::Lazy blendShapes{"blendShapes"};
    Token::Lazy classicLinear{"classicLinear"};
    Token::Lazy dualQuaternion{"dualQuaternion"};
    Token::Lazy extent{"extent"};
    Token::Lazy jointNames{"jointNames"};
    Token::Lazy joints{"joints"};
    Token::Lazy normalOffsets{"normalOffsets"};
    Token::Lazy offsets{"offsets"};
    Token::Lazy pointIndices{"pointIndices"};
    Token::Lazy primvarsSkelGeomBindTransform{"primvars:skel:geomBindTransform"};
    Token::Lazy primvarsSkelJointIndices{"primvars:skel:jointIndices"};
    Token::Lazy primvarsSkelJointWeights{"primvars:skel:jointWeights"};
    Token::Lazy primvarsSkelSkinningMethod{"primvars:skel:skinningMethod"};
    Token::Lazy proxyPrim{"proxyPrim"};
    Token::Lazy purpose{"purpose"};
    Token::Lazy restTransforms{"restTransforms"};
    Token::Lazy rotations{"rotations"};
    Token::Lazy scales{"scales"};
    Token::Lazy skelAnimationSource{"skel:animationSource"};
    Token::Lazy skelBlendShapeTargets{"skel:blendShapeTargets"};
    Token::Lazy skelBlendShapes{"skel:blendShapes"};
    Token::Lazy skelJoints{"skel:joints"};
    Token::Lazy skelSkeleton{"skel:skeleton"};
    Token::Lazy translations{"translations"};
    Token::Lazy visibility{"visibility"};
    Token::Lazy xformOpOrder{"xformOpOrder"};
    struct {
      Token::Lazy BlendShape{"BlendShape"};
      Token::Lazy PackedJointAnimation{"PackedJointAnimation"};
      Token::Lazy SkelAnimation{"SkelAnimation"};
      Token::Lazy SkelBindingAPI{"SkelBindingAPI"};
      Token::Lazy SkelRoot{"SkelRoot"};
      Token::Lazy Skeleton{"Skeleton"};
    } types;
    // --(BEGIN CUSTOM CODE)--
    // --(END CUSTOM CODE)--
  };

  extern USG_API SkelTokensDef SkelTokens;
}
}
#endif
