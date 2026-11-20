// Copyright (c) 2021 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef DDIMAGE_LAYERI_H
#define DDIMAGE_LAYERI_H

#include "DDImage_API.h"

#include "ChannelSet.h"

#include <string>

namespace DD {
  namespace Image {

    //! \class DD::Image::LayerI
    //! Provides the user interface for a layer.
    class DDImage_API LayerI
    {
    public:

      //! Returns true if it is a built-in system layer, otherwise false.
      virtual bool builtIn() const = 0;

      //! Returns a ChannelSet containing all the channels in this layer.
      virtual ChannelSet getChannelSet() const = 0;

      //! Returns the name of this layer.
      virtual const std::string& name() const = 0;
    };

    //! Returns a pointer to a layer instance based on the layer name.
    DDImage_API LayerI* GetLayer(const std::string& name);
  }
}

#endif
