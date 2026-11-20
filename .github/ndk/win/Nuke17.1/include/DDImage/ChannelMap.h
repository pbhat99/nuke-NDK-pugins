#ifndef DDImage_ChannelMap_h
#define DDImage_ChannelMap_h

#include "DDImage/ChannelSet.h"

// Copyright (c) 2011 The Foundry Visionmongers Ltd.  All Rights Reserved.

namespace DD {
  namespace Image {

    /**
     * Helper class for doing quick lookups of channels within a ChannelSet
     */
    class DDImage_API ChannelMap 
    {
      DD::Image::ChannelSet _channels;
      std::vector<int> _channelIndexes;
      size_t _size;

    public:
      /// make an empty channel map
      ChannelMap() : _size(0) { }
      
      /// make a channel map based on a ChannelSet
      explicit ChannelMap(const DD::Image::ChannelSet& channels);

      /// get a reference to the ChannelSet
      operator const ChannelSet&() const
      {
        return _channels;
      }

      /// return whether or not the channel passed in is part of this set
      bool contains(DD::Image::Channel z) const
      {
        return size_t(z) < _channelIndexes.size() && _channelIndexes[z] != -1;
      }

      /// return the number of channels in this map
      size_t size() const
      {
        return _size;
      }

      /// return whether this ChannelMap is empty
      bool empty() const
      {
        return _size == 0;
      }

      /// returns true if this ChannelMap contains all of the channels in want
      bool containsAll(const DD::Image::ChannelSet& want) const
      {
        return _channels.intersection(want) == want;
      }

      /// return the index of the passed in channel within the ChannelMap (e.g. Chan_Green in Mask_RGB is 1),
      /// Chan_Alpha in Mask_RGBA is 3
      int chanNo(Channel z) const
      {
        return _channelIndexes[z];
      }

      /// compare to another ChannelMap. Returns true if they represent the same channels.
      bool operator==(const ChannelMap& chans) const
      {
        return _channels == chans._channels;
      }

      /// compare to a ChannelSet.  Returns true if they represent the same channels.
      bool operator==(const ChannelSet& chans) const
      {
        return _channels == chans;
      }
    };

  }
}

#endif
