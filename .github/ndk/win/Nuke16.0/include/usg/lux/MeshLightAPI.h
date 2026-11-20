// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

// This is automatically-generated code.
// You may add custom code between the --(BEGIN CUSTOM CODE)-- and --(END CUSTOM CODE)-- markers.
// Code added anywhere else will be destroyed when the code is generated.

#ifndef USG_LUX_MESHLIGHTAPI_H
#define USG_LUX_MESHLIGHTAPI_H

#include "usg/api.h"

#include "usg/lux/LuxTokens.h"

#include "usg/geom/Prim.h"

namespace usg {
namespace lux {

  // class MeshLightAPI
  //
  // Attributes:
  //     uniform token light:materialSyncMode = "materialGlowTintsLight"
  //     uniform token light:shaderId = "MeshLight"

  class USG_API MeshLightAPI {
  public:
    MeshLightAPI();
    MeshLightAPI(const Prim& prim);
    ~MeshLightAPI();

    bool validate();
    static bool isValidPrim(const Prim& prim);
    explicit operator bool() const { return isValidPrim(_prim); }

    void apply();

    Attribute createMaterialSyncModeAttr(const Value& defaultValue = Value());
    Attribute getMaterialSyncModeAttr() const;
    void setMaterialSyncMode(const Token& value, fdk::TimeValue time = fdk::defaultTimeValue());
    Token getMaterialSyncMode(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createShaderIdAttr(const Value& defaultValue = Value());
    Attribute getShaderIdAttr() const;
    void setShaderId(const Token& value, fdk::TimeValue time = fdk::defaultTimeValue());
    Token getShaderId(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    // --(BEGIN CUSTOM CODE)--
    // --(END CUSTOM CODE)--
  protected:
    Prim _prim;
  };

}
}

#endif
