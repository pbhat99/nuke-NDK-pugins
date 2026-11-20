// DirectGeneralTile.h
// Copyright (c) 2013 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_DirectGeneralTile_H
#define DDImage_DirectGeneralTile_H

#include "DDImage/GeneralTile.h"

#include "DDImage/ImagePlane.h"

namespace DD {
  namespace Image {

    /*!
     * Derivative of the GeneralTile which provides a two-level
     * operator[] which will returns a strided pointer type.
     */
    class DDImage_API DirectGeneralTile : public GeneralTile
    {
    public:
      /*!
       * load the entire requested area of /input/, for /channels/; if /mt/ is true
       * then it may spawn threads to render the area in parallel.  the InterestRatchet
       * /ir/ might be passed down to the underlying RowCacheTile, if it uses one.
       */
      DirectGeneralTile(Iop& input, const ChannelSet& chans, bool mt = false, InterestRatchet* ir = nullptr)
        : GeneralTile(input, getRequestBox(input), chans, false, mt, ir)
      {
      }

      /*!
       * load the area /box/ on /input/, for /channels/; if /mt/ is true
       * then it may spawn threads to render the area in parallel.  the InterestRatchet
       * /ir/ might be passed down to the underlying RowCacheTile, if it uses one.
       */
      DirectGeneralTile(Iop& input, const Box& box, const ChannelSet& chans, bool mt = false, InterestRatchet* ir = nullptr)
        : GeneralTile(input, box, chans, false, mt, ir)
      {
      }

      /*!
       * load the area (x, y, r, t) on /input/, for /channels/; if /mt/ is true
       * then it may spawn threads to render the area in parallel.  the InterestRatchet
       * /ir/ might be passed down to the underlying RowCacheTile, if it uses one.
       */
      DirectGeneralTile(Iop& input, int x, int y, int r, int t, const ChannelSet& chans, bool mt = false, InterestRatchet* ir = nullptr)
        : GeneralTile(input, Box(x, y, r, t), chans, false, mt, ir)
      {
      }

      /*! pointer to within an individual row */
      typedef ImageTileReadOnlyPtr RowPtr;

      /*! helper class used for return value of operator[] */
      class LinePointers
      {
      public:
        LinePointers(const DirectGeneralTile* tile, Channel channel)
          : _tile(tile), _channel(channel) { }

        /*!
         * return a pointer to the 'start' of an individual row (ie the
         * position at which x = 0, even if this is not in itsel
         * dereferancable
         */
        inline RowPtr operator[](int y) const
        {
          return _tile->lookupLine(_channel, y);
        }

      private:
        const DirectGeneralTile* _tile;
        const Channel _channel;
      };

      friend class LinePointers;
      
      /*!
       * given a channel, return something that acts like an array
       * of pointers to all the loaded lines
       */
      inline LinePointers operator[](Channel z) const
      {
        return LinePointers(this, z);
      }

      /*!
       * return an empty row pointer
       */
      static RowPtr NilRow()
      {
        return RowPtr();
      }

    private:
      /*! helper function for implementation of LinePointers::operator[] */ 
      RowPtr lookupLine(Channel z, int y) const;
      
    protected:
      /**
       * Constructor. This version expects an additional loadOnDemand argument. If set to
       * True, the tile properties will be initialised but image data will not be available
       * immediately after this call. It will instead be provided on demand as pixels are
       * read from the tile. This can be useful for cases such as sampling where the tile
       * defines a possible area that may be read from, but the exact pixels which are
       * needed are not known when the tile is constructed.
       */
      DirectGeneralTile(bool loadOnDemand, Iop& input, const Box& box, const ChannelSet& chans, bool mt = false, InterestRatchet* ir = nullptr)
        : GeneralTile(loadOnDemand, input, box, chans, false, mt, ir)
      {
      }
      
      template <class TileType>
      friend class TSamplerImpl;
    };

  }
}

#endif
