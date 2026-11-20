// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

// This is automatically-generated code.
// You may add custom code between the --(BEGIN CUSTOM CODE)-- and --(END CUSTOM CODE)-- markers.
// Code added anywhere else will be destroyed when the code is generated.

#ifndef USG_GEOM_VISIBILITYAPI_H
#define USG_GEOM_VISIBILITYAPI_H

#include "usg/api.h"

#include "usg/geom/GeomTokens.h"

#include "usg/geom/Prim.h"

namespace usg {

  // class VisibilityAPI
  //
  // Attributes:
  //     uniform token guideVisibility = "invisible" (allowed values: inherited, invisible, visible)
  //     uniform token proxyVisibility = "inherited" (allowed values: inherited, invisible, visible)
  //     uniform token renderVisibility = "inherited" (allowed values: inherited, invisible, visible)

  class USG_API VisibilityAPI {
  public:
    VisibilityAPI();
    VisibilityAPI(const Prim& prim);
    ~VisibilityAPI();

    bool validate();
    static bool isValidPrim(const Prim& prim);
    explicit operator bool() const { return isValidPrim(_prim); }

    void apply();

    Attribute createGuideVisibilityAttr(const Value& defaultValue = Value());
    Attribute getGuideVisibilityAttr() const;
    void setGuideVisibility(const Token& value, fdk::TimeValue time = fdk::defaultTimeValue());
    Token getGuideVisibility(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createProxyVisibilityAttr(const Value& defaultValue = Value());
    Attribute getProxyVisibilityAttr() const;
    void setProxyVisibility(const Token& value, fdk::TimeValue time = fdk::defaultTimeValue());
    Token getProxyVisibility(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createRenderVisibilityAttr(const Value& defaultValue = Value());
    Attribute getRenderVisibilityAttr() const;
    void setRenderVisibility(const Token& value, fdk::TimeValue time = fdk::defaultTimeValue());
    Token getRenderVisibility(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    // --(BEGIN CUSTOM CODE)--
    // --(END CUSTOM CODE)--
  protected:
    Prim _prim;
  };

}

#endif
