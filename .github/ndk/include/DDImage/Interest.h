// Interest.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_Interest_h
#define DDImage_Interest_h

#include "Iop.h"
#include "MemoryHolder.h"

#ifdef __INTEL_COMPILER
#pragma warning(disable:444)  
#endif

namespace DD
{
  namespace Image
  {

    // forward declaration
    class LineState;

    class CacheBase : public DD::Image::MemoryHolder
    {
    public:
      LineState* line_state;
      const float** line_pointers[Chan_Last + 1];
    };

    /**
    ** If you create one of these, and pass it to 'Interest' then it will remember
    ** which Iops it has previously called addInterest on, and not do re-do these,
    ** thus saving time as addInterest involves contention
    **
    ** Interests are removed again when the InterestRatchet is destroyed.
    **/
    class DDImage_API InterestRatchet
    {
      std::set<Cache*> _interests;

    public:

      InterestRatchet() {
      }

      bool interestedIn(Cache* cache)
      {
        return _interests.find(cache) != _interests.end();
      }

      void addInterest(Cache* cache)
      {
        _interests.insert(cache);
      }

      /**
       * destructor for InterestRatchet, must call removeInterest on
       * each of the Caches it is maintaining an interest count on
       */
      ~InterestRatchet();
    };

    //This object derives from a Box* type, which doesn't have a virtual destructor. This
    //is why warning 444 has been disabled for the intel compiler at the top of this file - the important
    //message is to not delete via a Box* pointer, as the destructor will not
    //get called.

    /*! \class DD::Image::Interest

        An Interest object creates a cache on the Iop and prevents
        any lines that are put into that cache from being deleted
        until the Interest is destroyed. This can be used by an
        engine() implementation to indicate that it will ask for
        the same data using Iop::at() or Iop::get() multiple times.

        The lines will probably stay around for some time after the
        interest is destroyed, so you can assume they are still there
        the next time the engine() is called.
     */
    class DDImage_API Interest : public Box
    {
      InterestRatchet* _interestRatchet;

      friend class Cache;
      ChannelSet channels_;
      CacheBase* cache_;

      float _fracCPU;
      bool _unlocked;
      bool _multithreaded;


      void launch_threads(); // part of the constructor

      void init(Iop* ip, const Box& b, ChannelMask channels, bool mt, InterestRatchet* interestRatchet, float fracCPU);

    public:
      //! type of actual row data.  to be replaced with a strided pointer
      typedef const float* RowPtr;

      //! type of the linebuffer returned by operator[].
      typedef RowPtr const* LinePointers;

      /*! This constructor locks the entire requested() area and all
          requested channels into the memory. */
      Interest(Iop &, ChannelMask, bool mt = false, InterestRatchet * ir = nullptr, float fracCPU = 1.0f);

      /*! This constructor takes a box and channel mask, and only the
          interesection of these and the "requested" box and channels of
          the operator are locked into memory.

          If \a channels is empty (== Mask_None) then the bounding box is
          not clipped.

          If \a mt is true, threads()-1 parallel threads are started to
          try to fill in the lines in the interest. This should be useful
          for getting the input of an operator that has to run single-threaded.
       */
      Interest(Iop &, int x, int y, int r, int t, ChannelMask, bool mt = false, InterestRatchet * ir = nullptr, float fracCPU = 1.0f);
      Interest(Iop &, const Box& box,             ChannelMask, bool mt = false, InterestRatchet * ir = nullptr, float fracCPU = 1.0f);

      Interest(const Interest &)           = delete;
      Interest& operator=(const Interest&) = delete;

      ChannelMask channels() const { return channels_; }

      /*!
         Call this to indicate that the Interest does not actually lock lines
         into memory.  This means the Interest is simply used to \e encourage
         caching of the lines. Calling this after calling the multithreaded
         constructor is an easy way to get parallel threads to generate lines
         you may use.

         Warning: none of the access functions such as at() will work
         reliably after this, as the memory may disappear right in the
         middle of their operation! Use get() on the Iop instead to
         retrieve the data, or create other temporary Interests.
       */
      void unlock();

      /*! \fn bool Interest::unlocked() const;
         Returns true if unlock() had been called. */
      bool unlocked() const { return _unlocked; }

      /*! Return true if any done() data in the cache matches the current
          state of the Iop. Most users of Interest and Tiles do not need
          to check this, as the Interest is created after the Iop is validated
          and is destroyed before it is invalidated. You need this in case
          you are keeping interests around while changing the Iop data.
       */
      bool valid() const;

      /*! \fn bool Interest::done(int y) const
          Returns true if line \a y has been calculated and thus is holding
          correct data in the cache memory. This means that at() will return
          quickly, and that operator[] will work for this line.

          This is an extremely fast inline function, and it assumes \a y is
          inside the y() through t()-1 range. If y is outside this range
          unpredictable results occur. You can use cache->clamp(y) to move
          an arbitrary number in range. This also does not check valid(), if
          valid() is false then this may return true even if it is not.
       */
      bool done(int y) const;

      /*! \fn bool Interest::have_memory(Channel z, int y) const

          Returns true if there is memory allocated for channel \a z of line
          \a y.  If this returns true them operator[] will not crash for
          this channel and line. If \a z is one of the \a channels() then
          you can also assume that memory exists for all the other channels
          for this row.

          The memory may contain correct data, or it may contain an older
          version of the image. If it contains the older version it may get
          overwritten with the new data at any time.
       */
      bool have_memory(Channel z, int y) const { return cache_->line_pointers[z][y] != nullptr; }

      /*! \fn LinePointers Interest::operator[](Channel z) const;

          To access an arbitrary pixel at channel z, row y, horizontal
          position x, call <tt>tile[z][y][x]</tt>.

          If you access non-existent data you will get a crash or garbage.
          To avoid this you must make sure that:

          1. <tt>!z || tile.channels() & z</tt>

          2. <tt>y >= tile.y() && y < tile.t()</tt>

          3. <tt>x >= tile.x() && x < tile.r()</tt>

          4. Make sure the line is done() (or at least have_memory()). For
          the Tile subclass you can assume this is true.

          Depending on your algorithim you can usually insure this without
          doing a test on every pixel like at() does.
       */
      LinePointers operator[](Channel z) const { return cache_->line_pointers[z]; }
      
      /*! Return true if it knows that this row and channel are zero. This
          will happen if some earlier operator called erase() on the Row
          when creating the data. Also the caching sometimes detects zero
          and turns this on.

          This also returns true if the channel is not in this tile.

          \a y is clamped to the range of the tile. However you must
          make sure done(y) is true (for Tile this can be assumed).
       */
      bool is_zero(Channel z, int y) const;

      /*! Load a set of lines, so that done() is true for all lines from
          bottom to top-1. Obeys the ydirection and cooperates with other
          threads that may be loading the same set of lines. The range must
          line inside the y() to t()-1 range of the Interest or unpredicable
          results will occur!

          For historical reasons setting the cache pointer to null is used
          to signal the threads to quit. The destructor for the Interest does
          this and then waits for this function to return.
       */
      void load_range(int, int);

      Cache* cache() const { return (Cache*)cache_; }

      /*! Return several channels of a pixel in the interest. The passed array
          is indexed by channel number and filled in with the values from
          the image. Unrequested channels (including Chan_Black) are
          \e unchanged in the array. The array must be big enough to contain
          the largest of the requested channels.

          If a channel was not asked for in the constructor or was not in the
          requested_channels() of the iop then zero is returned.

          The x and y coordinates are moved to the nearest edge of the
          interest's area, which is the intersection of the requested()
          area of the iop and the arguments passed to the constructor.

          Conceivably this is faster than Iop::at() because it can
          assume the lines are already locked, but currently it is
          the same speed.
       */
      void at(int x, int y, ChannelMask m, float* o);

      /*! Return a single channel of a pixel in the interest.

          If the channel was not asked for or was not in the
          requested_channels() of the iop then zero is returned.

          The x and y coordinates are moved to the nearest edge of the
          interest area, which is the intersection of the requested()
          area of the iop and the arguments passed to the constructor.

          Because the interest guarantees that a line stays in memory, this
          will probably be faster than Iop::at() for the second and
          subsequent requests for pixels in the same row.
       */
      float at(int x, int y, Channel z);

      /*! The destuctor unlocks the pixels from the input iop's cache. */
      ~Interest();
      
      /*! Not implemented in the library but you can define it. */
      friend std::ostream& operator<<(std::ostream&, const Interest&);

      /** the topmost op that actually contributes image data into /iop/ */
      static Iop* inputIop(Iop& iop);

    protected:

      /*! Declares an interest in the specified range of scanlines without 
          actually loading them into the cache. Used for on-demand loading
          of tiles
      */
      void reserve_range(int bottom, int top);
    
    private:
      bool _loadOnDemand = false;
    };

  }
}

#ifdef __INTEL_COMPILER
#pragma warning(default:444)  
#endif

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
