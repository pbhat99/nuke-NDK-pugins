// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

// This is automatically-generated code.
// You may add custom code between the --(BEGIN CUSTOM CODE)-- and --(END CUSTOM CODE)-- markers.
// Code added anywhere else will be destroyed when the code is generated.

#ifndef USG_GEOM_XFORMCOMMONAPI_H
#define USG_GEOM_XFORMCOMMONAPI_H

#include "usg/api.h"

#include "usg/geom/GeomTokens.h"

#include "usg/geom/Prim.h"

namespace usg {

  // class XformCommonAPI

  class USG_API XformCommonAPI {
  public:
    XformCommonAPI();
    XformCommonAPI(const Prim& prim);
    ~XformCommonAPI();

    bool validate();
    static bool isValidPrim(const Prim& prim);
    explicit operator bool() const { return isValidPrim(_prim); }

    void apply();

    // --(BEGIN CUSTOM CODE)--
    // --(END CUSTOM CODE)--
  protected:
    Prim _prim;
  };

}

#endif
