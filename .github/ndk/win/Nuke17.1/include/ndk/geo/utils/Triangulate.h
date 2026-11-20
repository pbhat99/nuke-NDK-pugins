// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef NDK_GEO_UTILS_TRIANGULATE_H
#define NDK_GEO_UTILS_TRIANGULATE_H

#include "ndk/api.h"

#include "fdk/math/Vec3.h"

#include <vector>

namespace ndk {

  //! Triangulate a polygon using ear-clipping.
  void NDK_API Triangulate(const fdk::Vec3f* points, const int* faceVertexIndices, int n, std::vector<int>& triangles);

}

#endif
