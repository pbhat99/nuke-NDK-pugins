// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

// This is automatically-generated code.
// You may add custom code between the --(BEGIN CUSTOM CODE)-- and --(END CUSTOM CODE)-- markers.
// Code added anywhere else will be destroyed when the code is generated.

#ifndef USG_GEOM_GEOMSUBSETPRIM_H
#define USG_GEOM_GEOMSUBSETPRIM_H

#include "usg/api.h"

#include "usg/geom/GeomTokens.h"

#include "usg/geom/Prim.h"
#include "usg/base/ArrayTypes.h"

namespace usg {

  // class GeomSubsetPrim
  //   inherits from Prim
  //
  // Attributes:
  //     uniform token elementType = "face" (allowed values: face)
  //     uniform token familyName
  //     int[] indices

  class USG_API GeomSubsetPrim : public Prim {
  public:
    GeomSubsetPrim();
    GeomSubsetPrim(const Prim& prim);
    ~GeomSubsetPrim();

    bool validate();
    static bool isValidPrim(const Prim& prim);
    static GeomSubsetPrim defineInLayer(const LayerRef& layer, const Path& path);
    static GeomSubsetPrim overrideInLayer(const LayerRef& layer, const Prim& prim);
    static GeomSubsetPrim getInStage(const StageRef& stage, const Path& path);

    Attribute createElementTypeAttr(const Value& defaultValue = Value());
    Attribute getElementTypeAttr() const;
    void setElementType(const Token& value, fdk::TimeValue time = fdk::defaultTimeValue());
    Token getElementType(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createFamilyNameAttr(const Value& defaultValue = Value());
    Attribute getFamilyNameAttr() const;
    void setFamilyName(const Token& value, fdk::TimeValue time = fdk::defaultTimeValue());
    Token getFamilyName(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createIndicesAttr(const Value& defaultValue = Value());
    Attribute getIndicesAttr() const;
    void setIndices(const IntArray& value, fdk::TimeValue time = fdk::defaultTimeValue());
    IntArray getIndices(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    // --(BEGIN CUSTOM CODE)--
    // --(END CUSTOM CODE)--
  };

}

#endif
