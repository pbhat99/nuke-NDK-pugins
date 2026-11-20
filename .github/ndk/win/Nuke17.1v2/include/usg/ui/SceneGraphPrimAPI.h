// Copyright (c) 2024 The Foundry Visionmongers Ltd. All Rights Reserved.

// This is automatically-generated code.
// You may add custom code between the --(BEGIN CUSTOM CODE)-- and --(END CUSTOM CODE)-- markers.
// Code added anywhere else will be destroyed when the code is generated.

#ifndef USG_UI_SCENEGRAPHPRIMAPI_H
#define USG_UI_SCENEGRAPHPRIMAPI_H

#include "usg/api.h"
#include "usg/geom/Prim.h"
#include "usg/ui/UITokens.h"

namespace usg {
namespace ui {

  // class SceneGraphPrimAPI
  //
  // Attributes:
  //     uniform token ui:displayGroup
  //     uniform token ui:displayName

  class USG_API SceneGraphPrimAPI {
  public:
    SceneGraphPrimAPI();
    SceneGraphPrimAPI(const Prim& prim);
    ~SceneGraphPrimAPI();

    bool validate();
    static bool isValidPrim(const Prim& prim);
    explicit operator bool() const { return isValidPrim(_prim); }

    void apply();

    Attribute createDisplayGroupAttr(const Value& defaultValue = Value());
    Attribute getDisplayGroupAttr() const;
    void setDisplayGroup(const Token& value, fdk::TimeValue time = fdk::defaultTimeValue());
    Token getDisplayGroup(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createDisplayNameAttr(const Value& defaultValue = Value());
    Attribute getDisplayNameAttr() const;
    void setDisplayName(const Token& value, fdk::TimeValue time = fdk::defaultTimeValue());
    Token getDisplayName(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    // --(BEGIN CUSTOM CODE)--
    // --(END CUSTOM CODE)--
  protected:
    Prim _prim;
  };

}
}

#endif
