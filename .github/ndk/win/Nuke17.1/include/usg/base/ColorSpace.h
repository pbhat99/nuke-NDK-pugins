// Copyright (c) 2025 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef USG_BASE_COLORSPACE_H
#define USG_BASE_COLORSPACE_H

#include "usg/api.h"
#include "usg/base/Token.h"

#include "fdk/math/Vec3.h"

#include <memory>

namespace usg {

  //! A  wrapper for GfColorspace
  class USG_API ColorSpace
  {
  public:
    class Impl;

    ColorSpace();
    ColorSpace(const usg::Token& name);
    ~ColorSpace();

    ColorSpace& operator=(const ColorSpace& rhs);

    //! Converts pixels from source colorspace to this one
    void convert(const ColorSpace& source, fdk::Vec3f* rgb, size_t count);

  private:
    Impl* _pImpl = nullptr;
  };

}

#endif
