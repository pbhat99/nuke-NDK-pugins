// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

// This is automatically-generated code.
// You may add custom code between the --(BEGIN CUSTOM CODE)-- and --(END CUSTOM CODE)-- markers.
// Code added anywhere else will be destroyed when the code is generated.

#ifndef USG_KATANA_KATANALIGHTAPI_H
#define USG_KATANA_KATANALIGHTAPI_H

#include "usg/api.h"

#include "usg/katana/KatanaTokens.h"

#include "usg/geom/Prim.h"

namespace usg {
  namespace katana {

    // class KatanaLightAPI
    //
    // Attributes:
    //     uniform double geometry:centerOfInterest = 20.0

    class USG_API KatanaLightAPI
    {
    public:
      KatanaLightAPI();
      KatanaLightAPI(const Prim& prim);
      ~KatanaLightAPI();

      bool validate();
      static bool isValidPrim(const Prim& prim);
      explicit operator bool() const { return isValidPrim(_prim); }

      void apply();

      Attribute createCenterOfInterestAttr(const Value& defaultValue = Value());
      Attribute getCenterOfInterestAttr() const;
      void setCenterOfInterest(double value, fdk::TimeValue time = fdk::defaultTimeValue());
      double getCenterOfInterest(fdk::TimeValue time = fdk::defaultTimeValue()) const;

      // --(BEGIN CUSTOM CODE)--
      // --(END CUSTOM CODE)--
    protected:
      Prim _prim;
    };

  }
}

#endif
