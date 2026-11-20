// Row.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_Row_h
#define DDImage_Row_h

#ifdef FN_DEBUG
  #define RETURN_BOUNDED
#endif

#undef RETURN_BOUNDED


#include "DDImage/ChannelSet.h"
#include "DDImage/RowCheckMacros.h"
#include "DDImage/Deprecation.h"
#include <cassert>
#include <memory>
#include <array>


namespace DD
{
  namespace Image
  {
    class Iop;
    class CacheLineUnlockGuard;

    /*! \class DD::Image::Row

        Contains a read-write buffer of pixels representing
        a set of channels for a single horizontal line in an image. This
        is the standard method of passing image data around in DDImage.

        A Row acts like an array of 32 arrays of float pixel values, one
        for each possible channel. Each of these "buffers" can be indexed
        by X position. Only the locations greater or equal to a "left"
        position and less than a "right" position are legal, this range is
        set by the constructor or by the range() or offset() methods.
        (the stored pointer is offset from the allocated memory block
        by -left and the memory block is at least right-left long).

        The normal method of putting data into a Row is to call one of
        the get() functions. You can also call writable() to get a pointer
        to write to, or use erase() or copy().

        A great deal of code is dedicated to allowing the buffers to be
        shared between Row, Interest, caches, and local arrays, to avoid
        the expense of allocating them and especially of copying them.
        Each buffer may either be owned by the Row (in which case it is
        writable and it is deleted when the Row is destroyed), or it can
        be a read-only pointer to another piece of memory. Rows and other
        objects can copy the pointers from each other and can also transfer
        the "write" ownership of a buffer between them.

        Notice that getting your data into a Row and then looking at it
        is \e enormously faster than calling Iop::at(). You
        should do this if at all possible.
     */
    class DDImage_API Row
    {
      friend class Cache;

      std::array<float*, Chan_Last+1> _buffers;
      ChannelSet _writable; /*!< buffer can be written to */
      ChannelSet _own_buffer; /*!< buffer is owned by this Row */

      int _left;        /*!< X from constructor */
      int _right;        /*!< R from constructor */
      std::unique_ptr<CacheLineUnlockGuard> _g;

      // disable copy and assignment:
      Row& operator=(const Row&);

      /*! \fn Row::Row(const Row& from);
          The copy constructor is the same as creating a row with the same
          range as \a from and then doing from.pre_copy(row, Mask_All). This
          means the Row can write to any writable buffers that exist in
          the \a from Row.
       */
      Row(const Row &);

      /*! For internal use only. */
      void free(Channel z);

      /*! For internal use only.
       */
      static float* allocate_buffer(int W);

      // These are used by icache:
      const float* get_buffer(Channel z, int X, int R) const;

      /*! For internal use.
       */
      static void free_buffer(const float*, int X, int R);

      /*! For internal use.
       */
      void initialize(int X, int R);

      /*! For internal use.
       */
      void clear();

    public:

      int getLeft() const { return _left; }
      int getRight() const { return _right; }


      /*!
          Construct a Row where the legal range of horizontal indexes
          is x >= X and x < R. No buffers are allocated and attempts to
          index them without calling writable() first are undefined.
       */
      Row(int X, int R);

      /*! The destructor does release() and deletes any buffers allocated
          by writable().
       */
      ~Row();

      /*! Rows may have pointers directly into Iop caches, where they
          increment a reference count to lock the data into the cache.
          However this reference count is ignored when caches are cleared or
          deleted, so it is vital that you get rid of these pointers before
          this happens (which is when invalidate() is called on an Iop).

          This is usually done by the Row destructor but if you want to
          keep the row around for some reason, you can call this.
       */
      void release();

      /*! Checks whether row has a pointer to an Iop cache.
       */
      bool holdsCacheLine() const;

      /*! Sets the pointer to an Iop cache.
          This is for internal use and should not be used.
       */
      void setCacheLine(std::unique_ptr<CacheLineUnlockGuard>&& guard);

      /*! Change the Row to contain at least the horizontal range X..R.
          The actual range will be the \e union of the range passed to the
          constructor, and all calls to range() since then.

          This is useful if Iop::engine() would like to write pixels outside the
          x,r range passed to it. Without this call, writing these pixels
          could crash because that memory may not be allocated.

          An example of such an Iop is file readers which need to decompress
          starting at pixel zero. It is far easier and faster to write the
          decompressor so it can store all the pixels as it calculates them,
          rather than having to test each to see if it is in range. So most
          such file readers call range(0,width) and then write the entire
          width.

          Because callers typically reuse rows for multiple requests, the
          reallocated buffers and larger size will likely be preserved for
          the next call. This means that much less memory allocation and
          freeing will be done than it looks like this does.

          <i>WARNING: this will (possibly) free all buffers. Any pointers
          returned by [z] or by writable() are no longer valid.</i>

          See also the offset() function, which does not free buffers, but
          can only change the range to a new one of the same length.
       */
      void range(int X, int R);

      // Same as range() but will respect reductions of row width as well as expansions.
      void forceRange(int X, int R);

      /*! Move all the data and the left/right range of the row right by
          \a delta. This can be used to horizontally shift data without
          copying it.

          If the Row does not belong to you, you must restore the shift
          by calling this with -delta before returning.

          This can be used by an operator that horizontally shifts an image,
          for instance the Position operator does this (dx and dy contain
          the translation in pixels):

          \code
          row.offset(-dx);
          row.get(input0(), Y-dy, X-dx, R-dx, channels);
          row.offset(dx);
          \endcode
       */
      void offset(int delta);

      /*! You can make a Row write to your own allocated memory by calling
          this. The area from buffer[left] to buffer[right-1] (where left
          and right are the current range of the Row) must exist and it
          must stay around until after the Row is destroyed.

          There is no guarantee that the buffer will be written to, Iops may
          just change the pointer to caches, zeros, or other data. You
          should check if row[z]==buffer and if not you want to copy the
          data from row[z]+x to row[z]+r to buffer+x.
       */
      void write(Channel, float*);

      /*! You can make a Row read from your own allocated memory by calling
          this. The area from buffer[left] to buffer[right-1] (where left
          and right are the current range of the Row) must exist and it
          must stay around at least until the Row is destroyed.
       */
      void read(Channel, float*);

      ChannelMask writable_channels() const { return _writable; }

#ifdef RETURN_BOUNDED
      class ReadablePtr;

      /**
       * bounds-checked pointer, aborts if you try to access
       * when _ptr is outside the range (_min, _max)
       */
      class WritablePtr
      {
        friend class ReadablePtr;
        float* _ptr, * _min, * _max;
      public:
        WritablePtr(int) : _ptr(0), _min(0), _max(0)
        {
        }
        WritablePtr(float* ptr, float* min, float* max) : _ptr(ptr), _min(min), _max(max)
        {
        }
        WritablePtr(float* ptr, int x, int r) : _ptr(ptr), _min(ptr + x), _max(ptr + r)
        {
        }
        operator float*() {
          return _ptr;
        }
        operator const float*() const {
          return _ptr;
        }
        float& operator*()
        {
          mFnAssert (_ptr >= _min && _ptr < _max);
          return *_ptr;
        }
        float& operator[](int idx)
        {
          return *(_ptr + idx);
        }
        template<class T>
        WritablePtr operator+(T offset)
        {
          return WritablePtr(_ptr + offset, _min, _max);
        }
        template<class T>
        WritablePtr operator-(T offset)
        {
          return WritablePtr(_ptr - offset, _min, _max);
        }
        WritablePtr operator++(int)
        {
          float* orig = _ptr;
          _ptr++;
          return WritablePtr(orig, _min, _max);
        }
        WritablePtr operator--(int)
        {
          float* orig = _ptr;
          _ptr--;
          return WritablePtr(orig, _min, _max);
        }
        WritablePtr& operator++()
        {
          ++_ptr;
          return *this;
        }
        WritablePtr& operator--()
        {
          --_ptr;
          return *this;
        }
        void operator+=(size_t i)
        {
          _ptr += i;
        }
        void operator-=(size_t i)
        {
          _ptr += i;
        }
      };

      /**
       * bounds-checked pointer, aborts if you try to access
       * when _ptr is outside the range (_min, _max)
       */
      class ReadablePtr
      {
        const float* _ptr, * _min, * _max;
      public:
        ReadablePtr(int) : _ptr(0), _min(0), _max(0)
        {
        }
        ReadablePtr(const float* ptr, const float* min, const float* max) : _ptr(ptr), _min(min), _max(max)
        {
        }
        ReadablePtr(const WritablePtr& other) : _ptr(other._ptr), _min(other._min), _max(other._max)
        {
        }
        operator const float*() const {
          return _ptr;
        }
        operator float*() {
          return (float*)_ptr;
        }
        const float& operator*() const
        {
          mFnAssert (_ptr >= _min && _ptr < _max);
          return *_ptr;
        }
        template<class T>
        ReadablePtr operator+(T offset)
        {
          return ReadablePtr(_ptr + offset, _min, _max);
        }
        template<class T>
        ReadablePtr operator-(T offset)
        {
          return ReadablePtr(_ptr - offset, _min, _max);
        }
        ReadablePtr operator++(int)
        {
          const float* orig = _ptr;
          _ptr++;
          return ReadablePtr(orig, _min, _max);
        }
        ReadablePtr operator--(int)
        {
          const float* orig = _ptr;
          _ptr--;
          return ReadablePtr(orig, _min, _max);
        }
        ReadablePtr& operator++()
        {
          ++_ptr;
          return *this;
        }
        ReadablePtr& operator--()
        {
          --_ptr;
          return *this;
        }
        void operator+=(size_t i)
        {
          _ptr += i;
        }
        void operator-=(size_t i)
        {
          _ptr += i;
        }
      };

  #define READABLE_PTR(a, b, c) Row::ReadablePtr(a, b, c)
  #define WRITABLE_PTR(a, b, c) Row::WritablePtr(a, b, c)

#else
      typedef float* WritablePtr;
      typedef const float* ReadablePtr;

  #define READABLE_PTR(a, b, c) Row::ReadablePtr(a)
  #define WRITABLE_PTR(a, b, c) Row::WritablePtr(a)

#endif


      /*! \fn ReadablePtr Row::operator[](Channel z) const;

          Return a pointer to the pixel at x == 0 in channel \a z.

          Only the region of this buffer between the X and R passed to
          the constructor actually exists. Indexing outside that area
          will produce garbage or a core dump. If this channel has not
          been requested with get() or writable() the returned value
          is garbage.

          To access an arbitrary pixel at channel z, horizontal
          position x, call <tt>row[z][x]</tt>. For maximum speed it is best
          to use a local pointer with <tt>const float* IN = row[z]+x</tt>
          and then increment it across the row.
       */
      ReadablePtr operator[](Channel z) const { return READABLE_PTR(_buffers[z], _buffers[z] + _left, _buffers[z] + _right); }

      /*! Return a pointer to pixel at x == 0 in channel \a z, this pointer
          points at memory that may be written. If the current buffer is
          not writable, it is thrown away and a buffer is allocated.

          Converting a row to writable <i>does not preserve the contents</i>.
          However you can assume that multiple calls to writable() will
          return the same pointer.

          You can only write to the region between the X and R passed to
          the constructor. Writing outside that area is a sure way to get
          a core dump!

          If z is Chan_Black then a pointer is returned to the shared
          junk_buffer(), where you can safely dispose of your data without
          having to put a test for Chan_Black into your code. However
          all operators and threads share this buffer so you cannot rely
          on the data staying unchanged after you store it, this is a
          write-only buffer!
       */
      WritablePtr writable(Channel z);

      /*!
         Utility function.
         Allocate a buffer at channel \a z where all the values are set to
         \a val. Otherwise behaves exactly the same as writable().
       */
      WritablePtr writableConstant(const float val, Channel z);

      static bool is_zero(const float*, int X, int R);


      /*! Set things up so a later copy() with the same arguments may run
          a lot faster.

          This is done by modifying \a source to share buffer pointers with
          this Row. This is a hack: you must not do any calls to \a this Row
          other than the matching copy() until \a source is deleted. Any
          attempt to use this Row may cause some buffers to be freed and
          then the \a source row will be pointing at freed memory.

          The arguments are exactly like copy() and you call this method
          on the \e destination row, even though it really modifies the
          source row. This was done so that you can easily duplicate the
          arguments to copy() when making this function. See copy() for
          descriptions of the arguments.

          Sample usage:
         \code
         void MyIop::engine(int y, int x, int r, ChannelMask channels, Row& out) {
         ChannelSet achannels = channels; achannels &= (copied_channels);
         ChannelSet bchannels = channels; bchannels -= (copied_channels);
         Row arow(x,r);
         out.pre_copy(arow, uncooked);
         arow.get(input1(), y, x, r, achannels);
         out.get(input0(), y, x, r, bchannels);
         out.copy(arow, uncooked, x, r);
         }
         \endcode
       */
      void pre_copy(Channel z, Row&, Channel source_channel) const;
      void pre_copy(Row& s, Channel z) const { pre_copy(z, s, z); }

      /*! Copy a channel from the \a source Row to this row.

          WARNING: \a X and \a R indicate what region you \e require to
          be copied, but more than that may be copied! This is not
          a merge! To merge data, you must copy the pixels yourself.

          Calling pre_copy() with the same arguments before filling the
          source row can greatly speed this up by allowing an already-existing
          output buffer to be shared. In this case the copy may already be
          done and this will return immediately.
       */
      void copy(Channel z, const Row&, Channel source_channel, int x, int r);
      void copy(const Row& s, Channel z, int x, int r) { copy(z, s, z, x, r); }

      /*! Set things up so a later copy() with the same arguments may run
          a lot faster.

          This is the same as calling pre_copy(z,source,z) for all the given
          channels.
       */
      void pre_copy(Row &, ChannelMask) const;

      /*! Same as "foreach(z,channels) copy(z, source, z, x, r)" but this may
          be faster.
       */
      void copy(const Row &, ChannelMask, int x, int r);

      /*! \fn void Row::get(Iop& iop, int y, int x, int r, ChannelMask channels)
          For back compatibility only, this is the same as doing
          iop.get(y, x, r, channels, this);
       */
      void get(Iop &, int y, int x, int r, ChannelMask);

      /*! Not implemented in the library but you can define it. */
      friend std::ostream& operator<<(std::ostream&, const Row&);


      /*!
         Return an array containing at least \a size 0.0 values.
         This can be used as a source of zeros in an algorithim that does not
         want the overhead of checking whether a pointer is null or not. It is
         also used by the erase() method and pointers are checked against the
         return values for the is_zero() method.
       */
      static const float* zero_buffer(unsigned size);
      
      /*! Return true if \a array is equal to one of the return values from zero_buffer(). */
      static bool is_zero_buffer(const float*);

      /*!
         Return an array containing at least \a size locations you can
         write to.  This can be used as a "data sink" for algorithims that
         produce data that might be ignored. A pointer to this is returned
         when writable(Chan_Black) is called.
       */
      static float* junk_buffer(unsigned size);


      /*!
         Clear the channels to zero. This is done by changing the pointer
         to point at zero_buffer(). This will make the is_zero() test return true.
       */
      void erase(ChannelMask);

      /*!
         Clear the entire channel to zero. This is done by changing the
         pointer to point at zero_buffer(). This makes the is_zero() test
         return true.
       */
      void erase(Channel);

      /*!
         Fast test to see if all of the given channel is zero. This will return
         true if erase() was called, or if other code such as caches has detected
         that the row is entirely zero.
       */
      bool is_zero(Channel channel) const;

      void _debug(int x, int r, ChannelMask channels) const;
#if DD_ROW_CHECK
      void debug(int x, int r, ChannelMask channels) const { _debug(x, r, channels); }
#else // DD_ROW_CHECK
      void debug(int x, int r, ChannelMask channels) const {}
#endif // DD_ROW_CHECK
    };

  }
}

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
