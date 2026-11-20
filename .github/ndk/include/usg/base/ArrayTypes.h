// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef USG_BASE_ARRAYTYPES_H
#define USG_BASE_ARRAYTYPES_H

#include "usg/base/Array.h"

#include "fdk/math/half.h"
#include "fdk/math/Mat4.h"
#include "fdk/math/Vec2.h"
#include "fdk/math/Vec3.h"
#include "fdk/math/Vec4.h"
#include "fdk/math/Quat.h"
#include "fdk/math/Box2.h"
#include "fdk/math/Box3.h"

namespace usg
{

  // Aliases for various array types.
  using StringArray = usg::Array<std::string>;
  using BoolArray   = usg::Array<bool>;
  using CharArray   = usg::Array<char>;
  using UCharArray  = usg::Array<unsigned char>;
  using IntArray    = usg::Array<int32_t>;
  using Int32Array  = usg::Array<int32_t>;
  using Int64Array  = usg::Array<int64_t>;
  using UintArray   = usg::Array<uint32_t>;
  using Uint32Array = usg::Array<uint32_t>;
  using Uint64Array = usg::Array<uint64_t>;
  using HalfArray   = usg::Array<fdk::half>;
  using FloatArray  = usg::Array<float>;
  using DoubleArray = usg::Array<double>;

  using Vec2fArray = usg::Array<fdk::Vec2f>;
  using Vec2dArray = usg::Array<fdk::Vec2d>;
  using Vec2iArray = usg::Array<fdk::Vec2i>;
  using Vec2hArray = usg::Array<fdk::Vec2h>;

  using Vec3fArray = usg::Array<fdk::Vec3f>;
  using Vec3dArray = usg::Array<fdk::Vec3d>;
  using Vec3iArray = usg::Array<fdk::Vec3i>;
  using Vec3hArray = usg::Array<fdk::Vec3h>;

  using Vec4fArray = usg::Array<fdk::Vec4f>;
  using Vec4dArray = usg::Array<fdk::Vec4d>;
  using Vec4iArray = usg::Array<fdk::Vec4i>;
  using Vec4hArray = usg::Array<fdk::Vec4h>;

  using QuathArray = usg::Array<fdk::Quath>;
  using QuatfArray = usg::Array<fdk::Quatf>;
  using QuatdArray = usg::Array<fdk::Quatd>;

  using Mat4fArray = usg::Array<fdk::Mat4f>;
  using Mat4dArray = usg::Array<fdk::Mat4d>;

  using Box2fArray = usg::Array<fdk::Box2f>;
  using Box2dArray = usg::Array<fdk::Box2d>;
  using Box2iArray = usg::Array<fdk::Box2i>;
  using Box2hArray = usg::Array<fdk::Box2h>;

  using Box3fArray = usg::Array<fdk::Box3f>;
  using Box3dArray = usg::Array<fdk::Box3d>;
  using Box3iArray = usg::Array<fdk::Box3i>;
  using Box3hArray = usg::Array<fdk::Box3h>;

}

#endif
