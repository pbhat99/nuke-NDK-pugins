// Copyright (c) 2018 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_DeepPixel_h
#define DDImage_DeepPixel_h

#include "DDImage/ChannelSet.h"
#include "DDImage/ChannelMap.h"

#include "Build/fnMacros.h"
#include <assert.h>
#include <stdlib.h>

#include <vector>

namespace DD {
  namespace Image {

    /**
     * DeepPixel is a helper class for accessing DeepPlanes.  It represents all the data
     * at a particular x/y position in a DeepPlane.  It does not contain its own copy
     * of the data, and remains valid for only as long as the DeepPlane it is associated
     * with exists.
     */
    class DDImage_API DeepPixel
    {
      friend class DeepPlane;

    public:
      /**
       * which ordering the pixel data is in.
       */
      enum Ordering
        {
          eUnordered, //! unordered - the samples are in no particular depth order
          eZAscending, //! z ascending - the samples with lowest Z are first
          eZDescending, //! z descending - the samples with highest Z are first
      };

      /**
       * Swap the pixel ordering to the other way around.  Unordered remains
       * unordered, but Z-ascending and Z-descending swap.
       */
      static Ordering SwapOrdering(Ordering ordering)
      {
        switch (ordering) {
        case eUnordered: return eUnordered;
        case eZAscending: return eZDescending;
        case eZDescending: return eZAscending;
        }
        ::abort();
        return eUnordered;
      }

      /**
       * Constructor.  Usually not accessed directly, invoked by DeepPlane::getPixel
       */
      DeepPixel(const DD::Image::ChannelMap& channels, const float* pixels, size_t dataCount, Ordering ordering): 
        _channels(channels),
        _data(pixels),
        _dataCount(dataCount),
        _chanCount(_channels.size()),
        _ordering(ordering)
      {
        _sampleCount = (_chanCount > 0 ) ? _dataCount / _chanCount : 0;
        _doneSampleOrder = false;

        if (dataCount)
          mFnAssert(pixels);
      }

    protected:

      /**
       * The channels that this DeepPixel is for.
       */
      const DD::Image::ChannelMap& _channels;

      /**
       * The raw data for the DeepPixel
       */
      const float* _data;

      /**
       * the amount of floats pointed to by _data for this pixel
       */
      size_t _dataCount;

      /**
       * the number of channels present in _channels.
       */
      size_t _chanCount;

      /**
       * The number of samples (equivalent to _dataCount / _chanCount)
       */
      size_t _sampleCount;

      /**
       * The order in which the samples are stored (unordered, z-ascending,
       * z-descending)
       */
      Ordering _ordering;

      /**
       * An index for accessing the samples in z-order
       */
      mutable std::vector<size_t> _sampleOrder;

      /**
       * Whether or not the _sampleOrder index has been
       * built
       */
      mutable bool _doneSampleOrder;
      
      /**
       * Build up the sample order index
       */
      void makeSampleOrder() const;
      
    public:
      /**
       * Get the channels that this DeepPixel contains
       */
      const ChannelMap& channels() const
      {
        return _channels;
      }
      
      /**
       * Get the number of samples
       */
      size_t getSampleCount() const
      {
        return _sampleCount;
      }

      /**
       * Get the pointer to the data
       */
      const float* data() const
      {
        return _data;
      }

      /**
       * Get the total number of data elements
       */
      size_t getDataCount() const
      {
        return _dataCount;
      }

      /**
       * Get a pointer to the pixel data for a particular sample number.  Data will
       * be the channels present in channels(), ordered by numeric channel id.  Samples
       * are in whatever order they happen to be in the pixel.
       */
      const float* getUnorderedSample(size_t sampNo) const
      {
        return _data + (sampNo * _chanCount);
      }
      
      /**
       * Get a particular sample and channel.  Samples
       * are in whatever order they happen to be in the pixel.
       */
      const float& getUnorderedSample(size_t sampNo, DD::Image::Channel channel) const
      {
        mFnAssert(_channels.contains(channel));
        if (!_channels.contains(channel) || channel == Chan_Black) {
          static const float zero = 0;
          return zero;
        }
        return _data[sampNo * _chanCount + _channels.chanNo(channel)];
      }

      /**
       * Get a particle sample (by depth) and channel. Samples will be ordered
       * in descending order.
       */
      const float& getOrderedSample(size_t depthIdx, DD::Image::Channel channel) const;
    };

    /**
     * DeepOutputPixel is used to write to DeepInPlaceOutputPlanes.
     *
     * It allows direct access to the samples in DeepPlaneData.
     * At a particular x,y position in a DeepOutputPlane, a DeepOutputPixel provides
     * methods for direct access to the plane's data. It remains valid for only
     * as long as the DeepOutputPlane it is associated with exists.
     */
    class DDImage_API DeepOutputPixel : public DeepPixel
    {
    public:
      /**
      * DeepOutputPixel constructor
      */
      DeepOutputPixel(const DD::Image::ChannelMap& channels, float* pixels, size_t dataCount, Ordering ordering)
        : DeepPixel(channels, pixels, dataCount, ordering)
      {

      }

      /**
      * Get a writable the pointer to this DeepPixel's DeepOutputPlane's data
      */
      float* writable() const;

      /**
      * Get a writable pointer to the pixel data for a particular sample number.  Data will
      * be the channels present in channels(), ordered by numeric channel id.  Samples
      * are in whatever order they happen to be in the pixel.
      */
      float* getWritableUnorderedSample(size_t sampNo)
      {
        return writable() + (sampNo * _chanCount);
      }

      /**
      * Get a writable reference to a particular sample's channel.  Samples
      * are in whatever order they happen to be in the pixel.
      */
      float& getWritableUnorderedSample(size_t sampNo, DD::Image::Channel channel)
      {
        mFnAssert(_channels.contains(channel));
        if (!_channels.contains(channel) || channel == Chan_Black) {
          static float zero = 0;
          return zero;
        }
        return writable()[sampNo * _chanCount + _channels.chanNo(channel)];
      }

      /**
      * Get a writable reference to a particular sample (by depth) and channel.  Samples will be ordered so
      * that if depthIdx == 0, it will fetch the sample with the lowest Z.
      */
      float& getWritableOrderedSample(size_t depthIdx, DD::Image::Channel channel);
    };
  }
}

#endif
