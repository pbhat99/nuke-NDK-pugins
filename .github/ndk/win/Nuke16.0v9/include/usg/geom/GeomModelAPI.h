// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

// This is automatically-generated code.
// You may add custom code between the --(BEGIN CUSTOM CODE)-- and --(END CUSTOM CODE)-- markers.
// Code added anywhere else will be destroyed when the code is generated.

#ifndef USG_GEOM_GEOMMODELAPI_H
#define USG_GEOM_GEOMMODELAPI_H

#include "usg/api.h"

#include "usg/geom/GeomTokens.h"

#include "usg/geom/Prim.h"
#include "fdk/math/Vec3.h"

namespace usg {

  // class GeomModelAPI
  //
  // Attributes:
  //     uniform bool model:applyDrawMode
  //     uniform token model:cardGeometry = "cross" (allowed values: cross, box, fromTexture)
  //     asset model:cardTextureXNeg
  //     asset model:cardTextureXPos
  //     asset model:cardTextureYNeg
  //     asset model:cardTextureYPos
  //     asset model:cardTextureZNeg
  //     asset model:cardTextureZPos
  //     uniform token model:drawMode = "inherited" (allowed values: origin, bounds, cards, default, inherited)
  //     uniform float3 model:drawModeColor = (0.18, 0.18, 0.18)

  class USG_API GeomModelAPI {
  public:
    GeomModelAPI();
    GeomModelAPI(const Prim& prim);
    ~GeomModelAPI();

    bool validate();
    static bool isValidPrim(const Prim& prim);
    explicit operator bool() const { return isValidPrim(_prim); }

    void apply();

    Attribute createApplyDrawModeAttr(const Value& defaultValue = Value());
    Attribute getApplyDrawModeAttr() const;
    void setApplyDrawMode(bool value, fdk::TimeValue time = fdk::defaultTimeValue());
    bool getApplyDrawMode(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createCardGeometryAttr(const Value& defaultValue = Value());
    Attribute getCardGeometryAttr() const;
    void setCardGeometry(const Token& value, fdk::TimeValue time = fdk::defaultTimeValue());
    Token getCardGeometry(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createCardTextureXNegAttr(const Value& defaultValue = Value());
    Attribute getCardTextureXNegAttr() const;
    void setCardTextureXNeg(const AssetPath& value, fdk::TimeValue time = fdk::defaultTimeValue());
    AssetPath getCardTextureXNeg(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createCardTextureXPosAttr(const Value& defaultValue = Value());
    Attribute getCardTextureXPosAttr() const;
    void setCardTextureXPos(const AssetPath& value, fdk::TimeValue time = fdk::defaultTimeValue());
    AssetPath getCardTextureXPos(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createCardTextureYNegAttr(const Value& defaultValue = Value());
    Attribute getCardTextureYNegAttr() const;
    void setCardTextureYNeg(const AssetPath& value, fdk::TimeValue time = fdk::defaultTimeValue());
    AssetPath getCardTextureYNeg(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createCardTextureYPosAttr(const Value& defaultValue = Value());
    Attribute getCardTextureYPosAttr() const;
    void setCardTextureYPos(const AssetPath& value, fdk::TimeValue time = fdk::defaultTimeValue());
    AssetPath getCardTextureYPos(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createCardTextureZNegAttr(const Value& defaultValue = Value());
    Attribute getCardTextureZNegAttr() const;
    void setCardTextureZNeg(const AssetPath& value, fdk::TimeValue time = fdk::defaultTimeValue());
    AssetPath getCardTextureZNeg(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createCardTextureZPosAttr(const Value& defaultValue = Value());
    Attribute getCardTextureZPosAttr() const;
    void setCardTextureZPos(const AssetPath& value, fdk::TimeValue time = fdk::defaultTimeValue());
    AssetPath getCardTextureZPos(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createDrawModeAttr(const Value& defaultValue = Value());
    Attribute getDrawModeAttr() const;
    void setDrawMode(const Token& value, fdk::TimeValue time = fdk::defaultTimeValue());
    Token getDrawMode(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createDrawModeColorAttr(const Value& defaultValue = Value());
    Attribute getDrawModeColorAttr() const;
    void setDrawModeColor(const fdk::Vec3f& value, fdk::TimeValue time = fdk::defaultTimeValue());
    fdk::Vec3f getDrawModeColor(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    // --(BEGIN CUSTOM CODE)--
    // --(END CUSTOM CODE)--
  protected:
    Prim _prim;
  };

}

#endif
