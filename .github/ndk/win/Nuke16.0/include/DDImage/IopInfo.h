// IopInfo.h
// Copyright (c) 2011 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_IopInfo_h
#define DDImage_IopInfo_h

#include "DDImage/Box.h"
#include "DDImage/ChannelSet.h"
#include "DDImage/Format.h"
#include "DDImage/Info2D.h"

#ifdef __INTEL_COMPILER
#pragma warning(disable:444)  
#endif

namespace DD
{
  namespace Image
  {
      class DeepInfo;

      //This object derives from a Box* type, which doesn't have a virtual destructor. This
      //is why warning 444 has been disabled for the intel compiler at the top of this file - the important
      //message is to not delete via a Box* pointer, as the destructor will not
      //get called.

      /*! \class DD::Image::IopInfo

         This structure contains information that Iop::_validate() must
         calculate. Because this information is often copied unchanged
         or with tiny variations from the input operator, it is put into
         this structure so that it can be copied with a single assignment.

         This is a subclass of Box and that provides x,y,r,t
         values that describe the bounding box around all the interesting
         pixels (that is, the region of definition). It also provides a merge
         and intersect operators that are useful when _validate is combining
         the info from two inputs.
       */
      class DDImage_API IopInfo : public Info2D
      {
      private:

        ChannelSet    channels_; ///< channels present on output

        int black_outside_;

      public:

        ChannelSet& channels() { return channels_; }

        /*! \fn ChannelMask DD::Image::IopInfo::channels() const
           Which channels exist in the output of this Iop.
         */
        ChannelMask channels() const { return channels_; }
        void channels(ChannelSetInit v) { channels_ = v; }
        void channels(ChannelMask v) { channels_ = v; }
        void channels(Channel v) { channels_ = v; }

        /*! \fn bool DD::Image::IopInfo::black_outside() const

           True indicates that a 1-pixel border all around the edge of the
           bounding box is zero in all channels. Operators are expected to
           set this correctly to match their output.

           Many operators can take advantage of this to avoid adding their
           own black edge and thus avoid increasing the bounding box size.
         */
        bool black_outside() const { return black_outside_ != 0; }
        void black_outside(bool v) { black_outside_ = v; }
        void turn_on(ChannelSetInit m) { channels_ += (m); }

        /*! \fn void DD::Image::IopInfo::turn_on(ChannelMask m)
           Same as channels(turn_on(channels(), m)).
         */
        void turn_on(ChannelMask m) { channels_ += (m); }
        
        /*! \fn void DD::Image::IopInfo::turn_on(Channel c)
           Same as channels(turn_on(channels(), c)).
         */
        void turn_on(Channel c) { channels_ += (c); }
        void turn_off(ChannelSetInit m) { channels_ -= (m); }

        /*! \fn void DD::Image::IopInfo::turn_off(ChannelMask m)
           Same as channels(turn_off(channels(), m)).
         */
        void turn_off(ChannelMask m) { channels_ -= (m); }

        /*! \fn void DD::Image::IopInfo::turn_off(Channel c)
           Same as channels(turn_off(channels(), c)).
         */
        void turn_off(Channel c) { channels_ -= (c); }
        bool is_constant() const { return !channels_ || Box::is_constant(); }

        /*!
           Expand the bounding box by dx,dy,dr,dt but do not expand it outside
           the box determined by 0,0,format().width(),format().height() if it
           is at or inside this. This is used by filter operations, and stops
           them from expanding their data outside the current format. This
           clipping is usually what is expected and avoids keeping unnecessary
           data in the cache buffers, and avoids random-sized caches which
           fragment memory.

           If bounding box is 1 pixel wide in either direction it leaves it
           unchanged, since this indicates a constant value.
         */
        void clipmove(int dx, int dy, int dr, int dt);

        
        /// copy over the subset of DeepInfo and IopInfo onto this
        void operator=(const DeepInfo& deepInfo);

      };

      //! Print out terse info about the IopInfo to a stream.
      DDImage_API  std::ostream& operator << (std::ostream&, const IopInfo&);

  }
}

#ifdef __INTEL_COMPILER
#pragma warning(default:444)  
#endif

#endif
