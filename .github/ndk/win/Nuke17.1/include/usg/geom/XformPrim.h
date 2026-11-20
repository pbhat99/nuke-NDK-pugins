// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

// This is automatically-generated code.
// You may add custom code between the --(BEGIN CUSTOM CODE)-- and --(END CUSTOM CODE)-- markers.
// Code added anywhere else will be destroyed when the code is generated.

#ifndef USG_GEOM_XFORMPRIM_H
#define USG_GEOM_XFORMPRIM_H

#include "usg/api.h"

#include "usg/geom/GeomTokens.h"

#include "usg/geom/Prim.h"
#include "usg/geom/XformablePrim.h"

namespace usg {

  // class XformPrim
  //   inherits from Xformable, Imageable
  //
  // Inherited attributes:
  //   From Xformable:
  //     uniform token[] xformOpOrder
  //   From Imageable:
  //     rel proxyPrim
  //     uniform token purpose = "default" (allowed values: default, render, proxy, guide)
  //     token visibility = "inherited" (allowed values: inherited, invisible)

  class USG_API XformPrim : public XformablePrim {
  public:
    XformPrim();
    XformPrim(const Prim& prim);
    ~XformPrim();

    bool validate();
    static bool isValidPrim(const Prim& prim);
    static XformPrim defineInLayer(const LayerRef& layer, const Path& path);
    static XformPrim overrideInLayer(const LayerRef& layer, const Prim& prim);
    static XformPrim getInStage(const StageRef& stage, const Path& path);

    // --(BEGIN CUSTOM CODE)--
    // --(END CUSTOM CODE)--
  };

}

#endif
