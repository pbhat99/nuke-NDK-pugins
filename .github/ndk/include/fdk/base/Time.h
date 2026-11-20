// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FDK_BASE_TIME_H
#define FDK_BASE_TIME_H

#include "fdk/api.h"

#include <limits> // for quiet_NaN
#include <cmath>  // for isnan()
#include <vector>
#include <set>
#include <iostream>


//-------------------------------------------------------------------------
//
// These helper types/functions are intended to be compatible with the
// UsdTimeCode::Default() time value, but are intended to be generally
// useful without being dependent on the Usd libs.
//
// TODO: I don't think these types need to be classes as we can keep the
// time methods in the global fdk namespace for ease of use, or at the
// most wrap them in a 'Time' namespace.
//------------------------------------------------------------------------


namespace fdk
{

  //--------------------------------------------------------------------------------------------------
  //--------------------------------------------------------------------------------------------------


  /*! TimeValues are fractional seconds where 1.0 = one second, 2.0 =
      2 seconds, 1.5 = one-and-a-half seconds, etc.

      A TimeValue is an abstract, absolute time reference that's unlocked from
      the frame rate that most animation keys are defined on. ie a key at
      frame 105 in a 24 fps project is at a different absolute time value
      than frame 105 in a 48 fps project (4.375 seconds vs. 2.1875 seconds.)

      A TimeValue can be set to a 'default' or 'no-time' value, typcially used to
      indicate the object the TimeValue relates to is a single sample, or has
      no animation.
  */
  using TimeValue     = double;
  using TimeValueSet  = std::set<TimeValue>;
  using TimeValueList = std::vector<TimeValue>;


  /*! A 'not-animated' time value (either frame or time) represents 'no time',
      eg a non-animated value or keyframe.

      This is the same as doing 'time = std::numeric_limits<double>::quiet_NaN()'.
  */
  FDK_API
  inline TimeValue   notAnimatedTimeValue() { return std::numeric_limits<TimeValue>::quiet_NaN(); }

  FDK_API
  inline TimeValue   defaultTimeValue()     { return notAnimatedTimeValue(); }

  FDK_API
  inline TimeValue   earliestTimeValue()    { return std::numeric_limits<TimeValue>::lowest(); }



  //--------------------------------------------------------------------------------------------------
  //--------------------------------------------------------------------------------------------------


  /*! FrameValues are absolute frame numbers dependent on a companion
      frames-per-second rate (24.0 fps, 30.0 fps, 48.0 fps, etc.)

      The only point when a TimeValue and a FrameValue are ever equal is
      at exactly 0.0.

      Conversion to/from a TimeValue is trivial as you just need the
      frames-per-second rate:
          TimeValue  = FrameValue / frameRate;
          FrameValue = TimeValue * frameRate;

      A FrameValue can be set to a 'default' or 'no-time' value, typcially used to
      indicate the object the FrameValue relates to is a single sample, or has
      no animation.
  */
  using FrameValue     = double;
  using FrameValueSet  = std::set<FrameValue>;
  using FrameValueList = std::vector<FrameValue>;


  //! Same as notAnimatedTimeValue().
  FDK_API
  inline FrameValue   notAnimatedFrameValue() { return std::numeric_limits<TimeValue>::quiet_NaN(); }

  //! Same as defaultTimeValue().
  FDK_API
  inline FrameValue   defaultFrameValue()     { return defaultTimeValue(); }

  FDK_API
  inline FrameValue   earliestFrameValue()    { return std::numeric_limits<FrameValue>::lowest(); }

  FDK_API
  inline bool         isDefaultTimeValue(const TimeValue& time) { return std::isnan(time); }
  FDK_API
  inline bool         isDefaultFrameValue(const FrameValue& frame) { return std::isnan(frame); }
  FDK_API
  inline bool         isEarliestTimeValue(const TimeValue& time) { return (time == std::numeric_limits<TimeValue>::lowest()); }
  FDK_API
  inline bool         isEarliestFrameValue(const FrameValue& frame) { return (frame == std::numeric_limits<FrameValue>::lowest()); }


  //--------------------------------------------------------------------------------------------------
  //--------------------------------------------------------------------------------------------------


  //! Convert a FrameValue to an absolute TimeValue. Requires frames-per-second.
  FDK_API
  inline TimeValue    getAbsoluteTime(const FrameValue& frame,
                                      double            frames_per_second=24.0) { return (frame / frames_per_second); }

  //! Convert an absolute TimeValue to a FrameValue. Requires frames-per-second.
  FDK_API
  inline FrameValue   getFrameValue(const TimeValue& time,
                                    double           frames_per_second=24.0)    { return (time * frames_per_second); }

  FDK_API
  inline void   setNotAnimated(TimeValue& time) { time = notAnimatedTimeValue(); }

  FDK_API
  inline void   setNotAnimated(TimeValueList& times) { times.resize(1); times[0] = notAnimatedTimeValue(); }


  //--------------------------------------------------------------------------------------------------


  /*! Does the time value represent 'no time', i.e. a non-animated value?
      This is the same as doing std::isnan(time).
  */
  FDK_API
  inline bool   isAnimated(const TimeValue& time)    { return !std::isnan(time); }

  FDK_API
  inline bool   isNotAnimated(const TimeValue& time) { return !isAnimated(time); }


  /*! If the TimeValue array is non-empty and the first entry isDefaultTimeValue()
      then the entire array represents a single default value stored at array[0].
      This is used in conjunction with an associated value array of matching size.
  */
  FDK_API
  inline bool   isAnimated(const TimeValueList& times)    { return (times.size() > 0 && isAnimated(times[0])); }

  FDK_API
  inline bool   isNotAnimated(const TimeValueList& times) { return !isAnimated(times); }



  //--------------------------------------------------------------------------------

#if 1
  FDK_API void printTimes(std::ostream& o, const TimeValueSet& times); // also works for FrameValueSet
  FDK_API void printTimes(std::ostream& o, const TimeValueList& times); // also works for FrameValueList
#else
  FDK_API std::ostream& operator << (std::ostream& o, const TimeValueSet&); // also works for FrameValueSet
  FDK_API std::ostream& operator << (std::ostream& o, const TimeValueList&); // also works for FrameValueList
#endif


} // namespace fdk

#endif
