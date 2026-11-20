// Copyright (c) 2025 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef NDK_GEO_UTILS_CONSTRAIN_H
#define NDK_GEO_UTILS_CONSTRAIN_H

#include "ndk/api.h"

#include "fdk/math/Mat4.h"

#include <optional>

namespace ndk {

  enum class ConstraintMode {
    LookAt,
    Parent,
    Transformation,
    Translation,
    Rotation,
    Scale
  };

  //! True for constraints for which an offset can be maintained (all but ConstraintMode::LookAt)
  NDK_API bool CanMaintainOffsetOnConstraint(ndk::ConstraintMode constraint);

  //! Creates constrained matrices
  class NDK_API ConstraintMatrixParams
  {
  public:
    //! Enable application of the constraint
    bool enable;

    //! The matrix to be constrained
    fdk::Mat4d sourceXform;

    //! The matrix to constrain to. For ConstraintMode::LookAt we extract the point to look at from the translation of this matrix
    fdk::Mat4d targetXform;

    //! [ConstraintMode::LookAt only] Enable rotation around X axis
    bool rotateX;

    //! [ConstraintMode::LookAt only] Enable rotation around Y axis
    bool rotateY;

    //! [ConstraintMode::LookAt only] Enable rotation around Z axis
    bool rotateZ;

    //! [Currently ConstraintMode::LookAt only] Interpolates between 0 (matrix unmodified) and 1 (fully looking at target)
    double strength;

    //! [Constraint::LookAt only] Rotation axis direction
    fdk::AxisDirection lookAxis;

    //! [ConstraintMode::Parent/Transformation only] Enable use of the translation component of the target matrix
    bool applyTranslation;

    //! [ConstraintMode::Parent/Transformation only] Enable use of the rotation component of the target matrix
    bool applyRotation;

    //! [ConstraintMode::Parent/Transformation only] Enable use of the scale component of the target matrix
    bool applyScale;

    //! [ConstraintMode::Parent/Transformation/Translation/Rotation/Scale] Optional, maintain relative offset to this matrix
    std::optional<fdk::Mat4d> targetXformAtMaintainOffsetFrame;

    //! [ConstraintMode::Transformation/Translation/Rotation/Scale] Optional, source matrix at the time of the offset frame
    std::optional<fdk::Mat4d> sourceXformAtMaintainOffsetFrame;

    //! Returns the constrained matrix
    fdk::Mat4d getConstraintMatrix(ConstraintMode constraint) const;

  };

} // namespace ndk

#endif
