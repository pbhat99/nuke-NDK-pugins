// PixelIop.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_PixelIop_h
#define DDImage_PixelIop_h

#include "DDImage/Iop.h"

namespace DD
{
  namespace Image
  {

    /*! \class DD::Image::PixelIop
        This is a base class for a very common type of operation, where each
        output pixel depends only on the input pixel at the same location,
        the x,y position, and on the Iop's controls. This includes all
        drawing operations, color correctors, merge, most key pullers, etc.

        To subclass PixelIop you must write in_channels(), and
        pixel_engine(). You also need to call set_out_channels() if your
        op does not modify all channels (either call it in the constructor
        or in _validate()).

        NukeWrapper will call a subclass of PixelIop much more efficiently
        than it will call a normal Iop.
     */
    class DDImage_API PixelIop : public Iop
    {
    protected:

      /*! engine() gets all the channels that are not in out_channels(), and
          all the channels needed by in_channels(), from the input. It then
          calls pixel_engine() to have it modify the data.

          Don't replace engine(), as NukeWrapper will bypass this anyway.
       */
      void engine(int y, int x, int r, ChannelMask, Row &) override;
    public:

      /*!
         engine() calls this to calculate the result. This must calculate
         all the pixels between horizontal positions \a x and \a r, for
         vertical line \a y, for all channels in \a mask, and write them to \a out.

         \a mask will always be a subset of out_channels().

         \a in contains the channels returned by in_channels(0,mask) in the
         range \a x through \a r-1 of line \a y of input0(). It may contain
         additional channels and pixels but you should ignore them. You can
         use at() or get() or any other methods to read pixels that are not
         in this row, or to get pixels from other inputs.

         \a in and \a out may be the same instance of Row. This means that
         writing to out.writable(n)[x] may overwrite in[n][x], and that
         out.writable(n) or out.erase(n) may change the pointer returned by
         in[n]. The proper way to write your algorithim is something like this:

         \code
         // Copy the pointers for each input channel first:
         const float* IN1 = in[inchannel1];
         const float* IN2 = in[inchannel2];
         // Now allocate the output pointers:
         float* OUT1 = out.writable(outchannel1);
         float* OUT2 = out.writable(outchannel2);
         // Now loop for every pixel:
         for (int X = x; X < r; X++) {
          // Copy the source pixels to local memory:
          const float A = IN1[X];
          const float B = IN2[X];
          // Now store the result:
          OUT1[X] = function1(A,B);
          OUT2[X] = function2(A,B);
         }
         \endcode

         If you wish to copy a channel unchanged from input to output, use
         the following code, which may allocate less memory than calling
         out.writable():

         \code
         out.copy(out_channel, in, in_channel, x, r);
         \endcode

       */
      virtual void pixel_engine(const Row &in, int y, int x, int r, ChannelMask, Row &) = 0;
      PixelIop(Node * node) : Iop(node) {}
      ~PixelIop() override {}
      unsigned node_color() const override;

      /*!
         Return hints to control when and how this op will be evaluated by the top-down system.
      */
      OpHints opHints() const override;
    };

  }
}

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
