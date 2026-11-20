// Convolve.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_Convolve_h
#define DDImage_Convolve_h

#include "DDImage/Iop.h"

namespace DD
{
  namespace Image
  {

    /*! A variable-width and height array.  You must use this data
       structure if you want to use Array_knob in Nuke to store values
       into your Iop. */
    struct DDImage_API ConvolveArray
    {
      int width, height;
      float* array;
      const float* operator[](int n) const { return array + n * width; }
      /*! Set the value to a const array by casting-away const: */
      void set(int h, int w, const float* v) { height = h;
                                               width = w;
                                               array = (float*)v; }
      ConvolveArray() : width(0), height(0), array(nullptr) {}
    };

    /*! \class DD::Image::Convolve

        Convolves (filters) the input with a matrix vector of weights.
        The matrix is placed so it's middle pixel is at 0,0 (If the matrix
        is not odd in size this will also shift the output up or left by
        1/2 pixel).

        For the Nuke UI, this goes through the matrix "upside down".  This
        is so the matrix the user types into the UI is in the same orientation
        as the pixels it affects.
     */
    class DDImage_API Convolve : public Iop
    {
      ConvolveArray array_;
      bool normalize_;
      float normalize_factor;
    public:
      void _validate(bool) override;
      void _request(int x, int y, int r, int t, ChannelMask, int count) override;
      void engine(int y, int x, int r, ChannelMask, Row &) override;
      Convolve(Node * node) : Iop(node), normalize_(false) {}

      int width() const { return array_.width; }
      int height() const { return array_.height; }
      float* array() const { return array_.array; }
      const float* operator[](int n) const { return array_.array + n * array_.width; }
      void array(int h, int w, const float* v) { array_.set(h, w, v); }

      bool normalize() const { return normalize_; }
      void normalize(bool b) { normalize_ = b; }

      void knobs(Knob_Callback) override;
      /*! Command name is "Matrix" rather than "Convolve" */
      const char* Class() const override;
      const char* node_help() const override;
      static const Description d;
    };

  }
}

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
