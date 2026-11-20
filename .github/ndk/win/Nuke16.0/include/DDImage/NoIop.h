// NoIop.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_NoIop_h
#define DDImage_NoIop_h

#include "DDImage/Iop.h"

namespace DD
{
  namespace Image
  {

    /*! \class DD::Image::NoIop

        This operator returns the input unchanged. You can use this as a
        base class for an Iop that does nothing to the image but changes
        some of the info_, such as the format, attributes, or set of
        channels. This avoids the need to write the pure virtual functions
        that a normal Iop needs.
     */
    class DDImage_API NoIop : public Iop
    {
    protected:

      /*! This does set_out_channels(Mask_None), so _request() and engine()
          will not be called.  If you subclass this you almost certainly
          need to override this function. You should call copy_info() and
          then modify the info in any way you want.
       */
      void _validate(bool) override;

      /*! Requests the same area and channels from the input. _request() will not
          be called unless you override _validate().
       */
      void _request(int x, int y, int r, int t, ChannelMask, int count) override;

      /*! Returns the input unchanged. _engine() will not be called unless you
          override _validate() to not call disable().
       */
      void engine(int y, int x, int r, ChannelMask channels, Row& row) override;
    public:
      NoIop(Node * node = nullptr) : Iop(node) {}

      /*! No knobs. */
      void knobs(Knob_Callback) override;

      /*! Enables transforms forwarding. */
      bool pass_transform() const override;

      /*! Returns getDefaultTransform(). This ensures the transforms applied
          prior to this Node are preserved and allows for correct behaviour
          regarding concatenation.
       */
      Transform* getTransform() override;
    };

  }
}

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
