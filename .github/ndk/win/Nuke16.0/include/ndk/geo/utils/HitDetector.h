// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef NDK_GEO_UTILS_HITDETECTOR_H
#define NDK_GEO_UTILS_HITDETECTOR_H

#include "ndk/api.h"
#include "ndk/geo/utils/Intersect.h"

#include "usg/geom/Stage.h"
#include "usg/geom/MeshPrim.h"

namespace ndk {

  //! A class for performing hit detection in a scene. Create one of these with the prims you want to test. Then create a HitDetector with it
  //! and call the intersect method. If using multiple threads, you must create an HitDetector for each thread.
  //! The HitDetectModel object will create an acceleration structure, and you can keep the object alive in order to avoid rebuilding this for
  //! subsequent hit tests.
  class NDK_API HitDetectModel {
  public:
    //! Create a hit detector onto the given prims
    HitDetectModel(const std::vector<usg::MeshPrim>& meshes, fdk::TimeValue time = fdk::defaultTimeValue());

    //! Create a hit detector onto all mesh descendents of the root prim
    HitDetectModel(const usg::Prim& root, fdk::TimeValue time = fdk::defaultTimeValue());

    //! Create a hit detector onto the given stage
    HitDetectModel(const usg::StageRef& stage, fdk::TimeValue time = fdk::defaultTimeValue());

    ~HitDetectModel();

  private:
    friend class HitDetector;
    std::unique_ptr<class HitDetectModelImpl> _pImpl;
  };

  class NDK_API HitDetector {
  public:
    //! Create a hit detector onto the given prims
    HitDetector(const HitDetectModel& model);

    ~HitDetector();

    //! Cast the ray and return hit geometry in \a result. If using multiple threads, an IntersectionContext must be created for every thread.
    bool intersect(const IntersectionContext& context, const fdk::Rayf& ray, CollisionResult& result) const;

  private:
    std::unique_ptr<class HitDetectorImpl> _pImpl;
  };

}

#endif
