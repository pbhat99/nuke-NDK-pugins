// Shutter.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_Shutter_h
#define DDImage_Shutter_h

#include "DDImage/Op.h"
#include "DDImage_API.h"

namespace DD
{
  namespace Image
  {
    class Knob_Closure;

    /** Class used to standardise shutter controls throughout NUKE */
    class DDImage_API ShutterControls
    {
      double _duration;        ///< shutter time
      int   _offset;           ///< choice param with four options, centred, start, stop, custom
      double _customOffset;    ///< shutter offset to use if _shutterOffset is set to custom
      Knob* _durationKnob;
      Knob* _offsetKnob;
      Knob* _customOffsetKnob;

    public:

      /** Enum used to specify what kind of shutter offset we have */
      enum OffsetEnum {
        eCentredOffset,
        eStartOffset,
        eEndOffset,
        eCustomOffset
      };

      /// Constructor
      ShutterControls(double duration = 0.5, OffsetEnum offset = eStartOffset, double customOffset = 0.0)
        : _duration(duration)
        , _offset(int(offset))
        , _customOffset(customOffset)
      {}

      /** The owner of this object must call this in their Op::knobs() function */
      void knobs(
        Knob_Closure& f,
        bool          earlyStore = false,
        const char*   durationName = "shutter",
        const char*   offsetTypeName = "shutteroffset",
        const char*   customOffsetName = "shuttercustomoffset"
        );

      /** The duration */
      double getDuration() const { return _duration; }

      /** Offset to the start of the shutter period from integer frame number */
      double calcOffset() const;

      /** The type of offset. */
      OffsetEnum getOffset() const { return OffsetEnum(_offset); }

      /** The Offset if the enum is set to eCustomOffset */
      double getCustomOffset() const { return _customOffset; }

      void setDuration(double v)     { _duration = v; }
      void setOffset(OffsetEnum v)   { _offset = int(v); }
      void setCustomOffset(double v) { _customOffset = v; }

      Knob* getDurationKnob() { return _durationKnob; }
      Knob* getOffsetKnob() { return _offsetKnob; }
      Knob* getCustomOffsetKnob() { return _customOffsetKnob; }

      void append(DD::Image::Hash& hash)
      {
        hash.append(_duration);
        hash.append(_offset);
        hash.append(_customOffset);
      }
    };
  }
}

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
