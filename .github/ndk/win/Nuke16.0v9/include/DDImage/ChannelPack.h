// Copyright (c) 2021 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef DDIMAGE_CHANNELPACK_H
#define DDIMAGE_CHANNELPACK_H

#include "DDImage/ChannelSet.h"

namespace DD {
  namespace Image {


    /*! Helper class for doing faster iterative lookups of channels within a
        ChannelSet and avoid iterating the ChannelSet, which can be slow.

        The channels are packed so that the first Channel in the list is at
        _idx[0] and the last Channel is at _idx[_nChans-1]. The Channel index
        list is a fixed-size array to save on new/delete cost of a std::vector
        and to keep this class POD.

        When constructing or adding from a ChannelSet the Channels will be
        packed in numerical order, so iterating through this array
        will usually match iterating through a ChannelSet via a for loop.
        However if multiple channel adds have been done they are appended
        serially to the end of the list and not kept in sorted Channel order.
        In those cases Chan_Alpha could come before Chan_Red. For most
        channel loops the order of the channels is usually moot.

        Note that while 'Mask_None' is trivially handled if the array size
        is zero, 'Mask_All' is NOT specially handled and the entire array
        would need to be completely filled up to Chan_Last.

        The assumption is that, like Pixel, this class will be used in
        pixel processing loops and not used for storage of ChannelSets,
        which are more compact.
    */
    class DDImage_API ChannelPack
    {
    protected:
      Channel   _idx[Chan_Last + 1];  //!< Array of Channel indices
      uint32_t  _nChans;              //!< Number of assigned indices


    public:
      //! Default ctor is an empty set
      ChannelPack() : _nChans(0) {}

      //! Construct from a ChannelSet. Channel order will be identical to using a for loop.
      explicit ChannelPack(const ChannelSet& channels) { this->set(channels); }

      //! Construct from a single Channel.
      explicit ChannelPack(Channel chan) { this->set(chan); }

      //! Convert back to a ChannelSet.
      operator ChannelSet () const { return ChannelSet(_idx, _nChans); }

      uint32_t size()  const { return _nChans; }
      bool     empty() const { return (_nChans == 0); }
      void     clear() { _nChans = 0; }

      //! Read/write access to channel indices
      Channel  operator [](uint32_t i) const { return _idx[i]; }
      Channel& operator [](uint32_t i)       { return _idx[i]; }

      const Channel* begin() const { return _idx; }
      const Channel* end()   const { return (_idx + _nChans); }

      //! Access to entire Channel array
      Channel* array() { return _idx; }

      //! Assign from a ChannelSet.
      ChannelPack& operator = (const ChannelSet& channels) { this->set(channels); return *this; }

      //! Append channels from a ChannelSet. Duplicate Channels are skipped.
      ChannelPack& operator += (const ChannelSet& channels) { this->add(channels, true); return *this; }

      //! Append a single Channel. Duplicate Channels are skipped.
      ChannelPack& operator += (Channel chan) { this->add(chan, true); return *this; }

      //! Assign from a ChannelSet.
      void set(const ChannelSet& channels) { _nChans = 0; this->add(channels, false); }

      //! Assign from a single Channel.
      void set(Channel chan) { _nChans = 1; _idx[0] = chan; }

      /*! Add channels from a ChannelSet. If \b deduplicate is true duplicate Channels
          are skipped but this method is slightly more expensive to calculate.

          Note that use of this method can cause the channel iteration order to no
          longer match a ChannelSet. ie it's possible to have Chan_Alpha process
          before Chan_Red if Chan_Alpha is added first, then Chan_Red.
      */
      void add(const ChannelSet& channels, bool deduplicate = true);
      /*! Add a single Channel. If \b deduplicate is true the Channel will be skipped
          if it's already in the array but this method is slightly more expensive to
          calculate.

          Note that use of this method can cause the channel iteration order to no
          longer match a ChannelSet. ie it's possible to have Chan_Alpha process
          before Chan_Red if Chan_Alpha is added first, then Chan_Red.
      */
      void add(Channel chan, bool deduplicate = true);

    };

    DDImage_API std::ostream& operator<<(std::ostream&, const ChannelPack&);


    //--------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------

    inline void
    ChannelPack::add(const ChannelSet& channels, bool deduplicate)
    {
      if (deduplicate) {
        const ChannelSet arrayset(_idx, _nChans);
        for (auto z : channels) {
          if (!arrayset.contains(z)) {
            _idx[_nChans++] = z;
          }
        }
      }
      else {
        for (auto z : channels) {
          _idx[_nChans++] = z;
        }
      }
    }
    inline void
    ChannelPack::add(Channel chan, bool deduplicate)
    {
      if (deduplicate) {
        const ChannelSet arrayset(_idx, _nChans);
        if (!arrayset.contains(chan)) {
          _idx[_nChans++] = chan;
        }
      }
      else {
        _idx[_nChans++] = chan;
      }
    }


  }
}


#endif
