// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef NDK_GEO_UTILS_INTERSECT_H
#define NDK_GEO_UTILS_INTERSECT_H

#include "usg/geom/Path.h"
#include "ndk/api.h"

#include "fdk/math/Vec3.h"
#include "fdk/math/Ray.h"

#include "usg/base/ArrayTypes.h"

#include <unordered_set>

namespace ndk {

  // A struct filled in by the Intersect functions with the results of a collision detection.
  struct NDK_API CollisionResult {
    float        collisionTime = std::numeric_limits<float>::max();  //! The ray time of the hit
    fdk::Vec3f   collisionNormal{0.0f, 0.0f, 0.0f};                  //! The normal at the hit point
    fdk::Vec3f   collisionPosition{0.0f, 0.0f, 0.0f};                //! The hit point itself
    unsigned int collisionPrimitiveIdx = 0;                          //! The index of the primitive that was hit
    unsigned int collisionFaceIdx = 0;                               //! The face index of the hit primitive
    bool         collided = false;                                   //! true if a collision occurred
  };

  //! A struct containing hit testing parameters
  struct NDK_API IntersectionContext {
    bool returnClosestHit = true; //< True to return the closest hit, otherwise return any hit
    std::unordered_set<usg::Path> excludePaths; //< Exclude these prims from intersection testing. Takes precedence over includeType
    std::unordered_set<usg::Token> includeType; //< Only test against prims of these types
  };

  //! Intersect a ray with a triangle
  bool NDK_API Intersect(const fdk::Rayf& ray, const fdk::Vec3f& p0, const fdk::Vec3f& p1, const fdk::Vec3f& p2, CollisionResult& result);

  //! Intersect a ray with a Box
  bool NDK_API Intersect(const fdk::Rayf& ray, const fdk::Box3f& box, float& minT, float& maxT);

  //! Intersect a ray with a mesh
  bool NDK_API Intersect(const IntersectionContext& context,
                             const fdk::Rayf& ray,
                             const usg::Vec3fArray& points,
                             const usg::IntArray& faceCounts,
                             const usg::IntArray& faceVertexIndices,
                             CollisionResult& result);

}

#endif
