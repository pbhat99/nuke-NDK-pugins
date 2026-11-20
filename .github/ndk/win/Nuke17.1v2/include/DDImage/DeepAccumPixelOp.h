// Copyright (c) 2011 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_DeepAccumPixelOp_H
#define DDImage_DeepAccumPixelOp_H

#include "DDImage/DeepPixelOp.h"
#include "DDImage/Pixel.h"

namespace DD {
  namespace Image {

  /**
   * Helper class for Deep operators.  The DeepAccumPixelOp is intended
   * for operators that want to work on the accumlated deep samples rather than the individual samples like in DeepPixelOp.
   *
   * This is useful for operators such as a color correct which should not act on individual samples but rather act on the accumulated
   * result and then spread that result over the deep samples.
   *
   * processPixel gets passed the current accumulated sample with the accumulated color unpremultiplied
   *
   * the result of the operation should be put into output ( unpremultipied )
   *
   * The results of these operations will then be unaccumulated and put back into the deep samples.
   *
   * If the alpha was affected then all the deep samples will be reweighted by the new accumulated alpha
   *
   */

    class DDImage_API DeepAccumPixelOp : public DeepPixelOp
    {
      ChannelSet _channels;

    protected:
      // could be private with get/set
      bool _deoverlap = true; // Merge perfrom operation on combined samples.
    public:
      ChannelSet channels() const { return _channels; }

      DeepAccumPixelOp(Node* node);
      ~DeepAccumPixelOp() override;

      bool doDeepEngine(DD::Image::Box box, const ChannelSet& channels, DeepOutputPlane& plane) override;
      void processSample(int y,
                                 int x,
                                 const DD::Image::DeepPixel& deepPixel,
                                 size_t sampleNo,
                                 const DD::Image::ChannelSet& channels,
                                 DeepOutPixel& output) const override
      { assert(false); } // not implemented

      /*** Process the current accumulated pixel
       *
       * @param in - the current accumulated pixel ( unpremultiple color )
       * @param channels - the channels to process
       * @param out - the method should fill this with the result of the operation
       */

      virtual void processPixel( const Pixel& in,
                                 const DD::Image::ChannelSet& channels,
                                 Pixel& output) const = 0;


      /** Adds a channel selector knob, these channels will be processed by processSample
      *
      * Note that this *doesn't* call the DeepPixelOp base class knobs method.
      *
      * In a derived class if you want access to the DeepPixelOp knobs such as mix and limitz as well as the channel selector
      * call DeepPixelOp::knobs as well as DeepAccumPixelOp::knobs.
      *
      */
      void knobs(Knob_Callback f) override;

      /** always requests alpha and the deep channels */
      void in_channels(int, ChannelSet& channels) const override;
    };
  }
}

#endif
