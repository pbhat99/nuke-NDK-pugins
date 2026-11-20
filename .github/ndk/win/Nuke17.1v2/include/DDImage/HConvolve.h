// HConvolve.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef HConvolveIop_H
#define HConvolveIop_H

#include "DDImage/Iop.h"

namespace DD
{
  namespace Image
  {

    /*! \class DD::Image::HConvolve

       Convolves (filters) the input image by multiplying it by a horizontal
       vector of weights.  The vector is assumed to be normalized and should
       be an odd size to not shift the image.

       This operator may be combined with VConvolve to produce the
       equivalent of a Convolve operator with a NxM rectangular matrix,
       as long as that matrix is the result of multiplying a 1xM matrix
       by a Nx1 matrix. The cache is best used if you put the HConvolve
       \e after the VConvolve.

       Many useful low-pass filters can be represented this way, or
       approximated. This is thus used by the Blur operator. High-pass
       filters can also be approximated by subtracting the result of a
       low-pass filtering from the original picture, Sharpen uses this
       for that.
     */
    class DDImage_API HConvolve : public Iop
    {
      int size_;
      float* array_;
    public:
      void _validate(bool) override;
      void _request(int x, int y, int r, int t, ChannelMask, int count) override;
      void engine(int y, int x, int r, ChannelMask, Row &) override;
      HConvolve(Node * node) : Iop(node) {}

      int size() const { return size_; }
      float* vector() const { return array_; }
      void vector(int s, float* v) { size_ = s;
                                     array_ = v; }

      const char* Class() const override;
      const char* node_help() const override;
    };

  }
}

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
