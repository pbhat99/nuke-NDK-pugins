// FrameRange.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_FrameRange_h
#define DDImage_FrameRange_h

#include "DDImage/Op.h"
#include "DDImage_API.h"

namespace DD
{
  namespace Image
  {
    class Knob_Closure;

    //! Class used to standardise frame range controls throughout NUKE
    class DDImage_API FrameRangeControls
    {
      double _frameRangeStart;               //!< Start frame
      double _frameRangeEnd;                 //!< End frame
      int _frameRangeType;
      Knob* _pFrameRangeStartKnob;
      Knob* _pFrameRangeEndKnob;
      Knob* _pFrameRangeTypeKnob;

      Knob* _pUnlimitedRangeScriptKnob;
      Knob* _pVisibleFromNowOnScriptKnob;
      Knob* _pVisibleCurrentFrameScriptKnob;
      Knob* _pVisibleUpTillNowScriptKnob;
      Knob* _pVisibleSpecificRangeScriptKnob;

    public:

      //! Enum used to specify what kind of frame range we have
      enum FrameRangeTypeEnum {
        eFrameRangeTypeAll = 0,
        eFrameRangeTypeFromStart,
        eFrameRangeTypeSingle,
        eFrameRangeTypeToEnd,
        eFrameRangeTypeFromStartToEnd
      };

      //! Constructor
      FrameRangeControls(double frameRangeStart = 1.0, double frameRangeEnd = 1.0, FrameRangeTypeEnum frameRangeType = eFrameRangeTypeSingle)
        : _frameRangeStart(frameRangeStart)
        , _frameRangeEnd(int(frameRangeEnd))
        , _frameRangeType(frameRangeType)
      {}

      //! The owner of this object must call this in their Op::knobs() function
      void knobs(
        Knob_Closure& f,
        bool          earlyStore = false,
        bool          shortMenu = false,
        const char*   frameRangeStartName = "framerange_start",
        const char*   frameRangeStartLabel = "start",
        const char*   frameRangeEndName = "framerange_end",
        const char*   frameRangeEndLabel = "end",
        const char*   frameRangeTypeName = "framerange_type",
        const char*   frameRangeTypeLabel = "type",
        bool          displayScriptKnobs = true,
        const char*   unlimitedRangeScript = "@Roto/UnlimitedLife;",
        const char*   visibleFromNowOnScript = "@Roto/VisibleFromNowOn;",
        const char*   visibleCurrentFrameScript = "@Roto/VisibleCurrentFrame;",
        const char*   visibleUpTillNowScript = "@Roto/VisibleUpTillNow;",
        const char*   visibleSpecificRangeScript = "@Roto/VisibleSpecificRange;"
        );

      //! The frame range start
      double getFrameRangeStart() const { return _frameRangeStart; }

      //! The frame range end
      double getFrameRangeEnd() const { return _frameRangeEnd; }

      //! The frame range type
      FrameRangeTypeEnum getFrameRangeType() const { return FrameRangeTypeEnum(_frameRangeType); }

      void setFrameRangeStart(double v) { _frameRangeStart = v; }
      void setFrameRangeEnd(double v) { _frameRangeEnd = v; }
      void setFrameRangeType(FrameRangeTypeEnum v)   { _frameRangeType = int(v); }

      Knob* getFrameRangeStartKnob() { return _pFrameRangeStartKnob; }
      Knob* getFrameRangeEndKnob() { return _pFrameRangeEndKnob; }
      Knob* getFrameRangeTypeKnob() { return _pFrameRangeTypeKnob; }

      Knob* getUnlimitedRangeScriptKnob() { return _pUnlimitedRangeScriptKnob; }
      Knob* getVisibleFromNowOnScriptKnob() { return _pVisibleFromNowOnScriptKnob; }
      Knob* getVisibleCurrentFrameScriptKnob() { return _pVisibleCurrentFrameScriptKnob; }
      Knob* getVisibleUpTillNowScriptKnob() { return _pVisibleUpTillNowScriptKnob; }
      Knob* getVisibleSpecificRangeScriptKnob() { return _pVisibleSpecificRangeScriptKnob; }

      std::vector<DD::Image::Knob*> getAllKnobs()
      {
        std::vector<DD::Image::Knob*> vec;
        vec.push_back(_pFrameRangeTypeKnob);
        vec.push_back(_pFrameRangeStartKnob);
        vec.push_back(_pFrameRangeEndKnob);
        vec.push_back(_pUnlimitedRangeScriptKnob);
        vec.push_back(_pVisibleFromNowOnScriptKnob);
        vec.push_back(_pVisibleCurrentFrameScriptKnob);
        vec.push_back(_pVisibleUpTillNowScriptKnob);
        vec.push_back(_pVisibleSpecificRangeScriptKnob);
        return vec;
      }

      void append(DD::Image::Hash& hash)
      {
        hash.append(_frameRangeStart);
        hash.append(_frameRangeEnd);
        hash.append(_frameRangeType);
      }
    };
  }
}

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
