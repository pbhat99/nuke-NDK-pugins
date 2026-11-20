// Copyright (c) 2023 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef NDK_GEO_SELECTION_GLOBALGEOSELECTION_H
#define NDK_GEO_SELECTION_GLOBALGEOSELECTION_H

#include "ndk/api.h"
#include "ndk/geo/selection/GeoSelection.h"

namespace ndk {

  namespace Selection {

    //! A singleton class for storing the global geometry selection
    class NDK_API GlobalGeoSelection
    {
    public:
      GlobalGeoSelection(GlobalGeoSelection const&) = delete;
      void operator=(GlobalGeoSelection const&) = delete;

      //! Return the only instance of the global geometry selection singleton
      static GlobalGeoSelection& GetInstance();

      //! Get the current geometry selection
      const GeoSelection& getCurrent() const;

      //! Get the current geometry selection, allowing modification
      GeoSelection& getWritableCurrent();

      //! Get the current anchor geometry selection
      const GeoSelection& getAnchor() const;

      //! Overwrite the anchor selection with the current selection
      void setAnchorFromCurrent();

      //! Add selections in the anchor selection to the current selection
      void addAnchorToCurrent();

      //! Clear the anchor selection
      void clearAnchor();

    private:
      GlobalGeoSelection();

      GeoSelection _curSelection;
      GeoSelection _anchorSelection;
    };

  }
}

#endif
