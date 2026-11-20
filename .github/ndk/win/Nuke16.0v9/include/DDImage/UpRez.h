// UpRez.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef UpRezIop_H
#define UpRezIop_H

#include "DDImage/MultiTileIop.h"

namespace DD
{
  namespace Image
  {

    /** \class DD::Image::UpRez

        Scale up the image by an integer factor and using bilinear
        interpolation.  This is fast and predictable and will invert
        DownRez without shifting the image any, so the pair can be used
        by the fast blur.
     */
    class DDImage_API UpRez : public MultiTileIop
    {
      int factor_x_, factor_y_; // scale factor
    public:

      /*! _validate() sets the output size to include the 1/2 of an unscaled
          pixel that will be filled outside the image.
       */
      void _validate(bool) override;

      /*! request() has to request an extra pixel outside the area. */
      void _request(int x, int y, int r, int t, ChannelMask, int count) override;

      Iop* inputToRead() const override;

      template<class TileType> inline void doEngine(int y, int x, int r, ChannelMask channels, Row& row);

      mFnDDImageMultiTileIop_DeclareFunctions_engine(int y, int x, int r, ChannelMask m, Row& row);

      UpRez(Node * node) : MultiTileIop(node) { factor_x_ = factor_y_ = 1; }

      int factor() const { return factor_x_; }
      void factor(int v) { factor_x_ = factor_y_ = v; }
      void factor(int v, int w) { factor_x_ = v;
                                  factor_y_ = w; }
      int factor_x() const { return factor_x_; }
      void factor_x(int v) { factor_x_ = v; }
      int factor_y() const { return factor_y_; }
      void factor_y(int v) { factor_y_ = v; }

      const char* Class() const override;
      const char* node_help() const override;
    };

  }
}

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
