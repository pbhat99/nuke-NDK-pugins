// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

// This is automatically-generated code.
// You may add custom code between the --(BEGIN CUSTOM CODE)-- and --(END CUSTOM CODE)-- markers.
// Code added anywhere else will be destroyed when the code is generated.

#ifndef USG_LUX_BOUNDABLELIGHTBASEPRIM_H
#define USG_LUX_BOUNDABLELIGHTBASEPRIM_H

#include "usg/api.h"

#include "usg/lux/LuxTokens.h"

#include "usg/geom/Prim.h"

namespace usg {
namespace lux {

  // class BoundableLightBasePrim
  //   inherits from Prim

  class USG_API BoundableLightBasePrim : public Prim {
  public:
    BoundableLightBasePrim();
    BoundableLightBasePrim(const Prim& prim);
    ~BoundableLightBasePrim();

    bool validate();
    static bool isValidPrim(const Prim& prim);
    static BoundableLightBasePrim overrideInLayer(const LayerRef& layer, const Prim& prim);
    static BoundableLightBasePrim getInStage(const StageRef& stage, const Path& path);

    // --(BEGIN CUSTOM CODE)--
    // --(END CUSTOM CODE)--
  };

}
}

#endif
