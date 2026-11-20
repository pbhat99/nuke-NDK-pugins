// Blur.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_Blur_h
#define DDImage_Blur_h

#include "DDImage/Iop.h"
#include "DDImage/DownRez.h"
#include "DDImage/VConvolve.h"
#include "DDImage/HConvolve.h"
#include "DDImage/UpRez.h"

namespace DD
{
  namespace Image
  {

    /*! \class DD::Image::Blur
        The "fast blur".  Does a 2-pass convolution with a gaussian filter.
        If the amount of blur is large enough it uses DownRezIop and UpRezIop
        to scale it down so a smaller filter can be used.

        Odd integer sizes when the filter is set to BOX make a box filter
        of exactly that size. Non-integer values cause the outer edge of
        the next larger box to gradually fade in. The filters used when
        the type is not BOX are larger and chosen to result in approximately
        the same amount of blurring as when box is used.
     */
    class DDImage_API Blur : public Iop
    {

      DownRez downrez;
      VConvolve vpass;
      HConvolve hpass;
      UpRez     uprez;
      bool downrezonly;
      bool rezonly;
      bool croptoformat;

    protected:

      double size_x_, size_y_; // scale size
      int quality_;
      int type_;
      double falloff;

    public:

      Blur(Node * node);
      ~Blur() override;

      void _validate(bool) override;
      void _request(int x, int y, int r, int t, ChannelMask, int count) override;
      void engine(int y, int x, int r, ChannelMask, Row &) override;
      void _close() override;

      double size() const { return size_x_; }
      void size(double v) { size_x_ = size_y_ = v; }
      void size(double v, double w) { size_x_ = v;
                                      size_y_ = w; }
      double size_x() const { return size_x_; }
      void size_x(double v) { size_x_ = v; }
      double size_y() const { return size_y_; }
      void size_y(double v) { size_y_ = v; }

      enum { BOX = 0, TRIANGLE, QUADRATIC, GAUSSIAN };
      int type() const { return type_; }
      void type(int t) { type_ = t; }

      /*! Fill in an array with the coefficients of a filter of the given
          \a type (one of BOX, TRIANGLE, QUADRATIC, or
          GAUSSIAN, possibly others will be added in the future).
          The \a size is how big the array is, if this is smaller than
          what filter_size() returned the filter has the ends truncated
          and is renormalized so the portion adds to one. If it is
          bigger the rest if filled with zero. \a size must be an
          odd number. \a diameter controls the filter width, for box filters
          integers indicate a box of exactly that size, all other numbers
          and filter types set the coefficients to resemble and smoothly
          interpolate those box filters.
       */
      static void make_filter(int type, float* array, int size, double diameter, double gamma = 1.0);

      /*! Return the recommended size of an array to pass to
          make_filter(). The returned value is always odd, so that the
          filter has a center value.  */
      static int filter_size(int type, double diameter);

      /*! This array may be passed to Enumeration_knob if you want to make
          a control where the user selects one of the filter types that
          the Blur can generate with filter_size() and make_filter().
       */
      static const char* const filter_names[];

      void knobs(Knob_Callback) override;
      const char* Class() const override;
      const char* node_help() const override;
      static const Description d;
    };

  }
}

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
