// Format.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_Format_h
#define DDImage_Format_h

#include "Box.h"
#include "DDMath.h"

#include <iostream>

namespace DD
{
  namespace Image
  {

    class Hash;

    //! Describes the size and shape of an image.
    /*! \class DD::Image::Format

        This is the structure returned by Iop::info().format

        When an image is written the width and height of the format determine
        the size of the image file. Data outside this is clipped.

        The x,y,r,t rectangle is the "aperture" which is where the image
        data actually is, the rest is padding or borders. In most formats
        the box is equal to 0,0,width,height, this is used by film formats
        such as Academy where a large gap at the left is soundtrack, or
        for letterboxed images imbedded in larger 4:3 image formats. The
        primary use of this box is to convert coordinates between formats
        so that the user's idea of the center and edges of the image is
        preserved. Some operators also use this to clip drawing or to set
        the center of projection.

        The pixel_aspect is used to make the operators act "square" even if
        the data isn't. Filters use this to adjust their aspects, and
        transformations use this to make rotations work.

        Though the info() of an Iop will often match either the aperture
        box or the outside of the format, there is no requirement of
        this. The data in an image may be larger or smaller than the
        Format and does not have to intersect it.
     */
    class DDImage_API Format : public Box
    {
      const char* name_;
      int width_;
      int height_;
      double pa_;

    public:

      const char* name() const { return name_; }  //!< user-visible name of format
      void name(const char* n) { name_ = n; }     //!< change the name of the format

      int width() const { return width_; }    //!< Width of image file in pixels
      void width(int v) { width_ = v; }
      int height() const { return height_; }  //!< Height of image file in pixels
      void height(int v) { height_ = v; }
      double pixel_aspect() const { return pa_; } //!< Ratio of pixel_width/pixel_height
      void pixel_aspect(double v) { pa_ = v; }

      Format() {} /*!< Default constructor puts garbage in all fields. */

      Format(int w, int h, int x, int y, int r, int t, double pa = 1) {
        set(x, y, r, t);
        name_ = nullptr;
        width_ = w;
        height_ = h;
        pa_ = pa;
      }

      Format(int w, int h, double pa = 1) {
        set(0, 0, w, h);
        name_ = nullptr;
        width_ = w;
        height_ = h;
        pa_ = pa;
      }

      /*!
         Add this instance to a list of "named" formats. This list is searched
         by findExisting(). You should not delete this format after doing this.
         The name can be null in which case Nuke will generate a "640x480" style
         name for you.
       */
      void add(const char* name);

      /*!
         Finds the first format with the given size and pixel aspect.

         If \a pixel_aspect is zero then the first match with \e any pixel_aspect
         is returned. This is for the common case of files where the size is known
         but not the pixel aspect, it allows the size to choose the pixel aspect.
       */
      static Format* findExisting(int w, int h, double pixel_aspect = 0);

      /*! Return the i'th format known about. i must be between 0 and size(). */
      static Format* index(int i);

      /*! Return the number of formats known about. */
      static unsigned size();

      /*! The format fields of DD::Image::Iop are initialized to this value,
          which is a video (640x480) sized image. Most operators will change
          the initial value.
       */
      static Format None;
      bool is_none() const { return this == &None; } /*!< True if this is None */

      /*! Add this format to the hash. Does not include the format name. */
      void append(Hash&) const;

      /*! Transform a UV coordinate in the range 0-1 into the format's XY range */
      void from_uv(float u, float v, float& X, float& Y) const
      {
        X = (u * float(w())) + float(x());
        Y = (v * float(h())) + float(y());
      }
      
      /*! Back-transform an XY coordinate in the format's space into UV space. */
      void to_uv(float X, float Y, float& u, float& v) const
      {
        u = (X - float(x())) / float(w());
        v = (Y - float(y())) / float(h());
      }

      /*! Transform a screen centre based UV coordinate in the range 0-1 into the format's XY range */
      void from_uv_centre(float u, float v, int& X, int& Y, bool ignore_pixel_aspect = false) const
      {
        float const yz = ignore_pixel_aspect ? 1.f : float(pixel_aspect());
        // Round the coordinates to match pixels size of the format
        X = static_cast<int>(fast_rint(floor(u * w() * 0.5 + center_x() + 0.5)));
        Y = static_cast<int>(fast_rint(floor(v * w() * yz *0.5 + center_y() + 0.5)));
      }

      /*! Back-transform an XY coordinate in the format's space into screen centre based UV space. */
      void to_uv_centre(int X, int Y, float& u, float& v, bool ignore_pixel_aspect = false) const
      {
        float const yz = ignore_pixel_aspect ? 1.f : float(pixel_aspect());
        u = (X - center_x()) * 2.f / w();
        v = (Y - center_y()) * 2.f / yz / w();
      }
    };


    class FormatPair
    {
      const Format* fullSizeFormat_;
      const Format* format_;

      Format _formatStorage; // for use by Format_Knob, don't touch

    public:

      FormatPair() : fullSizeFormat_(&Format::None), format_(&Format::None) {}

      FormatPair(const Format* regular, const Format* full) : fullSizeFormat_(full), format_(regular) {}

      Format& formatStorage()
      {
        return _formatStorage;
      }

      const Format* fullSizeFormat() const { return fullSizeFormat_; }
      const Format* format() const { return format_; }
      void fullSizeFormat(const Format* f) { fullSizeFormat_ = f; }
      void format(const Format* f) { format_ = f; }
    };


    //! Print out terse info about the Format to a stream.
    DDImage_API  std::ostream& operator << (std::ostream&, const Format&);

  }
}

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
