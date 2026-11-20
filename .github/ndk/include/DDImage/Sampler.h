// Copyright (c) 2019 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_Sampler_h
#define DDImage_Sampler_h

#include "DDImage/Pixel.h"
#include "DDImage/Iop.h"
#include "DDImage/Filter.h"

namespace DD
{
  namespace Image
  {
    //! Pick a rectangle with the same area as a parallelogram but with one direction
    //! equal to average of the two vectors in that direction.
    //! This is the standard filter area calculation used by Iop and Sampler.
    //!
    //! This can be thought of as a 'poor-man's EWA', calculating the major 'ellipse'
    //! axis but fitted inside a parallelogram using the filter kernel to approximate
    //! the ellipse weighting. Obviously for rotated thin ellipses this is a very poor
    //! approximation which results in over filtering, but that is generally better
    //! than under filtering.
    DDImage_API
    void GetFilterSampleArea(const Vector2& dU, const Vector2& dV, float& filterW, float& filterH);

    //---------------------------------------------------------------------------------

    //! Store information about a position to sample from an image. This class can be
    //! used to support a two-pass sampling approach where the first pass populates a
    //! list of SamplePositions and the second pass uses those positions to perform the
    //! actual sampling. The advantage of this approach is that an accurate bounding box
    //! can be calculated for the region we wish to sample from during the first pass. 
    //! This allows us to create a tile of exactly the size we need when the sampler is
    //! created rather than having to default to the entire input request box.
    struct DDImage_API SamplePosition
    {
      SamplePosition(const Vector2& center, const Vector2& dU, const Vector2& dV, int x);
      SamplePosition(float cx, float cy, float w, float h, int x);

      // Calculate the bounds of the filter for the current position. Used for overall
      // bounding box calculations prior to creating a sampler.
      Box filterBounds(const Filter& filter) const;

      float cx;
      float cy;
      float w;
      float h;
      int x; // Position where sampled pixel should go
    };

    //---------------------------------------------------------------------------------

    //! A class for sampling pixels from an image. It works like a Tile, but with
    //! sample methods. This is generally more efficient than making many calls to
    //! Iop::sample although it may render more than you need from  the input if
    //! you don't sample all of the requested box.
    class DDImage_API Sampler {
    public:
      //! What to do when sampling off the edges of the input
      enum EdgeHandling {
        eEdgeZero,    // Black outside
        eEdgeClamp,   // Repeat pixels at edges
        eEdgeFromIop, // Use Iop's black_outside setting
      };

      // Note: if no filter is specified at construction time then it must be supplied as an argument to any later calls to sample() instead.
      Sampler( Iop* iop, int x, int y, int r, int t, const ChannelSet& channels, Filter* filter = nullptr, EdgeHandling edges = eEdgeFromIop, InterestRatchet* ir = nullptr, bool loadOndemand = true);
      Sampler( Iop* iop, const Box& box, const ChannelSet& channels, Filter* filter = nullptr, EdgeHandling edges = eEdgeFromIop, InterestRatchet* ir = nullptr, bool loadOnDemand = true);
      ~Sampler();

      Iop* iop() const;
      const Format& format() const;
      const ChannelSet& channels() const;
      Filter* filter() const;

      //! Sample a single pixel, with edge clipping.
      float at(int x, int y, Channel z) const;
      void at(int x, int y, const ChannelSet& get, Pixel& out) const;
      //! Same except samples out.channels.
      void at(int x, int y, Pixel& out) const;

      //! Sample a parallelogram. It is centered on \a center and the length of the two axis are
      //! given by \a dU and \a dV. The channels requested by \a get are put into \a out.
      //!
      //! The default implementation turns the parallelogram into a rectangle of the same area
      //! and calls the rectangle sample() function.
      void  sample(const Vector2& center, const Vector2& dU, const Vector2& dV, Filter* filter, const ChannelSet& get, Pixel& out);
      //! Same except samples out.channels.
      void  sample(const Vector2& center, const Vector2& dU, const Vector2& dV, Filter* filter, Pixel& out);

      //! Same except the filter used is the pre-assigned one.
      void  sample(const Vector2& center, const Vector2& dU, const Vector2& dV, const ChannelSet& get, Pixel& out);
      //! Same except the filter used is the pre-assigned one and samples out.channels.
      void  sample(const Vector2& center, const Vector2& dU, const Vector2& dV, Pixel& out);

      //! Sample a rectangular area. Note that \a cx,cy is the \e center of the rectangle, not the corner.
      void  sample(float cx, float cy, float W, float H, Filter* filter, const ChannelSet& get, Pixel& out);
      //! Same except samples out.channels.
      void  sample(float cx, float cy, float W, float H, Filter* filter, Pixel& out);

      //! Same except the filter used is the pre-assigned one.
      void  sample(float cx, float cy, float W, float H, const ChannelSet& get, Pixel& out);
      //! Same except the filter used is the pre-assigned one and samples out.channels.
      void  sample(float cx, float cy, float W, float H, Pixel& out);

      //! Specialized version for texture sampling where \a UV is a texture coordinate between 0..1
      //! to be fitted into the Format, and \a dU and \a dV are its partial derivatives.
      //! ChannelSet \a get is provided separately so that a subset of \a out can be filled without
      //! affecting its assigned ChannelSet, which is ignored.
      void  sampleTextureUV(const fdk::Vec3f& UV,
                            const fdk::Vec3f& dU,
                            const fdk::Vec3f& dV,
                            Filter*           filter,
                            const ChannelSet& get,
                            Pixel&            out);

      //! This version is used for two-pass sampling and uses the filter that was previously specified when the sampler was constructed.
      void  sample(SamplePosition& samplePosition, Pixel& out);

    private:
      std::unique_ptr<class SamplerImpl> _pImpl;
    };
  }
}

#endif

