// Copyright (c) 2011 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_DeepSample_H
#define DDImage_DeepSample_H

#include "DDImage/ChannelMap.h"
#include "DDImage/ChannelPack.h"
#include "DDImage/DeepPixel.h"

#include <vector>

namespace DD {
  namespace Image {

    /// Helper class representing a container of deep samples, one element per channel in a set.
    /// The sample value is represented in a single precision floating-point type.
    ///
    /// \note
    /// The channel map object is preserved in the class, which allows for indexing
    /// the samples by a given channel. For a lighter version of this class please
    /// consider using \ref DeepLightSample.
    class DDImage_API DeepSample
    {
      ChannelMap _map;

      typedef std::vector<float> SampleContainer;
      SampleContainer _samples;

    public:
      /// Initialise based on a ChannelMap. Set all samples to zero.
      DeepSample(const ChannelMap& map) : _map(map), _samples(_map.size(), 0)
      {
      }

      /// Initialise based on a sample from a DeepPixel. if fetchOrdered is true it will use
      /// getOrderedSample, otherwise getUnorderedSample.
      DeepSample(const ChannelMap& map, const DeepPixel& pixel, int sample, bool fetchOrdered = false) : _map(map)
      {
        _samples.reserve(_map.size());
        const ChannelSet& chans = _map;

        foreach(z, chans) {
          if (pixel.channels().contains(z))
            if (fetchOrdered)
              _samples.push_back(pixel.getOrderedSample(sample, z));
            else
              _samples.push_back(pixel.getUnorderedSample(sample, z));
          else
            _samples.push_back(0);
        }
      }

      void operator=(const DeepSample& other)
      {
        _map = other._map;
        _samples = other._samples;
      }

      void operator=(DeepSample& other)
      {
        _map = other._map;
        _samples = other._samples;
      }

      bool operator==(DeepSample& other) const
      {
        return (_map == other._map) && (_samples.size() == other._samples.size()) &&
               (std::equal(_samples.begin(), _samples.end(), other._samples.begin()));
      }

      /// access the float for a particular channel
      float& operator[](Channel z)
      {
       // mFnAssert(_map.contains(z));
        return _samples[_map.chanNo(z)];
      }

      /// access the float for a particular channel
      const float& operator[](Channel z) const
      {
        //mFnAssert(_map.contains(z));
        return _samples[_map.chanNo(z)];
      }

      SampleContainer::iterator begin()
      {
        return _samples.begin();
      }
      SampleContainer::iterator end()
      {
        return _samples.end();
      }

      /// get number of samples
      size_t size() const
      {
        return _samples.size();
      }
    };

    /// Lightweight helper class representing a container of deep sample, one element per channel in a set.
    /// The sample value is represented in a single precision floating-point type.
    ///
    /// \note
    /// The channel map object isn't preserved in the class. For indexing a sample by a given channel
    /// the same channel map needs to be provided along with the channel, which was used for
    /// the initialization of the class.
    /// For a version of this class which preserves the channel map object, please consider using
    /// \ref DeepSample.
    class DDImage_API DeepLightSample
    {
      typedef std::vector<float> SampleContainer;
      SampleContainer _samples;

    public:
      /// Initialise based on a ChannelMap. Set all samples to zero.
      DeepLightSample(const ChannelMap& map) : _samples(map.size(), 0)
      {
      }

      /// Initialise based on a ChannelPack. Set all samples to zero.
      DeepLightSample(const ChannelPack& pack) : _samples(pack.size(), 0)
      {
      }

      /// Initialise based on a sample from a DeepPixel. If fetchOrdered is true it will use
      /// getOrderedSample, otherwise getUnorderedSample.
      DeepLightSample(const ChannelMap& map, const DeepPixel& pixel, int sample, bool fetchOrdered = false)
      {
        _samples.reserve(map.size());
        const ChannelSet& chans = map;

        for (auto z : chans) {
          if (pixel.channels().contains(z)) {
            _samples.push_back(
              fetchOrdered ? pixel.getOrderedSample(sample, z)
                           : pixel.getUnorderedSample(sample, z)
            );
          }
          else {
            _samples.push_back(0);
          }
        }
      }

      /// Initialise based on a sample from a DeepPixel. If fetchOrdered is true it will use
      /// getOrderedSample, otherwise getUnorderedSample.
      DeepLightSample(const ChannelPack& pack, const DeepPixel& pixel, int sample, bool fetchOrdered = false)
      {
        _samples.reserve(pack.size());

        for (auto z : pack) {
          if (pixel.channels().contains(z)) {
            _samples.push_back(
              fetchOrdered ? pixel.getOrderedSample(sample, z)
                           : pixel.getUnorderedSample(sample, z)
            );
          }
          else {
            _samples.push_back(0);
          }
        }
      }

      /// Copy the specified samples from a DeepPixel.
      /// If fetchOrdered is true it will use getOrderedSample, otherwise getUnorderedSample.
      void copyDeepSample(const ChannelPack& pack, const DeepPixel& pixel, int sample, bool fetchOrdered = false)
      {
        _samples.resize(pack.size());
        size_t i = 0;
        for (auto z : pack) {
          if (pixel.channels().contains(z)) {
            _samples[i] = fetchOrdered ? pixel.getOrderedSample(sample, z) : pixel.getUnorderedSample(sample, z);
          }
          else {
            _samples[i] = 0;
          }
          ++i;
        }
      }

      bool operator==(DeepLightSample& other) const
      {
        return _samples == other._samples;
      }

      /// access the sample for a particular channel
      float& operator()(const ChannelMap& map, Channel z)
      {
        return _samples[map.chanNo(z)];
      }

      /// access the sample for a particular channel
      float operator()(const ChannelMap& map, Channel z) const
      {
        return _samples[map.chanNo(z)];
      }

      SampleContainer::iterator begin()
      {
        return _samples.begin();
      }

      SampleContainer::iterator end()
      {
        return _samples.end();
      }

      SampleContainer::const_iterator begin() const
      {
        return _samples.begin();
      }

      SampleContainer::const_iterator end() const
      {
        return _samples.end();
      }

      /// get number of samples
      size_t size() const
      {
        return _samples.size();
      }
    };

    /// Helper class representing a container of deep sample, where the sample value
    /// is represented in the \ref DeepSample type in no particular order.
    /// The class presents an interface that resembles a DeepPixel, for the convenience
    /// of templated algorithms.
    class DeepSampleVector
    {
      typedef std::vector<DeepSample> DeepSampleContainer;
      DeepSampleContainer _deepSamples;

    public:
      /// get number of samples
      size_t getSampleCount() const
      {
        return _deepSamples.size();
      }

      /// get sample, in order (farthest is first)
      const float& getOrderedSample(int depthIdx, DD::Image::Channel channel) const
      {
        return _deepSamples[_deepSamples.size() - 1 - depthIdx][channel];
      }

      /// get sample, in no particular order
      const float& getUnorderedSample(int depthIdx, DD::Image::Channel channel) const
      {
        return _deepSamples[depthIdx][channel];
      }

      /// get number of samples
      size_t size() const
      {
        return _deepSamples.size();
      }

      /// get sample, in no particular order
      const DeepSample& operator[](int idx) const
      {
        return _deepSamples[idx];
      }

      /// get sample, in no particular order
      DeepSample& operator[](int idx)
      {
        return _deepSamples[idx];
      }

      /// resize the container and allocate new elements
      void resize(size_t newSize, const ChannelMap& channelMap)
      {
        _deepSamples.resize(newSize, DeepSample(channelMap));
      }

      /// append a sample
      void push_back(const DeepSample& deepSample)
      {
        _deepSamples.push_back(deepSample);
      }

      DeepSampleContainer::iterator begin()
      {
        return _deepSamples.begin();
      }

      DeepSampleContainer::iterator end()
      {
        return _deepSamples.end();
      }

      /// erase a given range of samples
      void erase(DeepSampleContainer::iterator begin, DeepSampleContainer::iterator end)
      {
        _deepSamples.erase(begin, end);
      }
    };

  }
}

#endif
