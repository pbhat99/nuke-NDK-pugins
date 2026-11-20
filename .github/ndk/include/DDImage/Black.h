// Black.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_Black_h
#define DDImage_Black_h

#include "Iop.h"

namespace DD
{
  namespace Image
  {

    /*! \class DD::Image::Black

       This operator produces a completely empty and black image. The
       output bounding box is 1x1 and the output channels is Mask_None.

       Nuke uses this for all disconnected inputs, setting the formats
       from those set on the Root group.

     */
    class DDImage_API Black : public Iop
    {
      /*! Does nothing, the info_ was filled in by the constructor or by
          the knobs and never changes.
       */
      void _validate(bool) override;

      /*! This should not be called because _validate() sets info_.channels to
          zero. If it is called the base class implementation is used, which
          should cause a pure-virtual exception.
       */
      void engine(int y, int x, int r, ChannelMask, Row &) override;
    public:
      const char* Class() const override;
      const char* node_help() const override;

      /*! The constructor takes the format and full-size-format to return.
         If \a format is null then Format::None is used. If
         \a full_size_format is null then \a format is used.
       */
      Black(Node *, const Format * = nullptr, const Format * = nullptr);

      /*! The constructor takes the format and full-size-format to return.
         If \a format is null then Format::None is used. If
         \a full_size_format is null then \a format is used.
       */
      Black(const Format * = nullptr, const Format * = nullptr);
      void first_frame(int a) { info_.first_frame(a); }
      int first_frame() const { return info_.first_frame(); }
      void last_frame(int a) { info_.last_frame(a); }
      int last_frame() const { return info_.last_frame(); }

      OpHints opHints() const override;
      //! Dedicated test for special 'Black' empty Iop subclass.
      bool isBlackIop() const override { return true; }

      static const Description d;

      //------------------------------------------------------------------------
      // Deprecated virtual shading methods
      //------------------------------------------------------------------------
      mFnDeprecatedLegacy3DSystemInNuke14("fragment_shader(const VertexContext&, Pixel&) has been deprecated.")
        void fragment_shader(const VertexContext&, Pixel&) override;
      mFnDeprecatedLegacy3DSystemInNuke14("shade_GL(ViewerContext*, GeoInfo&) has been deprecated.")
        bool shade_GL(ViewerContext*, GeoInfo&) override;
      //------------------------------------------------------------------------
      //------------------------------------------------------------------------
    };
  }
}

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
