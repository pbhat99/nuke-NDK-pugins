// RowCacheTile.h
// Copyright (c) 2013 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_RowCacheTile_h
#define DDImage_RowCacheTile_h

#include "Iop.h"
#include "Interest.h"

namespace DD
{
  namespace Image
  {

    /*! \class DD::Image::RowCacheTile

        A tile is the fastest way to get data from an image, but has
        the disadvantage that every pixel inside the requested area
        is calculated by the constructor. Thus this is only efficient
        if you expect to address every pixel in the region.

        The data is locked into the cache by the existence of the RowCacheTile,
        just like Interest does, but the constructor also calculates
        any lines that are not there yet. If you are running multithreaded
        then there is a good chance that other threads are calculating
        the same or overlapping tiles, so the filling of this tile is
        in fact multithreaded. (I may change this in the future so the
        constructor actually lauches threads to try to fill the tile in
        as fast as possible).

        The absolute fastest way to get data is to use operator[], but
        you will have to do boundary and channel mask checking yourself.

        You can also use at(), which match the same-named methods on Iop,
        but are significantly faster.
     */
    class DDImage_API RowCacheTile : public Interest
    {

      RowCacheTile& operator=(const RowCacheTile&); // disable assignment operator
      RowCacheTile(const RowCacheTile &); // disable copy constructor

    public:

      /*! \fn RowCacheTile::RowCacheTile(Iop& iop, ChannelMask channels, bool mt, InterestRatchet * ir)
          This constructor gets all the data in the reqested() area of
          the Iop.
       */
      RowCacheTile(Iop & iop, ChannelMask channels, bool mt = false, InterestRatchet * ir = nullptr) :
        Interest(iop, channels, mt, ir) { load_range(y(), t()); }

      RowCacheTile(Iop & iop, DD::Image::Box box, ChannelMask channels, bool mt = false, InterestRatchet* ir = nullptr) :
        Interest(iop, box.x(), box.y(), box.r(), box.t(), channels, mt, ir) { load_range(y(), t()); }

      /*!    \fn RowCacheTile::RowCacheTile(Iop& iop, int x, int y, int r, int t, ChannelMask channels, bool mt, InterestRatchet * ir)

          The constructor is exactly like the Interest constructor except it
          immediately calls load_range(y(), t()).

          You must check Op::aborted() after the constructor. If this is true
          then data is missing from the tile, and you should destroy it
          without using any of the data. (future versions will use exceptions,
          making this much less of a pain, but you must then not assume
          the constructor will even return!)

          The constructor will probably (unless the data already is in the
          cache()) execute the input Iop for every line in the tile. This can
          take a very long time. It may throw an exception if an error happens
          or the user interrupts processing, you must check for aborted() and
          don't try to use the data in that case.
       */
      RowCacheTile(Iop & iop, int x, int Y, int r, int T, ChannelMask channels, bool mt = false, InterestRatchet * ir = nullptr) :
        Interest(iop, x, Y, r, T, channels, mt, ir) { load_range(y(), t()); }

      /*! Return several channels of a pixel in the tile.

          This is the same as the Interest::at() but is faster because
          it can assume the line is already loaded.
       */
      void at(int x, int y, ChannelMask, float*);

      /*! Return a single channel of a pixel in the tile, with edge clipping.

          This is the same as the Interest::at() but is faster because
          it can assume the line is already loaded.
       */
      float at(int x, int y, Channel);

      /*! Not implemented in the library but you can define it. */
      friend std::ostream& operator<<(std::ostream&, const RowCacheTile&);

      static const float* NilRow()
      {
        return nullptr;
      }

    protected:

      /**
       * Constructor. This version expects an additional loadOnDemand argument. If set to
       * True, the tile properties will be initialised but image data will not be available
       * immediately after this call. It will instead be provided on demand as pixels are 
       * read from the tile. This can be useful for cases such as sampling where the tile 
       * defines a possible area that may be read from but the exact pixels which are 
       * needed are not known when the tile is constructed.
       */
      RowCacheTile(bool loadOnDemand, Iop & iop, DD::Image::Box box, ChannelMask channels,  bool mt = false, InterestRatchet* ir = nullptr) :
        Interest(iop, box.x(), box.y(), box.r(), box.t(), channels, mt, ir) { if (loadOnDemand) reserve_range(y(), t()); else load_range(y(), t()); }

      template <class TileType>
      friend class TSamplerImpl;
      friend class GeneralTile;
    };

  }
}

#endif

// Copyright (c) 2013 The Foundry Visionmongers Ltd.  All Rights Reserved.
