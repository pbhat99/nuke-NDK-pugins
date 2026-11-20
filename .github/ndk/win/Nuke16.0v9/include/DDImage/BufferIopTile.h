// BufferIopTile.h
// Copyright (c) 2017 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_BufferIopTile_H
#define DDImage_BufferIopTile_H

#include "DDImage/Box.h"
#include "DDImage/BufferIop.h"
#include "DDImage/ChannelSet.h"
#include "DDImage/Iop.h"


namespace DD {
  namespace Image {

    class InterestRatchet;

    /**
     * BufferIopTile is a Tile-like API for access to image data, that can use both the
     * traditional row-based Tile API and can access ImagePlane data stored inside
     * a BufferIop.
     *
     * Accessing the Tile will return data from from inside the ImagePlane, to avoid
     * this data being duplicated inside the row cache. Since BufferIops are utilised
     * to provide top-down execution, it is assumed that the ImagePlane already
     * contains data when the Tile is created. If this is not the case, the returned
     * Tile will not be valid. If there is any doubt about whether the data is available,
     * you should check the valid() state before attempting to access the tile.
     *
     * Unlike Interest/Tile this isn't a Box.  Some Box methods are provided
     * for source-compatibility, but it's probably best to use getBox() to get
     * the box instead.
     */

    class DDImage_API BufferIopTile
    {
    public:

      /**
       * Construct.  All image data will be available after this call, unless the
       * BufferIop did not contain an image plane with valid data. Please check
       * the valid() state after construction and do not use the pixel data if this is set.
       *
       * These constructors are provided for compatibility with other Tiles. Note that the
       * values of forceUnpacked, mt and InterestRatchet will be ignored for this tile type.
       *
       */
      BufferIopTile(Iop& input, const Box& box, const ChannelSet& chans, bool forceUnpacked = false, bool mt = false, InterestRatchet* ir = nullptr);
      BufferIopTile(Iop& input, int x, int y, int r, int t, const ChannelSet& chans, bool mt = false, InterestRatchet* ir = nullptr);
      BufferIopTile(Iop& input, const ChannelSet& chans, bool mt = false, InterestRatchet* ir = nullptr);

      /**
       * destructor
       */
      ~BufferIopTile();

      /*! helper class used for return value of operator[] */
      class LinePointers
      {
      public:
        LinePointers(const BufferIopTile& tile, Channel z) : _tile(tile), _z(z) {}

        /**
         * return a pointer to the 'start' of an individual row (ie the
         * position at which x = 0, even if this is not in itself
         * dereferencable
         */
        const float* operator[](int y) const;

      private:
        const BufferIopTile& _tile;
        const Channel _z;
      };

      /*! pointer to within an individual row */
      typedef const float* RowPtr;


      /** accessor for the box's x value (provided for
          compatibility with DD::Image::RowCacheTile) */
      int x() const
      {
        return _box.x();
      }

      /** accessor for the box's y value (provided for
          compatibility with DD::Image::RowCacheTile) */
      int y() const
      {
        return _box.y();
      }

      /** accessor for the box's r value (provided for
          compatibility with DD::Image::RowCacheTile) */
      int r() const
      {
        return _box.r();
      }

      /** accessor for the box's t value (provided for
          compatibility with DD::Image::RowCacheTile) */
      int t() const
      {
        return _box.t();
      }

      /** accessor for the box's width (provided for
          compatibility with DD::Image::RowCacheTile) */
      int w() const
      {
        return _box.w();
      }

      /** accessor for the box's height (provided for
          compatibility with DD::Image::RowCacheTile) */
      int h() const
      {
        return _box.h();
      }

      /** clamp a y value to within the box (provided for
          compatibility with DD::Image::RowCacheTile) */
      int clampy(int y) const
      {
        return _box.clampy(y);
      }

      /** clamp a x value to within the box (provided for
          compatibility with DD::Image::RowCacheTile) */
      int clampx(int x) const
      {
        return _box.clampx(x);
      }

      /** cast operator for the box (provided for
          compatibility with DD::Image::RowCacheTile) */
      operator const Box&() const
      {
        return _box;
      }

      /** return the box for the tile */
      const DD::Image::Box& box() const
      {
        return _box;
      }

      /** get the channels for the tile  */
      const ChannelSet& channels() const
      {
        return _channels;
      }

      /**
       * whether the BufferIopTile is accessible, or could not be created
       * (e.g. because the BufferIop's image plane has not been set yet)
       */
      bool valid() const { return _valid; }

      /**
       * get the data at a particular pixel. Clamps to the tile bounds.
       */
      float at(int x, int y, Channel z) const;

      /**
       * given a channel, return something that acts like an array
       * of pointers to all the loaded lines.
       */
      LinePointers operator[](Channel z) const;

      /**
       * return an empty row pointer
       */
      static const float* NilRow() { return nullptr; }

      /**
       * is this channel/row pair 'zero' (i.e. is it mapped
       * to no data, rather than checking explicitly whether it
       * is full of zeroes)
       *
       * this is here for compatibility with RowCacheTile; it
       * returns false always to match the behaviour of GeneralTile
       * for non-row-cache tiles.
       */
      bool is_zero(Channel z, int y) const { return false; }


    protected:
      /**
       * shared portion of the Tile constructors
       */
      void makeTile(Iop& input, const Box& box, const ChannelSet& chans);

      /**
       * the BufferIop for this tile
       */
      BufferIop* _bufferIop;

      /**
       * bounding box for the available data in the tile
       */
      Box _box;

      /**
       * channels available from this tile
       */
      ChannelSet _channels;

      /**
       * whether or not this Tile contains valid data
       */
      bool _valid;

    };

    inline const float* BufferIopTile::LinePointers::operator[](int y) const
    {
      // Important note: this relies on the fact that data
      // stored inside an BufferIop must always contain planar
      // data at present. If this were to change in future, we would
      // need to return a strided pointer here to provide access to
      // the rows of data inside the tile.

      return _tile._bufferIop->getReadableAddress(0, y, _z);
    }

    inline BufferIopTile::LinePointers BufferIopTile::operator[](Channel z) const
    {
      return LinePointers(*this, z);
    }

  }
}

#endif
