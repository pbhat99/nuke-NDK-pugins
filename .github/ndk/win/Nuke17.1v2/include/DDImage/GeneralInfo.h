#ifndef DDImage_GeneralInfo_h
#define DDImage_GeneralInfo_h

#include "DDImage/DDImage_API.h"

namespace DD
{
  namespace Image
  {
    class Op;
    typedef int FrameRange[2];  

    //! Base class for defining a time bounded region
    class DDImage_API GeneralInfo
    {
    public:
      //! Copy GeneralInfo values from another.
      //! If all GeneralInfo params are desired use this method vs. copying individual parameters
      //! so future additions to this class are automatically handled.
      void copyGeneralInfo(const GeneralInfo& from);


      //! First frame in the range.
      int firstFrame() const { return _firstFrame; }

      //! Last frame in the range.
      int lastFrame() const { return _lastFrame; }

      //! Set the first frame in the range
      void setFirstFrame(int first) { _firstFrame = first; }

      //! Set the last frame in the range
      void setLastFrame(int last) { _lastFrame = last; }

      //! Set both first and last frame.
      void setFrameRange(int first, int last)
      {
        _firstFrame = first;
        _lastFrame = last;
      }

      //! Set first and last from a FrameRange array.
      void setFrameRange(const FrameRange& frameRange) { setFrameRange(frameRange[0], frameRange[1]); }


      //--------------------------------------------------------------
      // Older methods, please use new methods above.

      //! first_frame() has been deprecated, use firstFrame() instead.
      int first_frame() const { return firstFrame(); }
      //! last_frame() has been deprecated, use lastFrame() instead.
      int last_frame() const { return lastFrame(); }
      //! first_frame(int) has been deprecated, use setFirstFrame(int) instead.
      void first_frame(int v) { setFirstFrame(v); }
      //! last_frame(int) has been deprecated, use setLastFrame(int) instead.
      void last_frame(int v) { setLastFrame(v); }

    private:
      int _firstFrame = 1;
      int _lastFrame = 1;

    };

    class DDImage_API GeneralInfoProvider
    {
    public:
      virtual ~GeneralInfoProvider();

      //! Returns the GeneralInfo.
      //!
      //! Warning! This will only return the local info for the provider Op which may not include
      //! modifications applied by non-Op nodes like FrameRange and TimeOffset - use with caution.
      //!
      //! Must implement.
      virtual const GeneralInfo& generalInfo() const = 0;
    };
  }
}

#endif
