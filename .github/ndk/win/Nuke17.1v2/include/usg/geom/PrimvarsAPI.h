// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

// This is automatically-generated code.
// You may add custom code between the --(BEGIN CUSTOM CODE)-- and --(END CUSTOM CODE)-- markers.
// Code added anywhere else will be destroyed when the code is generated.

#ifndef USG_GEOM_PRIMVARSAPI_H
#define USG_GEOM_PRIMVARSAPI_H

#include "usg/api.h"

#include "usg/geom/GeomTokens.h"

#include "usg/geom/Prim.h"
#include "usg/geom/Primvar.h"

namespace usg {

  // class PrimvarsAPI

  class USG_API PrimvarsAPI {
  public:
    PrimvarsAPI();
    PrimvarsAPI(const Prim& prim);
    ~PrimvarsAPI();

    bool validate();
    static bool isValidPrim(const Prim& prim);
    explicit operator bool() const { return isValidPrim(_prim); }

    void apply();

    // --(BEGIN CUSTOM CODE)--

    void block(const Token& name);

    Primvar createPrimvar(const Token& name,
                          const Value::Type& typeName,
                          const Token& interpolation=Token(),
                          const int elementSize=-1);

    // --(END CUSTOM CODE)--
  protected:
    Prim _prim;
  };

}

#endif
