// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef USG_GEOM_XFORMCACHE_H
#define USG_GEOM_XFORMCACHE_H

#include "usg/api.h"
#include "fdk/base/Time.h"
#include "fdk/math/Mat4.h"

#include <memory>

namespace usg {
  class Prim;

  //! A class for caching transform queries. Create one of these when you need to query a set of transforms.
  //! This is not threadsafe, so you should create one per thread if multithreading.
  class USG_API XformCache {
  public:
    XformCache(fdk::TimeValue time);
   ~XformCache();

    //! Returns the local to world transformation of the prim.
    fdk::Mat4d getLocalToWorldTransform (const Prim& prim);

    //! Returns the local to world transformation of the prim's parent.
    fdk::Mat4d getParentToWorldTransform (const Prim& prim);

    //! Returns the local transformation of the prim.
    fdk::Mat4d getLocalTransformation (const Prim& prim, bool* resetsXformStack);

    //! Returns the transformation of the prim relative to \a ancestor.
    fdk::Mat4d computeRelativeTransform (const Prim& prim, const Prim& ancestor, bool* resetsXformStack);


  public:
    // For internal use only....
    class Impl;

  private:
    std::unique_ptr<XformCache::Impl, void(*)(XformCache::Impl*)> _pImpl;
  };

}

#endif
