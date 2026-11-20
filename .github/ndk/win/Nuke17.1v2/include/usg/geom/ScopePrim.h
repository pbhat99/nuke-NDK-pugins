// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

// This is automatically-generated code.
// You may add custom code between the --(BEGIN CUSTOM CODE)-- and --(END CUSTOM CODE)-- markers.
// Code added anywhere else will be destroyed when the code is generated.

#ifndef USG_GEOM_SCOPEPRIM_H
#define USG_GEOM_SCOPEPRIM_H

#include "usg/api.h"

#include "usg/geom/GeomTokens.h"

#include "usg/geom/Prim.h"
#include "usg/geom/ImageablePrim.h"

namespace usg {

  // class ScopePrim
  //   inherits from Imageable
  //
  // Inherited attributes:
  //   From Imageable:
  //     rel proxyPrim
  //     uniform token purpose = "default" (allowed values: default, render, proxy, guide)
  //     token visibility = "inherited" (allowed values: inherited, invisible)

  class USG_API ScopePrim : public ImageablePrim {
  public:
    ScopePrim();
    ScopePrim(const Prim& prim);
    ~ScopePrim();

    bool validate();
    static bool isValidPrim(const Prim& prim);
    static ScopePrim defineInLayer(const LayerRef& layer, const Path& path);
    static ScopePrim overrideInLayer(const LayerRef& layer, const Prim& prim);
    static ScopePrim getInStage(const StageRef& stage, const Path& path);

    // --(BEGIN CUSTOM CODE)--
    // --(END CUSTOM CODE)--
  };

}

#endif
