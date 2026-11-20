// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

// This is automatically-generated code.
// You may add custom code between the --(BEGIN CUSTOM CODE)-- and --(END CUSTOM CODE)-- markers.
// Code added anywhere else will be destroyed when the code is generated.

#ifndef USG_GEOM_IMAGEABLEPRIM_H
#define USG_GEOM_IMAGEABLEPRIM_H

#include "usg/api.h"
#include "usg/geom/Prim.h"
#include "usg/base/ArrayTypes.h"

namespace usg {

  // class ImageablePrim
  //   inherits from Prim
  //
  // Attributes:
  //     rel proxyPrim
  //     uniform token purpose = "default" (allowed values: default, render, proxy, guide)
  //     token visibility = "inherited" (allowed values: inherited, invisible)

  class USG_API ImageablePrim : public Prim {
  public:
    ImageablePrim();
    ImageablePrim(const Prim& prim);
    ~ImageablePrim();

    bool validate() override;
    static bool isValidPrim(const Prim& prim);
    static ImageablePrim overrideInLayer(const LayerRef& layer, const Prim& prim);
    static ImageablePrim getInStage(const StageRef& stage, const Path& path);

    Relationship createProxyPrimRel();
    Relationship getProxyPrimRel() const;
    void setProxyPrim(const PathArray& value);
    PathArray getProxyPrim() const;

    Attribute createPurposeAttr(const Value& defaultValue = Value());
    Attribute getPurposeAttr() const;
    void setPurpose(const Token& value, fdk::TimeValue time = fdk::defaultTimeValue());
    Token getPurpose(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createVisibilityAttr(const Value& defaultValue = Value());
    Attribute getVisibilityAttr() const;
    void setVisibility(const Token& value, fdk::TimeValue time = fdk::defaultTimeValue());
    Token getVisibility(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    // --(BEGIN CUSTOM CODE)--
    fdk::Box3d computeLocalBound(const fdk::TimeValue& time) const;
    fdk::Box3d computeWorldBound(const fdk::TimeValue& time) const;
    // --(END CUSTOM CODE)--
  };

}

#endif
