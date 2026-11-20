// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

// This is automatically-generated code.
// You may add custom code between the --(BEGIN CUSTOM CODE)-- and --(END CUSTOM CODE)-- markers.
// Code added anywhere else will be destroyed when the code is generated.

#ifndef USG_LUX_LISTAPI_H
#define USG_LUX_LISTAPI_H

#include "usg/api.h"

#include "usg/lux/LuxTokens.h"

#include "usg/geom/Prim.h"
#include "usg/base/ArrayTypes.h"

namespace usg {
namespace lux {

  // class ListAPI
  //
  // Attributes:
  //     rel lightList
  //     token lightList:cacheBehavior (allowed values: consumeAndHalt, consumeAndContinue, ignore)

  class USG_API ListAPI {
  public:
    ListAPI();
    ListAPI(const Prim& prim);
    ~ListAPI();

    bool validate();
    static bool isValidPrim(const Prim& prim);
    explicit operator bool() const { return isValidPrim(_prim); }

    void apply();

    Relationship createLightListRel();
    Relationship getLightListRel() const;
    void setLightList(const PathArray& value);
    PathArray getLightList() const;

    Attribute createCacheBehaviorAttr(const Value& defaultValue = Value());
    Attribute getCacheBehaviorAttr() const;
    void setCacheBehavior(const Token& value, fdk::TimeValue time = fdk::defaultTimeValue());
    Token getCacheBehavior(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    // --(BEGIN CUSTOM CODE)--
    // --(END CUSTOM CODE)--
  protected:
    Prim _prim;
  };

}
}

#endif
