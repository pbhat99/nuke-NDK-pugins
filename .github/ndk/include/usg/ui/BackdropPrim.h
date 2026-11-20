// Copyright (c) 2024 The Foundry Visionmongers Ltd. All Rights Reserved.

// This is automatically-generated code.
// You may add custom code between the --(BEGIN CUSTOM CODE)-- and --(END CUSTOM CODE)-- markers.
// Code added anywhere else will be destroyed when the code is generated.

#ifndef USG_UI_BACKDROPPRIM_H
#define USG_UI_BACKDROPPRIM_H

#include "usg/api.h"
#include "usg/geom/Prim.h"
#include "usg/ui/UITokens.h"

namespace usg {
namespace ui {

  // class BackdropPrim
  //   inherits from Prim
  //
  // Attributes:
  //     uniform token ui:description

  class USG_API BackdropPrim : public Prim {
  public:
    BackdropPrim();
    BackdropPrim(const Prim& prim);
    ~BackdropPrim();

    bool validate();
    static bool isValidPrim(const Prim& prim);
    static BackdropPrim defineInLayer(const LayerRef& layer, const Path& path);
    static BackdropPrim overrideInLayer(const LayerRef& layer, const Prim& prim);
    static BackdropPrim getInStage(const StageRef& stage, const Path& path);

    Attribute createDescriptionAttr(const Value& defaultValue = Value());
    Attribute getDescriptionAttr() const;
    void setDescription(const Token& value, fdk::TimeValue time = fdk::defaultTimeValue());
    Token getDescription(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    // --(BEGIN CUSTOM CODE)--
    // --(END CUSTOM CODE)--
  };

}
}

#endif
