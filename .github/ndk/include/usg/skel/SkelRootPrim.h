// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

// This is automatically-generated code.
// You may add custom code between the --(BEGIN CUSTOM CODE)-- and --(END CUSTOM CODE)-- markers.
// Code added anywhere else will be destroyed when the code is generated.

#ifndef USG_SKEL_SKELROOTPRIM_H
#define USG_SKEL_SKELROOTPRIM_H

#include "usg/api.h"

#include "usg/skel/SkelTokens.h"

#include "usg/geom/Prim.h"
#include "usg/base/ArrayTypes.h"

namespace usg {
namespace skel {

  // class SkelRootPrim
  //   inherits from Prim
  //
  // Attributes:
  //     float3[] extent
  //     rel proxyPrim
  //     uniform token purpose = "default"
  //     token visibility = "inherited"
  //     uniform token[] xformOpOrder

  class USG_API SkelRootPrim : public Prim {
  public:
    SkelRootPrim();
    SkelRootPrim(const Prim& prim);
    ~SkelRootPrim();

    bool validate();
    static bool isValidPrim(const Prim& prim);
    static SkelRootPrim defineInLayer(const LayerRef& layer, const Path& path);
    static SkelRootPrim overrideInLayer(const LayerRef& layer, const Prim& prim);
    static SkelRootPrim getInStage(const StageRef& stage, const Path& path);

    Attribute createExtentAttr(const Value& defaultValue = Value());
    Attribute getExtentAttr() const;
    void setExtent(const Vec3fArray& value, fdk::TimeValue time = fdk::defaultTimeValue());
    Vec3fArray getExtent(fdk::TimeValue time = fdk::defaultTimeValue()) const;

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

    Attribute createXformOpOrderAttr(const Value& defaultValue = Value());
    Attribute getXformOpOrderAttr() const;
    void setXformOpOrder(const TokenArray& value, fdk::TimeValue time = fdk::defaultTimeValue());
    TokenArray getXformOpOrder(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    // --(BEGIN CUSTOM CODE)--
    static SkelRootPrim find(const Prim& prim);
    // --(END CUSTOM CODE)--
  };

}
}

#endif
