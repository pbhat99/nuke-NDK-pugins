// Copyright (c) 2009 The Foundry Visionmongers Ltd. All Rights Reserved.
#ifndef DDIMAGE_CHANNELSET_H
#define DDIMAGE_CHANNELSET_H

#include "Channel.h"
#include "Hash.h"

#define CHANNELSET_SPARE_SIZE 2

namespace ndk {
  class ChannelSet;  // In NdkLib
}

namespace DD {
  namespace Image {

    //! A set of zero or more DD::Image::Channel numbers. This set cannot
    //! contains Chan_Black (zero). As these sets are used in many places
    //! in Nuke, they are designed to be as fast and small as possible.
    class DDImage_API ChannelSet
    {
    public:
      // Constructors.
      ChannelSet() = default;
      ChannelSet(ChannelSetInit v) : mask(v) {}
      ChannelSet(Channel v) { *this = v; }
      ChannelSet(const Channel* array, int n) { insert(array, n); }

      //! Construct from an ndk::ChannelSet. ChannelManager is ignored as only the
      //! channel order is important.
      //! For internal use only!
      explicit ChannelSet(const ndk::ChannelSet& ndkChannels);

      // Copy constructor.
      ChannelSet(const ChannelSet& source);

      // Move constructor.
      ChannelSet(ChannelSet&& source);

      // Destructor.
      ~ChannelSet()
      {
        if (others != spare) {
          delete[] others;
        }
      }

      // Copy assignment operators.
      const ChannelSet& operator=(const ChannelSet& source);
      const ChannelSet& operator=(ChannelSetInit source)
      {
        mask = source;
        truncate();
        return *this;
      }
      const ChannelSet& operator=(Channel z);

      // Move assignment operator.
      const ChannelSet& operator=(ChannelSet&& source);

      void swap(ChannelSet& source);

      void clear()
      {
        mask = 0;
        truncate();
      }
      operator bool() const { return mask || others; }
      bool empty() const { return !mask && !others; }

      void append(DD::Image::Hash& hash) const
      {
        hash.append(mask);
        if (others) {
          for (size_t i = 1; i < *others; i++) {
            hash.append(others[i]);
          }
        }
      }

      //! A ChannelSet can store an infinite number of channels, where all
      //! channels after the last one that has been turned off are considered
      //! "on". This allows a set to be inverted, and to make a set that can
      //! be intersected with any other set and not change it.
      //!
      //! The only way to get a ChannelSet into this state is to assign or
      //! initialize with the Mask_All constant, or if you += an all() set.
      //! It will stay in this state until you intersect with a non-all() set
      //! or you -= an all() set.
      //!
      //! Some functions, in particular first(), next(), last(), and size(),
      //! do not work when all() is true and return undefined results.
      bool all() const { return (mask & 0x80000000u) != 0; }

      bool operator==(const ChannelSet& source) const;
      bool operator!=(const ChannelSet& source) const { return !(*this == source); }

      //! This is an arbitrary sorting that is used so that a ChannelSet
      //! can be put into an stl set.
      bool operator<(const ChannelSet& source) const;
      bool operator==(ChannelSetInit v) const { return mask == U32(v) && !others; }
      bool operator!=(ChannelSetInit v) const { return !(*this == v); }
      bool operator==(Channel z) const;
      bool operator!=(Channel z) const { return !(*this == z); }

      //! Turn on all the channels in \a source. This may more sense
      //! if it was operator|=, but calling it += allows -= to be
      //! used symetrically.
      void operator+=(const ChannelSet& source);
      void operator+=(ChannelSetInit source) { mask |= source; }
      void operator+=(Channel z);
      void insert(Channel z) { *this += z; }
      void insert(const Channel* array, int n)
      {
        for (int i = 0; i < n; i++) {
          *this += array[i];
        }
      }

      //! Turn off all the channels in \a source.
      void operator-=(const ChannelSet& source);
      void operator-=(ChannelSetInit source) { mask &= ~source; }
      void operator-=(Channel z);
      void erase(Channel z) { *this -= z; }
      void erase(const Channel* array, int n)
      {
        for (int i = 0; i < n; i++) {
          *this -= array[i];
        }
      }

      //! In-place intersection. Only the channels that were in both sets
      //! remain in the new set.
      void operator&=(const ChannelSet& source);
      void operator&=(ChannelSetInit source)
      {
        mask &= source;
        truncate();
      }
      void operator&=(Channel z);

      template<class Type> ChannelSet operator+(const Type& z) const
      {
        ChannelSet tmp = *this;
        tmp += z;
        return tmp;
      }

      template<class Type> ChannelSet operator-(const Type& z) const
      {
        ChannelSet tmp = *this;
        tmp -= z;
        return tmp;
      }

      //! Return the intersection of this ChannelSet with another as a ChannelSet
      template<class Type> ChannelSet intersection(const Type& z) const
      {
        ChannelSet tmp = *this;
        tmp &= z;
        return tmp;
      }

      //! Returns true if the intersection is not empty. This returns a
      //! bool, not a ChannelSet, because almost all uses of this are in
      //! if statements conditions.
      bool operator&(const ChannelSet& c) const;
      bool operator&(ChannelSetInit c) const { return (mask & c) != 0; }
      bool operator&(Channel z) const;
      unsigned count(Channel z) const { return *this & z ? 1 : 0; }

      // This may be useful as a "cast" to a ChannelSetInit:
      ChannelSetInit bitwiseAnd(ChannelSetInit lOther)     { return ChannelSetInit(mask & lOther); }
      // This is not recommended as it will lose any channels greater than 31 in both of them:
      //ChannelSetInit bitwiseAnd(const ChannelSet& lrOther) { return ChannelSetInit(mask & lrOther.mask); }

      //! Returns true if all the channels in \a source are in this.
      //! I.e. true if this is a superset of \a source.
      bool contains(const ChannelSet& source) const;
      bool contains(ChannelSetInit source) const { return !(~mask & source); }

      //! This is the same as *this&z except it returns true for Chan_Black.
      bool contains(Channel z) const;

      //! Same as "for (i=0; i<n; i++) *this += DD::Image::brother(channel,i);"
      //! but it may be more efficient.
      void addBrothers(Channel, unsigned n);

      //! Return the number of channels in the set. The answer is
      //! meaningless if all() is true.
      unsigned size() const;

      //! Returns the first channel in the set. Returns Chan_Black if the
      //! set is empty. Does not work if all() is true.
      Channel first() const
      {
        if (mask & 1) {
          return Chan_Red;
        }
        return _first();
      }

      //! Returns the first channel in the set that is greater than \a k.
      //! Returns Chan_Black if there is none.
      //! Does not work if all() is true.
      Channel next(Channel k) const
      {
        if (k < 31 && (mask & (1u << k))) {
          return Channel(k + 1);
        }
        return _next(k);
      }

      //! Returns the last channel in the set. Returns Chan_Black if it is empty.
      //! Does not work if all() is true.
      Channel last() const;

      //! Returns the last channel before \a k in the set. k must not be
      //! Chan_Black.  Returns Chan_Black if there is none.
      //! Does not work if all() is true.
      Channel previous(Channel k) const;

      // These are for back-compatibility only, do not use!
      bool operator&(int c) const { return *this & ChannelSetInit(c); }
      unsigned value() const;

      //! Convert to an ndk::ChannelSet assuming the channel mappings are not important
      //! and are just integer indices.
      ndk::ChannelSet asNdkChannelSet() const;

    private:
      U32 mask = 0;          // Channels 1..31 and the "all" bit.
      U32* others = nullptr; // Count + channels 32..inf.
      U32 spare[CHANNELSET_SPARE_SIZE] = {0};

      void auto_truncate();
      void truncate()
      {
        if (others != spare) {
          delete[] others;
        }
        others = nullptr;
      }
      void truncate(unsigned n);
      void extend(unsigned n);
      unsigned count() const { return others ? *others : 1; }
      Channel _first() const;
      Channel _next(Channel k) const;

      // Helper function for move semantics.
      void moveFrom(ChannelSet& source);
    };

    /*! \relates DD::Image::ChannelSet
       Runs the body of the loop with the local variable named VAR set to
       each channel in the ChannelSet.

       You should use the foreach() macro to go through all the channels in
       a set, as in this example:

       \code
       foreach(z, row.channels) process(row[z]+x, r-x);
       \endcode
     */
#ifdef foreach
  #undef foreach
#endif
#define foreach(VAR, CHANNELS) \
  for (DD::Image::Channel VAR = CHANNELS.first(); VAR; VAR = CHANNELS.next(VAR))

    /*! \relates DD::Image::ChannelSet

       Prints which channels are in the set as the smallest possible list
       of layer names and channel names. If all the channels in a layer are
       on, it prints that layer, else it prints the channel names.

       Prints "none" for the empty set, and "all except ..." for a set with
       the "all" bit set.
     */
    DDImage_API std::ostream& operator<<(std::ostream&, const ChannelSet&);

    ////////////////////////////////////////////////////////////////
    // Back compatibility functions:
    // Don't use any of this in new code!

    /*! \relates DD::Image::ChannelSet
       This typedef makes most method prototypes source-code compatible: */
    typedef const ChannelSet& ChannelMask;

    /*! \relates DD::Image::ChannelSet

       Constant for the highest Channel number that will be produced by Nuke.
       This can be used to declare a fixed-size array that will be indexed
       by channel. You can either declare the array to have a size of Chan_Last+1,
       or of a size Chan_Last and subtract 1 from the Channel to index it (thus
       removing the Chan_Black entry at 0).

       This is quite large and you should try to make a variable-sized
       array if possible (use ChannelSet::last() to find the last channel
       number instead).
     */
    const int Chan_Last = 1023;

    inline bool intersect(const ChannelSet& m, const ChannelSet& c) { return m & c; }
    inline bool intersect(const ChannelSet& m, ChannelSetInit c) { return m & c; }
    inline bool intersect(const ChannelSet& m, Channel z) { return m & z; }

    inline bool turn_off(const ChannelSet& c, const ChannelSet& m) { return !m.contains(c); }
    inline bool turn_off(ChannelSetInit c, const ChannelSet& m) { return !m.contains(c); }
    inline bool turn_off(Channel c, const ChannelSet& m) { return !m.contains(c); }

    inline ChannelSetInit depth2mask(int d) { return (ChannelSetInit)((1 << d) - 1); }

    inline void mask2array(Channel* array, unsigned n, ChannelMask mask, const char* layer)
    {
      unsigned i;
      for (i = 0; i < n;)
        array[i++] = Chan_Black;
      foreach (z, mask) {
        i = colourIndex(z, layer);
        if (i < n)
          array[i] = z;
      }
    }

    inline ChannelSet array2mask(const Channel* array, int n)
    {
      return ChannelSet(array, n);
    }

    // True back-compatibility requires this to convert to a ChannelSet,
    // but instead this relies on automatic type conversion.
    inline Channel mask(Channel c) { return c; }

    //! An iterator for ChannelSets. This provides just enough to support C++ ranged-for loops and no more.
    //!   for ( Channel z : channels ) {
    //!     ...
    //!   }
    //! N.B. In particular, operator!= doesn't check that the iterators are iterating the same ChannelSet. This
    //! is because comparing a ChannelSet is slow and we want for loops to have the same performance as the
    //! old "foreach" macro.
    class ChannelSetIterator {
    public:
      ChannelSetIterator(const ChannelSet& channels, bool end = false)
      : _channels(channels)
      , _current(end ? Chan_Black : channels.first())
      {
      }

      ChannelSetIterator(const ChannelSet& channels)
      : _channels(channels)
      , _current(channels.first())
      {
      }

      Channel& operator++()
      {
        _current = _channels.next(_current);
        return _current;
      }

      bool operator!=(const ChannelSetIterator& other)
      {
        return _current != other._current;
      }

      Channel& operator*()
      {
        return _current;
      }

    private:
      const ChannelSet& _channels;
      Channel _current;
    };

    //! Implementation of begin() needed to support ranged-for loops.
    inline ChannelSetIterator begin(const ChannelSet& channels)
    {
      return ChannelSetIterator(channels, false);
    }

    //! Implementation of end() needed to support ranged-for loops.
    inline ChannelSetIterator end(const ChannelSet& channels)
    {
      return ChannelSetIterator(channels, true);
    }

  }
}

#endif
