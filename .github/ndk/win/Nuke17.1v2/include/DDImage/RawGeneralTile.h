// RawGeneralTile.h
// Copyright (c) 2013 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_RawGeneralTile_H
#define DDImage_RawGeneralTile_H

#include "DDImage/GeneralTile.h"

namespace DD {
  namespace Image {

    /**
     * Derivative of the GeneralTile which provides a two-level operator[] which will returns a raw pointer type.
     *
     * Use of this means that the data from the input will have to be unpacked if it is provided packed,
     * so it is probably best avoided and provided solely for compatibility
     */
    class DDImage_API RawGeneralTile : public GeneralTile
    {
    public:
      /*!
       * load the entire requested area of /input/, for /channels/; if /mt/ is true
       * then it may spawn threads to render the area in parallel.  the InterestRatchet
       * /ir/ might be passed down to the underlying RowCacheTile, if it uses one.
       */
      RawGeneralTile(Iop& input, const ChannelSet& chans, bool mt = false, InterestRatchet* ir = nullptr)
        : GeneralTile(input, getRequestBox(input), chans, true, mt, ir)
      {
      }

      /*!
       * load the area /box/ on /input/, for /channels/; if /mt/ is true
       * then it may spawn threads to render the area in parallel.  the InterestRatchet
       * /ir/ might be passed down to the underlying RowCacheTile, if it uses one.
       */
      RawGeneralTile(Iop& input, const Box& box, const ChannelSet& chans, bool mt = false, InterestRatchet* ir = nullptr)
        : GeneralTile(input, box, chans, true, mt, ir)
      {
      }

      /*!
       * load the area (x, y, r, t) on /input/, for /channels/; if /mt/ is true
       * then it may spawn threads to render the area in parallel.  the InterestRatchet
       * /ir/ might be passed down to the underlying RowCacheTile, if it uses one.
       */
      RawGeneralTile(Iop& input, int x, int y, int r, int t, const ChannelSet& chans, bool mt = false, InterestRatchet* ir = nullptr)
        : GeneralTile(input, Box(x, y, r, t), chans, true, mt, ir)
      {
      }

      /*! pointer to within an individual row */
      typedef const float* RowPtr;

      /*! helper class used for return value of operator[] */
      class LinePointers
      {
      public:
        LinePointers(const RawGeneralTile* tile, Channel channel)
          : _tile(tile)
          , _channel(channel)
        {
        }
        
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
        const RawGeneralTile* _tile;
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
      static const float* NilRow()
      {
        return nullptr;
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
      RawGeneralTile(bool loadOnDemand, Iop& input, const Box& box, const ChannelSet& chans, bool mt = false, InterestRatchet* ir = nullptr)
        : GeneralTile(loadOnDemand, input, box, chans, true, mt, ir)
      {
      }
      
      template <class TileType>
      friend class TSamplerImpl;
    };

  }
}

#endif
