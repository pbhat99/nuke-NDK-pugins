// ColorLookup.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef ColorLookup_H
#define ColorLookup_H

#include "DDImage/PixelIop.h"

namespace DD
{
  namespace Image
  {

    /*! \class DD::Image::ColorLookup

        This is a base class that implements a lookup from input color
        values to output values. Values between 0 and range() are looked
        up in a 1282-entry lookup table indexed by the top 16 bits of the
        floating-point number, and linearly interpolated between those
        values. Values outside that range are linearly interpolated from
        the end entries in the table.

        To subclass this you must set range() in your _validate(), and then
        call the base class _validate(). You must also implement lookup()
        which will be called many times to fill in the tables.

        You can also implement mono() if all the channels use the same
        lookup table.

        into output ones.  The actual implementation calls this during
        _request() and uses it to set up a fast lookup table that is
        based on the top 16 bits of the floating point numbers. The
        subclass _validate() should also call set_out_channels() to
        the set that is not the identity, to avoid building tables for
        unchanging channels.

        The resulting lookup curve is made of many linear segments and
        thus large second derivatives in the lookup function can produce
        mach banding.  The range outside 0..1 is linearly extrapolated
        from the first and last segments.
     */
    class DDImage_API ColorLookup : public PixelIop
    {
    public:
      ColorLookup(Node* node);
      ~ColorLookup() override;
      
      void _validate(bool) override;
      void _open() override;
      void pixel_engine(const Row& in, int y, int x, int r, ChannelMask channels, Row& out) override;

      /*! Returns channels unchanged, since it only needs the same channel to produce the output. */
      void in_channels(int input, ChannelSet& channels) const override;

      /*! \fn float ColorLookup::lookup(int z, float value) const
          Subclasses must implement this function. It returns the what
          \a value in a channel with DD::Image::colourIndex()==z
          should turn into.

          Input values between zero and one are looked up in a table
          that is built during _open() by calling this. Values outside
          the zero to one range will linearly interpolate the ends
          of this table.
       */
      virtual float lookup(int colour, float value) const = 0;

    protected:
      static constexpr int kBottom = 0x3a80;    // Top 16 bits of 1.0/1024
      static constexpr int kTop = 0x3f80;       // Top 16 bits of 1.0
      static constexpr int kEntries = (kTop - kBottom + 2);

      enum { NUMTABLES = 4 };

    private:
      /*! The tables consist of both slope and offset values which define a linear
          function modelling a given channels curve in the ColorLookup LookupCurves. The first
          half of the table holds the offset values, while the second half holds the values
          of the curve's gradient. The first entry is for 0.0, followed by the values for
          each floating point number from kBottom to kTop.
      */
      float* tables[NUMTABLES];
    };
  }
}

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
