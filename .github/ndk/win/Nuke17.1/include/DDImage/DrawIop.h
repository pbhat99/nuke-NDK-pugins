// DrawIop.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_DrawIop_h
#define DDImage_DrawIop_h

#include "DDImage/PixelIop.h"

#define DRAWIOP_HAS_RAMP 1

namespace DD
{
  namespace Image
  {

    /*! \class DD::Image::DrawIop
        This is a base class for operations that draw a black and white
        image (such as a shape, text, PostScript, etc).  This base class
        provides a wrapper so that the image can be drawn into any channel,
        inverted, or merged or intersected with an input channel.
     */
    class DDImage_API DrawIop : public PixelIop
    {
    public:
      ChannelSet channelset;
      ChannelSet premult;
      float color1[4];
#if DRAWIOP_HAS_RAMP
      float color0[4];        //!< Colors for the ramp
      int ramp_type;
      double x0, y0, x1, y1;  //!< Control points for ramp
      enum { NORAMP, LINEAR, SMOOTH0, SMOOTH1, SMOOTH };
      float X, Y;             //!< Slopes of lines through the control points
      float deltaX;           //!< Rate of color change horizontally
      void calculate_guides();
#endif
      float opacity_;
      Channel maskChannelMask_;
      Channel maskChannelInput_;
      bool replace;
      bool invert;
      bool invert_mask;
      bool inject;
      int cliptype;
    private:
      Channel maskChannelI; // Mask after being fixed by validate
      enum MaskFrom { FROM_MASK, FROM_INPUT } maskFromI;
      Channel maskInjectI;  // Where to write mask to
      char validated;
    protected:

      /*! This default version of _validate() assumes the drawing completely
          fills the union of the incoming bounding box and format. The bbox controls
          can be used to reduce the output size. Most subclasses should figure out
          the actual area they will draw and call the other _validate().
       */
      void _validate(bool) override;

      /*!
         A subclass implementation of _validate() should call this.
         The \a xywh paramenters are a box that surrounds the pixels the
         drawing will occupy.

         This will fill in all the info() and out_channels() based on this
         bounding box and the settings of the knobs on the DrawIop.
         Typically the bounding box will be made at least 1 pixel
         larger to include a black edge and then unioned with the
         incoming bounding box. out_channels() will be set to those
         in the channel mask control.
       */
      void _validate(bool, int x, int y, int r, int t);

      /*!
         The engine calls draw_engine() to get the black/white image and
         then merges the result with the input channels and using the
         settings of the color and invert flags. Somewhat complex as
         it tries hard to detect and mark lines that are cleared to zero,
         because other operators that use the output as a mask can save
         time when they know it is zero.
       */
      void pixel_engine(const Row &, int y, int x, int r, ChannelMask, Row &) override;
      void in_channels(int, ChannelSet&) const override;
#if DRAWIOP_HAS_RAMP
      HandlesMode doAnyHandles(ViewerContext*) override;
      void build_handles(ViewerContext*) override;

      /*! Draws dotted lines to indicate the ramp. */
      void draw_handle(ViewerContext*) override;
#endif

      /*!
         Shows only the relevant maskChannel control depending on whether the mask
         input is connected and being used. This avoids questions about
         what happens when you set both of them (the answer is that the
         maskChannelInput is used and the mask input is ignored).
       */
      int knob_changed(Knob*) override;
    public:

      /*! \fn bool DrawIop::draw_engine(int y, int x, int r, float* buffer)

         The subclass only needs to provide this method that produces the
         image in monochrome, writing the locations between \a x and \a r in
         the \a buffer with values between 0.0 and 1.0.

         This should return true if the row intersects the drawing.  It can
         return false to indicate the row did not intersect the picture, in
         this case the buffer does not need to be filled with zeros and
         DrawIop will process the line much faster.
       */
      virtual bool draw_engine(int y, int x, int r, float* buffer) = 0;
      DrawIop(Node * node);

      /*! Your subclass should call this to insert knobs that should be
          before the controls for your shape. Currently this is the
          layer selectors, and the invert and replace checkmarks.
       */
      void input_knobs(Knob_Callback);

      /*! Your subclass should call this to insert knobs that should be
          after the controls for your shape.
       */
      void output_knobs(Knob_Callback, bool ramp = true);
      int optional_input() const override;
      int minimum_inputs() const override;
      int maximum_inputs() const override;
      Op* default_input(int) const override;
      float uses_input(int) const override;
      const char* input_label(int, char*) const override;
      unsigned node_color() const override;
    };

  }
}

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
