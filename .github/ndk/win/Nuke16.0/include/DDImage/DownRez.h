// DownRez.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DownRezIop_H
#define DownRezIop_H

#include "DDImage/Iop.h"

namespace DD
{
  namespace Image
  {

    /*! \class DD::Image::DownRez
        Scale down the image by an integer factor, using a box filter
        Much faster and very predictable results so this can be used by
        the fast blur.
     */
    class DDImage_API DownRez : public Iop
    {
      int factor_x_, factor_y_; // scale factor
    public:
      void _validate(bool) override;

      /*! Requests the enlarged area. */
      void _request(int x, int y, int r, int t, ChannelMask, int count) override;
      void engine(int y, int x, int r, ChannelMask, Row &) override;
      DownRez(Node * node = nullptr) : Iop(node) { factor_x_ = factor_y_ = 1; }

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
