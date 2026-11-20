// GeneralTile.h
// Copyright (c) 2013 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_GeneralTile_H
#define DDImage_GeneralTile_H

#include "DDImage/Box.h"
#include "DDImage/ChannelSet.h"

namespace DD {
  namespace Image {

    class PlanarI;
    class LocalPlaneCache;
    class Iop;
    class RowCacheTile;
    class InterestRatchet;

    /**
     * GeneralTile is a Tile-like API for access to image data, that can use both the
     * traditional row-based Tile API and can access ImagePlane data
     *
     * Unlike Interest/Tile this isn't a Box.  Some Box methods are provided
     * for source-compatibility, but it's probably best to use getBox() to get
     * the box instead.
     */
    class DDImage_API GeneralTile
    {
    public:

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
       * is this channel/row pair 'zero' (ie is it mapped
       * to no data, rather than checking explicitly whether it
       * is full of zeroes
       */
      bool is_zero(Channel z, int y) const;

      /**
       * whether the GeneralTile is accessible, or had it
       * aborted during construction.
       */
      bool valid() const;

      /**
       * Constructor. All image data will be available after this call, unless the calculation has
       * been aborted due to user action or an error.  Please check the aborted() state after
       * construction and do not use the pixel data if this is set.
       */
      GeneralTile(Iop& input, const Box& box, const ChannelSet& chans, bool forceUnpacked = false, bool mt = false, InterestRatchet* ir = nullptr);

      /**
       * destructor
       */
      ~GeneralTile();

      /**
       * get the data at a particular pixel.  with clipping
       */
      float at(int x, int y, Channel z);

      /**
       * whether this Tile was aborted during construction.
       */
      bool aborted() const;

      /**
       * Loads the image data from the specified range.
       * The range should be within bounds passed to the tile at construction time.
       */ 
      void load_range(int y, int t);

    protected:
      /**
       * helper function: get the requested box on an input, bypassing
       * Iops that contribute nothing towards the image
       */
      static Box getRequestBox(Iop& input);

      Iop& _input;
      PlanarI* _planarI;
      LocalPlaneCache* _ipc;
      ChannelSet _channels;
      Box _box;

      RowCacheTile* _rowCacheTile;

      bool _ipcIsShared;
      bool _valid;
      bool _wantPacked;
      std::set<ChannelSet> _allPlanes;

    private:
      void freeImagePlaneCache();
      void init(bool loadOnDemand, Iop& input, const Box& pBox, const ChannelSet& channels,
                        bool forceUnpacked, bool multiThread, InterestRatchet *interestRatchet);

    protected:

      /**
       * Constructor. This version expects an additional loadOnDemand argument. If set to
       * True, the tile properties will be initialised but image data will not be available
       * immediately after this call. It will instead be provided on demand as pixels are 
       * read from the tile. This can be useful for cases such as sampling where the tile 
       * defines a possible area that may be read from but the exact pixels which are 
       * needed are not known when the tile is constructed.
       */

      GeneralTile(bool loadOnDemand, Iop& input, const Box& box, const ChannelSet& chans,
          bool forceUnpacked = false, bool mt = false, InterestRatchet* ir = nullptr);

    };

  }
}

#endif
