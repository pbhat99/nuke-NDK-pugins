// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FDK_BASE_MOTIONSTEPUTILS_H
#define FDK_BASE_MOTIONSTEPUTILS_H

#include "fdk/base/Time.h"
#include "fdk/math/types.h"
#include "fdk/math/Mat4.h"


//-------------------------------------------------------------------------
//
// Utility functions for getting the index and interpolation
// offset inside a motion step.
//
//------------------------------------------------------------------------


namespace fdk
{

  enum
  {
    MOTIONSTEP_START,   //!<
    MOTIONSTEP_MID,     //!<
    MOTIONSTEP_END      //!<
  };


  /*! Finds the motion step that 'time' falls inside from the motion_times list,
      clamped between 0 and (motion_times.size() - 1).
  */
  uint32_t  getMotionStep(const TimeValueList& motion_times,
                          const TimeValue&     time);

  /*! Finds the motion step that 'time' falls inside and the interpolation offset
      within the step.
      Returns a hint to whether interpolation is required and whether to use the
      start or end of the motion_step (t=0 or 1).
      MOTIONSTEP_START = use start of motion_step (no interpolation)
      MOTIONSTEP_MID   = interpolate with motion_step_t
      MOTIONSTEP_END   = use end of motion_step (no interpolation)
  */
  int32_t   getMotionStep(const DoubleList& motion_times,
                          const TimeValue&  time,
                          uint32_t&         motion_step,
                          float&            motion_step_t);



  /*! Get a matrix, possibly interpolated, at 'time'.
      If 'time' is between keys then the matrix is linear interpolated using
      Mat4 lerp().
  */
  Mat4d     getMotionXformAt(const TimeValueList& motion_times,
                             const TimeValue&     time,
                             const Mat4dList&     motion_xforms);

  /*! Get two xforms at once, saving an additional motion step calculation.
      This is usually used when getting a matrix and its inverse at the same
      time. Both are linearly interpolated vs. deriving the first then
      inverting it.
  */
  void      getMotionXformsAt(const TimeValueList& motion_times,
                              const TimeValue&     time,
                              const Mat4dList&     motion_xformsA,
                              const Mat4dList&     motion_xformsB,
                              Mat4d&               xformA,
                              Mat4d&               xformB);




/*---------------------------------------------------------------------*/
/*---------------------------------------------------------------------*/
/*                   Inline Function Implementations                   */
/*---------------------------------------------------------------------*/
/*---------------------------------------------------------------------*/


inline uint32_t
getMotionStep(const TimeValueList& motion_times,
              const TimeValue&     time)
{
  const size_t nMotionSamples = motion_times.size();
  assert(nMotionSamples > 0);
  if (nMotionSamples == 1)
    return 0; // no motionblur

  // Find the motion step 'time' falls inside.
  // sample: 0             1             2             3
  //   step: |......0......|......1......|......2......|
  if (time < motion_times[0])
  {
    return 0; // before the first motion step
  }
  else
  {
    // Search the rest:
    for (size_t sample=1; sample < nMotionSamples; ++sample)
      if (time < motion_times[sample])
        return uint32_t(sample-1); // done!
    return uint32_t(nMotionSamples-1); // past last motion step
  }
}

inline int
getMotionStep(const TimeValueList& motion_times,
              const TimeValue&     time,
              uint32_t&            motion_step,
              float&               motion_step_t)
{
  const size_t nMotionSamples = motion_times.size();
  assert(nMotionSamples > 0);
  if (nMotionSamples == 1)
  {
    // No motionblur:
    motion_step   = 0;
    motion_step_t = 0.0f;
    return MOTIONSTEP_START;
  }

  // Find the motion step 'time' falls inside.
  // sample: 0             1             2             3
  //   step: |......0......|......1......|......2......|
  if (time <= motion_times[0])
  {
    // Before the first motion step:
    motion_step   = 0;
    motion_step_t = 0.0f;
    return MOTIONSTEP_START;
  }

  // Search the rest:
  for (size_t sample=1; sample < nMotionSamples; ++sample)
  {
    if (time < motion_times[sample])
    {
      motion_step   = uint32_t(sample-1);
      motion_step_t = float((time - motion_times[motion_step]) /
                              (motion_times[motion_step+1] - motion_times[motion_step]));
      return MOTIONSTEP_MID;
    }
  }

  // Past last motion step:
  motion_step   = uint32_t(nMotionSamples-2);
  motion_step_t = 1.0f;
  return MOTIONSTEP_END;
}

inline Mat4d
getMotionXformAt(const TimeValueList& motion_times,
                 const TimeValue&     time,
                 const Mat4dList&     motion_xforms)
{
  // Don't crash, just return identity():
  if (motion_xforms.size() == 0)
    return Mat4d::getIdentity();

  // Find the motion-step this shutter position falls inside:
  uint32_t      motion_step;
  float         motion_step_t;
  const int32_t motion_mode = getMotionStep(motion_times, time, motion_step, motion_step_t);
  assert(motion_step < motion_xforms.size());

  if (motion_mode == MOTIONSTEP_START)
    return motion_xforms[motion_step ];
  else if (motion_mode == MOTIONSTEP_END)
    return motion_xforms[motion_step+1];

  return lerpClamped(motion_xforms[motion_step], motion_xforms[motion_step+1], motion_step_t);
}

inline void
getMotionXformsAt(const TimeValueList& motion_times,
                  const TimeValue&     time,
                  const Mat4dList&     motion_xformsA,
                  const Mat4dList&     motion_xformsB,
                  Mat4d&               xformA,
                  Mat4d&               xformB)
{
  // Find the motion-step this shutter position falls inside:
  uint32_t      motion_step;
  float         motion_step_t;
  const int32_t motion_mode = getMotionStep(motion_times, time, motion_step, motion_step_t);

  if (motion_step >= motion_xformsA.size())
  {
    // Don't crash, just return identity():
    xformA = Mat4d::getIdentity();
  }
  else
  {
    if (motion_mode == MOTIONSTEP_START)
      xformA = motion_xformsA[motion_step ];
    else if (motion_mode == MOTIONSTEP_END)
      xformA = motion_xformsA[motion_step+1];

    xformA = lerpClamped(motion_xformsA[motion_step], motion_xformsA[motion_step+1], motion_step_t);
  }

  if (motion_step >= motion_xformsB.size())
  {
    // Don't crash, just return identity():
    xformB = Mat4d::getIdentity();
  }
  else
  {
    if (motion_mode == MOTIONSTEP_START)
      xformB = motion_xformsB[motion_step ];
    else if (motion_mode == MOTIONSTEP_END)
      xformB = motion_xformsB[motion_step+1];

    xformB = lerpClamped(motion_xformsB[motion_step], motion_xformsB[motion_step+1], motion_step_t);
  }
}


} // namespace fdk

#endif
