#ifndef DDImage_GeneralInfo_h
#define DDImage_GeneralInfo_h

#include "DDImage_API.h"

namespace DD
{
  namespace Image
  {

    typedef int FrameRange[2];  

    //! Base class for defining a time bounded region
    class DDImage_API GeneralInfo
    {
      int first_frame_;
      int last_frame_;

    public:
      GeneralInfo() : first_frame_(1), last_frame_(1) { }

      /// get the first frame in the range
      int firstFrame() const
      {
        return first_frame_;
      }

      /// get the first frame in the range

      /*! \fn int DD::Image::GeneralInfo::first_frame() const
         First frame in the range of the clip this Iop produces.
       */
      int first_frame() const
      {
        return first_frame_;
      }

      /// get the last frame in the range
      int lastFrame() const
      {
        return last_frame_;
      }

      /// get the last frame in the range

      /*! \fn int DD::Image::GeneralInfo::last_frame() const
         Last frame in the range of the clip this Iop produces.
       */
      int last_frame() const
      {
        return last_frame_;
      }

      /// set the first frame in the range
      void setFirstFrame(int v)
      {
        first_frame_ = v;
      }

      /// set the last frame in the range
      void setLastFrame(int v)
      {
        last_frame_ = v;
      }

      /// set the first frame in the range
      void first_frame(int v)
      {
        first_frame_ = v;
      }

      /// set the last frame in the range
      void last_frame(int v)
      {
        last_frame_ = v;
      }

      void setFrameRange(FrameRange frameRange)
      {
        first_frame_ = frameRange[0];
        last_frame_ = frameRange[1];
      }
    };

    class DDImage_API GeneralInfoProvider
    {
    public:
      virtual ~GeneralInfoProvider();
      //! Returns the GeneralInfo with the first and last frames.
      virtual const GeneralInfo& generalInfo() const = 0;
    };
  }
}

#endif
