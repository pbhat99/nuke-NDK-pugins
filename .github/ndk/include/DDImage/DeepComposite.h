// Copyright (c) 2011 The Foundry Visionmongers Ltd. All Rights Reserved.
#ifndef DDIMAGE_DEEPCOMPOSITE_H
#define DDIMAGE_DEEPCOMPOSITE_H

#include "DDImage/ChannelMap.h"
#include "DDImage/DeepSample.h"

#include <vector>

namespace DD {
  namespace Image {

    class ChannelMap;
    class DeepPixel;
    class DeepOutPixel;
    class Row;

    using CombineFunc = DeepSample (*)(const DD::Image::ChannelMap& channels, DeepSample sampleA, DeepSample sampleB);

    // a function to combine perfectly overlapping samples sampleA[Chan_DeepFront] == sampleB[Chan_DeepFront] && sampleA[Chan_DeepBack] == sampleB[Chan_DeepBack]
    using CombineOperation = DeepLightSample (*)(const DD::Image::ChannelMap& channels, const DeepLightSample& sampleA, const DeepLightSample& sampleB);

    // Combine Functions for perfectly overlapping samples
    //! Add two samples together, only adding the samples from the second sample if they are in the provided ChannelMap
    //! @param chanMap channels from sampleB to add to sampleA
    //! @param sampleA starting sample data
    //! @param sampleB data to add
    //! @return sample where each channel is the sum of sampleA and sampleB for that specific channel
    DDImage_API DeepLightSample PlusOperation(const ChannelMap& chanMap, const DeepLightSample& sampleA, const DeepLightSample& sampleB);

    //! Merge samples together. If the alpha of both is 1, then averages the values per channel. If the alpha of one of
    //! them is 1, then that one will override the result. If both of them don't have an alpha of 1, then the result is
    //! a weighted sum of sampleA and sampleB.
    //! @param chanMap the channels to merge together
    //! @param sampleA first sample to merge sets the deep front and back data
    //! @param sampleB second sample to merge
    //! @return a new merged sample
    DDImage_API DeepLightSample MergeOperation(const ChannelMap& chanMap, const DeepLightSample& sampleA, const DeepLightSample& sampleB);

    //! Combines overlapping samples from within /deepPixel/ into a number of non-overlapping samples, output in /pixels/
    //! will omit any samples after a sample with alpha 1.
    //! @param channels channels to combine together
    //! @param deepPixel pixel with sample data to combine together
    //! @param pixels the output pixel
    DDImage_API void CombineOverlappingSamples(const ChannelMap& channels, const DeepPixel& deepPixel, DeepSampleVector& pixels);
    DDImage_API void CombineOverlappingSamples(const ChannelMap& channels, const DeepSampleVector& deepPixel, DeepSampleVector& pixels);
    DDImage_API void CombineOverlappingSamples(const ChannelMap& channels, const DeepPixel& deepPixel, DeepOutPixel& pixels);

    //! Splits volumes that partially overlap - perfectly overlapping volumes aren't split
    //! @param channels channels to de-overlap
    //! @param deepPixel pixel to de-overlap
    //! @return DeepOutPixel with split volumes
    DDImage_API DeepOutPixel DeoverlapPixelSamples(const ChannelMap& channels, const DeepPixel& deepPixel);

    //! Splits volumes that partially overlap - perfectly overlapping volumes aren't split
    //! @param channels channels to de-overlap
    //! @param deepPixel pixel to de-overlap
    //! @param deepOutPixel the result DeepOutPixel with split volumes
    //! @param samples temporary vector of DeepLightPixels used internally in DeoverlapPixelSamples.
    //!                The same vector can be passed in to avoid reallocation when DeoverlapPixelSamples is called multiple times.
    DDImage_API void DeoverlapPixelSamples(const ChannelMap& channels, const DeepPixel& deepPixel, DeepOutPixel& deepOutPixel, std::vector<DeepLightSample>& samples);

    //! Merge perfectly overlapping samples - where the Deep_Front and Deep_Back are the same for the sample per channel
    //! @param channels channels to merge together
    //! @param in_pixel the pixel to merge the samples in
    //! @param f the function to apply in the case of overlapping samples - for example, PlusOperation, MergeOperation
    //! @return DeepOutPixel with the samples merged according to the function provided
    DDImage_API DeepOutPixel MergeOverlappingPixelSamples(const DD::Image::ChannelSet& channels, const DeepPixel& in_pixel, CombineOperation f);

    //! Merge perfectly overlapping samples - where the Deep_Front and Deep_Back are the same for the sample per channel
    //! @param channels channels to merge together
    //! @param in_pixel the pixel to merge the samples in
    //! @param deepOutPixel the result DeepOutPixel with the samples merged according to the function provided
    //! @param samples temporary vector of DeepLightPixels used internally in MergeOverlappingPixelSamples.
    //!                The same vector can be passed in to avoid reallocation when MergeOverlappingPixelSamples is called multiple times.
    //! @param f the function to apply in the case of overlapping samples - for example, PlusOperation, MergeOperation
    DDImage_API void MergeOverlappingPixelSamples(const DD::Image::ChannelSet& channels, const DeepPixel& in_pixel,  DeepOutPixel& deepOutPixel, std::vector<DeepLightSample>& samples, CombineOperation f);

    //! Composite the samples together. Samples can be a DeepPixel or a DeepSampleVector.
    //! @param samples samples to composite
    //! @param chans channels to composite
    //! @param row row to write into
    //! @param x x coordinate of the row to start writing into
    //! @param Zpix float address to output the percentage of the alpha relative to the total deep z
    //! @param Zfrontpix float address to output the largest Deep_Front of the samples
    DDImage_API void CompositeSamples(const DeepPixel& samples, const ChannelSet& chans, Row& row, int x, float* Zpix, float* Zfrontpix);
    DDImage_API void CompositeSamples(const DeepSampleVector& samples, const ChannelSet& chans, Row& row, int x, float* Zpix, float* Zfrontpix);

    //! Composite the samples together. Samples can be a DeepPixel or a DeepSampleVector.
    //! @param samples samples to composite
    //! @param chans channels to composite
    //! @param row row to write into
    //! @param x x coordinate of the row to start writing into
    //! @param Zpix float address to output the percentage of the alpha relative to the total deep z
    //! @param Zfrontpix float address to output the largest Deep_Front of the samples
    //! @param Zbackpix float address to output the smallest Deep_Back of the samples
    DDImage_API void CompositeSamples(const DeepPixel& samples, const ChannelSet& chans, Row& row, int x, float* Zpix, float* Zfrontpix, float* Zbackpix);
    DDImage_API void CompositeSamples(const DeepSampleVector& samples, const ChannelSet& chans, Row& row, int x, float* Zpix, float* Zfrontpix, float* Zbackpix);

    //! Check whether any samples overlap.  Samples can be a DeepPixel or a DeepSampleVector.
    //! @param t samples to check
    //! @return true if any of the samples overlap, false if the samples don't have Deep_Front and Deep_Back or don't overlap
    DDImage_API bool DetectOverlappingSamples(const DeepSampleVector& t);
    DDImage_API bool DetectOverlappingSamples(const DeepPixel& t);

  }
}

#endif
