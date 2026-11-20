// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

// This is automatically-generated code.
// You may add custom code between the --(BEGIN CUSTOM CODE)-- and --(END CUSTOM CODE)-- markers.
// Code added anywhere else will be destroyed when the code is generated.

#ifndef USG_LUX_NONBOUNDABLELIGHTBASEPRIM_H
#define USG_LUX_NONBOUNDABLELIGHTBASEPRIM_H

#include "usg/api.h"

#include "usg/lux/LuxTokens.h"

#include "usg/geom/Prim.h"

namespace usg {
namespace lux {

  // class NonboundableLightBasePrim
  //   inherits from Prim

  class USG_API NonboundableLightBasePrim : public Prim {
  public:
    NonboundableLightBasePrim();
    NonboundableLightBasePrim(const Prim& prim);
    ~NonboundableLightBasePrim();

    bool validate();
    static bool isValidPrim(const Prim& prim);
    static NonboundableLightBasePrim overrideInLayer(const LayerRef& layer, const Prim& prim);
    static NonboundableLightBasePrim getInStage(const StageRef& stage, const Path& path);

    // --(BEGIN CUSTOM CODE)--
    // --(END CUSTOM CODE)--
  };

}
}

#endif
