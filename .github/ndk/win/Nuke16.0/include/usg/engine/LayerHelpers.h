// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef USG_ENGINE_LAYERHELPERS_H
#define USG_ENGINE_LAYERHELPERS_H

#include "usg/geom/Layer.h"

#include <vector>
#include <string>

namespace usg 
{
  namespace LayerHelpers 
  {
    //! Reserves a number of sublayers on \a toLayer. New empty layers are added as necessary.
    void ReserveLayers(const usg::LayerRef& toLayer, size_t numLayers);

    //! Copies a stack of sublayers to \a toLayer, resizing if necessary.
    void CopyLayers(const usg::LayerRef& toLayer, const std::vector<std::string>& subLayers);
  }
}

#endif