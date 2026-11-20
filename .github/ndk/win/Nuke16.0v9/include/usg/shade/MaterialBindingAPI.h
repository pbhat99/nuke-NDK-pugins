// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

// This is automatically-generated code.
// You may add custom code between the --(BEGIN CUSTOM CODE)-- and --(END CUSTOM CODE)-- markers.
// Code added anywhere else will be destroyed when the code is generated.

#ifndef USG_SHADE_MATERIALBINDINGAPI_H
#define USG_SHADE_MATERIALBINDINGAPI_H

#include "usg/api.h"

#include "usg/geom/Prim.h"
#include "usg/geom/MaterialPrim.h"


namespace usg {
namespace shade {

  // class MaterialBindingAPI

  class USG_API MaterialBindingAPI {
  public:
    MaterialBindingAPI();
    MaterialBindingAPI(const Prim& prim);
    ~MaterialBindingAPI();

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
}

#endif
