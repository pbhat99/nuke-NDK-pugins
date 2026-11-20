// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FDK_MATH_TYPES_H
#define FDK_MATH_TYPES_H

#include "fdk/math/half.h"
#include "fdk/math/Mat4.h"
#include "fdk/math/Vec2.h"
#include "fdk/math/Vec3.h"
#include "fdk/math/Vec4.h"
#include "fdk/math/Quat.h"
#include "fdk/math/Box2.h"
#include "fdk/math/Box3.h"

#include <vector>

namespace fdk
{

  // Aliases for various array types.
  using BoolList   = std::vector<bool>;
  using CharList   = std::vector<char>;
  using IntList    = std::vector<int32_t>;
  using Int32List  = std::vector<int32_t>;
  using Int64List  = std::vector<int64_t>;
  using UintList   = std::vector<uint32_t>;
  using Uint32List = std::vector<uint32_t>;
  using Uint64List = std::vector<uint64_t>;
  using HalfList   = std::vector<fdk::half>;
  using FloatList  = std::vector<float>;
  using DoubleList = std::vector<double>;

  using Vec2fList = std::vector<fdk::Vec2f>;
  using Vec2dList = std::vector<fdk::Vec2d>;
  using Vec2iList = std::vector<fdk::Vec2i>;
  using Vec2hList = std::vector<fdk::Vec2h>;

  using Vec3fList = std::vector<fdk::Vec3f>;
  using Vec3dList = std::vector<fdk::Vec3d>;
  using Vec3iList = std::vector<fdk::Vec3i>;
  using Vec3hList = std::vector<fdk::Vec3h>;

  using Vec4fList = std::vector<fdk::Vec4f>;
  using Vec4dList = std::vector<fdk::Vec4d>;
  using Vec4iList = std::vector<fdk::Vec4i>;
  using Vec4hList = std::vector<fdk::Vec4h>;

  using QuathList = std::vector<fdk::Quath>;
  using QuatfList = std::vector<fdk::Quatf>;
  using QuatdList = std::vector<fdk::Quatd>;

  using Mat4fList = std::vector<fdk::Mat4f>;
  using Mat4dList = std::vector<fdk::Mat4d>;

  using Box2fList = std::vector<fdk::Box2f>;
  using Box2dList = std::vector<fdk::Box2d>;
  using Box2iList = std::vector<fdk::Box2i>;
  using Box2hList = std::vector<fdk::Box2h>;

  using Box3fList = std::vector<fdk::Box3f>;
  using Box3dList = std::vector<fdk::Box3d>;
  using Box3iList = std::vector<fdk::Box3i>;
  using Box3hList = std::vector<fdk::Box3h>;

}

#endif
