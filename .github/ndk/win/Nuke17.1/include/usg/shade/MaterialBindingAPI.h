// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef USG_SHADE_MATERIALBINDINGAPI_H
#define USG_SHADE_MATERIALBINDINGAPI_H

#include "usg/geom/Prim.h"
#include "usg/shade/ShadeTokens.h"

namespace usg::shade {
  class USG_API MaterialBindingAPI {
  public:
    class Impl;

    MaterialBindingAPI();
    explicit MaterialBindingAPI(const Prim& prim);
    ~MaterialBindingAPI() = default;

    explicit operator bool() const;

    static bool isValidPrim(const Prim& prim) { return prim.isValid(); }

    bool validate();
    void apply() const;
    Relationship computeBoundMaterial(const Token& purpose = ShadeTokens.purposes.allPurpose, bool supportLegacyBindings = true) const;

  private:
    std::unique_ptr<Impl, void(*)(Impl*)> _pImpl;
  };

}

#endif
