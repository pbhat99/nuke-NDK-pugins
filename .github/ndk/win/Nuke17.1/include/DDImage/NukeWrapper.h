// NukeWrapperIop.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef NukeWrapper_h
#define NukeWrapper_h

#include "DDImage/PixelIop.h"
#include "DDImage/GPUContext.h"
#include "DDImage/PlanarI.h"

#define NUKEWRAPPER_FRINGE 1

namespace DD
{
  namespace Image
  {
    class NukeWrapperImpl;

    /*! \class DD::Image::NukeWrapper

        Provides user-friendly masking and channel-selection controls to
        any operator. This makes the implementation of the operators much
        simpler and allows the basic operators to be much faster when
        used outside Nuke.

        The wrapper uses the in_channels() and out_channels() calls of the
        subclass to figure out what channels it needs from it, intesects
        this with the user-settable channel selector, and only asks the
        subclass to calculate those.  All other channels will be copied
        unchanged from input0(). The user can also select a "mask"
        channel, this is used to mix the original and final results. It
        can also do a constant dissolve between the input and result,
        controlled by the enable knob on the control panel.

        Much work is done to avoid calling the inner operator for any
        data that is not going to be used. Zero channels are detected
        and skipped, and the mask is checked first and only the region
        inside it is requested.

     */
    class DDImage_API NukeWrapper : public Iop, private PlanarI
    {
     friend class NukeWrapperImpl;

     protected:

      Iop* iop;

      // user interface controls:
      ChannelSet channels_;
      Channel unpremult;
      Channel maskChannelMask_;
      Channel maskChannelInput_;
      bool process_mask;
      bool keep_alpha_pulldown;
      float mix_;
      float mix_luminance_;
      int mix_luminance_colorspace_; //!< GUI color space index: Only YCbCr although can allow Lab as well (commented out).
      bool enable_mix_luminance_;
      bool invert_unpremult;
      bool invert_mask;
#if NUKEWRAPPER_FRINGE
      bool fringe;
#endif
      bool inject;

      // stuff from constructor
      int flags;      // remembers isPixelIop, and the noMix(), noChannels(), etc methods
      int mask_input; // input number selected for mask

      // stuff set by _validate():
      ChannelSet outchannels; // iop->out_channels() plus raw channels if not from 0
      ChannelSet enable_mask; // intersection of outchannels and channels_
      Channel maskChannelI;   // actual maskChannel()
      enum MaskFrom { FROM_MASK, FROM_INPUT, FROM_OUTPUT } maskFromI;
      Channel maskInjectI;    // channel to write mask into
      int split_mask_input;   // input_number(mask_input,0)

      // implementation:
      void mix_luminance(const Row &, Row &, ChannelMask, int, int) const;
      void get_from_iop(const Row &, int, int, int, ChannelMask, Row &) const;
      void merge(const float*, const float*, float*, const float*, int, int, int, int) const;
      void mix(const Row &, Row &, ChannelMask, int, int) const;

      void knobsAtTheEnd();
     
    public:

      const MetaData::Bundle& _fetchMetaData(const char* key) override;

      //! Return hints to control when and how this op will be evaluated by the top-down system.
      OpHints opHints() const override;
  
      //! Ensure asset update events are forwarded to the internal op.
      bool resolveKnobAsset(Knob* knob, const std::string& assetPath, std::string& resolvedAsset) override;

      bool updateKnobsFromAsset(Knob* knob) override;

      Iop* wrapped_iop() const { return iop; } //!< Return Iop passed to constructor

      //! Helper function, which unwraps an Op from a NukeWrapper if it is within one and
      //! returns the inner op, or just returns the op.
      static Op* unwrap(Op* op)
      {
        if (NukeWrapper* wrappingIop = dynamic_cast<NukeWrapper*>(op)) {
          return wrappingIop->wrapped_iop();
        }
        return op;
      }

      // overrides every Op virtual function:

      //! Calls editVersionHash() on the Iop.
      Hash editVersionHash() const override;
      //! Calls setEditVersionHash() on the Iop.
      void setEditVersionHash(const Hash& h) override;

      /*! Calls invalidate() on the Iop. */
      void _invalidate() override;

      /*! Calls validate() on the Iop, and figures out the output
          channels based on the channel selectors, mask, and out_channels
          of the iop.
       */
      void _validate(bool) override;

      /*! Calls open() on the wrapped Iop */
      void _open() override;

      /*! Calls close() on the wrapped Iop. */
      void _close() override;

      /*! The knobs this creates are controlled by the flags argument sent to
          the constructor. The iop's knobs() function is called in the middle
          of this, after the channels and before the mask knobs are made.
       */
      void knobs(Knob_Callback) override;

      /*!
         Shows only the relevant maskChannel control depending on whether the mask
         input is connected and being used. This avoids questions about
         what happens when you set both of them (the answer is that the
         maskChannelInput is used and the mask input is ignored).

         This also calls knob_changed() on the wrapped Iop.
       */
      int knob_changed(Knob* k) override;

      // From Op
      bool can_clone() const override;
      bool can_link() const override;

      HandlesMode doAnyHandles(ViewerContext* viewer) override
      {
        return iop->anyHandles(viewer);
      }

      PrevalidateResult doPreValidate(ViewerContext* ctx = nullptr, PreValidateContext* pvc = nullptr) override
      {
        return iop->preValidate(ctx, pvc);
      }
      
      void inputsAttached() override
      {
        iop->inputsAttached();
      }

      int knob_change_finished(Knob* k, bool changedByUser) override
      {
        return iop->knob_change_finished(k, changedByUser);
      }

      bool onAction(const ViewerContext* c, const DD::Image::Flags f, void* d) override
      {
        //pass action down to wrapped IOP
        return iop->onAction(c, f, d);
      }


      /*! Passed through to the iop. */
      void build_handles(ViewerContext*) override;

      /*! Passed through to the iop. */
      void draw_handle(ViewerContext*) override;

      /*! Returns the Iop's result. If the Iop actually has any
         optional inputs, the NukeWrapper's optional mask input is inserted
         after them. */
      int optional_input() const override;

      /*! Returns the result from the iop plus 1 for the mask */
      int minimum_inputs() const override;

      /*! Returns the result from the iop plus 1 for the mask */
      int maximum_inputs() const override;

      /*! Returns the result from the wrapped iop for any input other than
         0. For input 0 only Iop's are allowed, make sure you don't wrap an
         operator that expects something else!  */
      bool test_input(int, Op*) const override;

      /*! Change the corresponding input of the wrapped op. */
      void _setInput(int32_t i, const OpGraphLocation& connection, int32_t base, int32_t offset) override;
      void inputs(int n) override;
      using Iop::inputs;

      /*! Passed through to the wrapped iop. */
      void _knobStorePrep(const OutputContext&) override;
      void _knobStorePost(const OutputContext&, Hash&, Hash&) override;
      void setOutputContext(const OutputContext&) override;
      void append(Hash&) override;

      /*! Returns the result from the wrapped iop for any input other than
         0. For input 0 it uses the default Iop input (a black image)
       */
      Op* default_input(int) const override;

      /*! Returns the result from the iop. The Iop must not return < 1 for n == 0!
         And it currently screws up if anything other than 1 is returned...
       */
      int split_input(int n) const override;

      /*! Returns the result from the iop, except if the Mix slider is turned
         off it returns 1 for input 0 and 0 for all others. */
      float uses_input(int) const override;

      /*!
         Passed through to the iop.
         Any changes it makes to input 0 will also effect the masked-off area
         or the turned-off channels, which may not be what the user expects.
       */
      const OutputContext& inputContext(int n, int offset, OutputContext&) const override;

      /*! Passed through to the wrapped iop. */
      const char*  node_shape() const override;

      /*! Passed through to the wrapped iop. */
      unsigned node_color() const override;

      /*! Passed through to the wrapped iop. */
      const char* node_help() const override;

      /*! Passed through to the wrapped iop. */
      const char* input_label(int, char*) const override;

      /*! Returns the result from the iop. */
      const char* Class() const override;

      /*! Returns the result from the iop. */
      const char* displayName() const override;

      /*! Returns the result from the iop. */
      std::string docsPath() const override;

      // overrides every Iop virtual function:

      /*! Calls request() on the inner Iop, which will usually cause a request()
          to be done of the inputs to this Iop. If the mask or blending is
          enabled this will also call request() on input0() and this usually
          results in a count of 2 and the cache being enabled on the input.

          If the op is a PixelIop request() of the input is called with count
          = 0, which should prevent the cache from being created.
       */
      void _request(int x, int y, int r, int t, ChannelMask, int count) override;

      /*! Calls engine() of the Iop, and (depending on the settings) this
          also reads raw data in from the input and mixes it together with
          the iop output to produce the final image.

          If the operator is a PixelIop, PixelIop::pixel_engine() is called
          directly to avoid having to read a row twice from the input.

          If a mask is enabled and the process-mask flag is off, the mask
          is looked at first, and only the non-zero region is requested with
          the \a x and \a r arguments to the iop's engine. If the mask is
          completely blank no line is asked for at all. Make sure your
          wrapped Iop can handle x and r changing.
       */
      void engine(int y, int x, int r, ChannelMask, Row &) override;
      bool pass_transform() const override;

      /*! Fairly complex, this returns the correct set taking into account
         the in_channels() of the Iop, and the settings of premult, mask, and
         the mix slider.
       */
      void in_channels(int, ChannelSet&) const override;

      /*! Passed through to the wrapped iop. */
      bool firstEngineRendersWholeRequest() const override;

#if 0
      Transform* getTransform()
      {
        if (!maskChannelI && mix_ >= 1 && enable_mask == outchannels && !unpremult) {
          return iop->getTransform();
        }
        return 0;
      }
#endif

      void build_splits() override
      {
        iop->build_splits();
      }

      std::vector<DD::Image::OutputContext> getPrevVersionsWanted() const override
      {
        return iop->getPrevVersionsWanted();
      }

      // constructors:

      /*!
         If the NukeWrapper knows the wrapped Iop is a PixelIop subclass, it will
         call PixelIop::pixel_engine() directly, as this saves a lot of
         memory and cache space because it avoids getting the input pixels
         twice. This also enables the unpremultiply controls.

         The op's engine() will not be called, so you should not override
         the PixelIop::engine(). If the subclass does, be sure to cast back
         to an Iop so that the Iop constructor is called.
       */
      NukeWrapper(PixelIop * op);

      /*!
         The constructor takes a pointer to the Iop to "wrap". By default the
         full capabilities of the NukeWrapper are enabled, you can use the
         noMix(), noChannels(), noMask(), and noPremult() methods to turn
         off controls immediately after the constructor.
       */
      NukeWrapper(Iop * op);
      // you can call these after constructor to modify behavior, don't call otherwise:

      /*!
         You can call this immediately after the constructor to remove the
         "mix" control.  You don't want this if your operator has a mixing or
         amplitude control of it's own.
       */
      NukeWrapper* noMix();

      /*!
         You can call this immediately after the constructor to remove the
         channel selector controls. You don't want this if your operator does
         not naturally operate on \e all channels.

         Returns this to allow you to call this in the same statement as the
         constructor.
       */
      NukeWrapper* noChannels();

      /*!
         You can call this immediately after the constructor to remove the
         "mask" control and the extra mask input. You don't want this if your
         operator has it's own masking or some other way of using a channel
         to control it's amplitude.
       */
      NukeWrapper* noMask();

      /*!
         You can call this immediately after the constructor to enable the
         "luminance mix" control. The control is a slider. Changing the slider
         to any value between 0-1 will mix the original luminance channel (i.e. Y)
         with altered luminance. The value 0 meaning that the altered luminance
         will be used, and 1 being the original luminance.
         Note that for conversion it uses YCbCr color space.
         The default state of this control is off and "mix luminance" value is 0.
       */
      NukeWrapper* mixLuminance();

      /*!
         You can call this immediately after the constructor to remove the
         "unpremult by" control.  You don't want this if your operator requires
         premultiplied images.
       */
      NukeWrapper* noUnpremult();
      NukeWrapper* channels(ChannelSetInit i) { channels_ = i;
                                                return this; }
      NukeWrapper* channelsRGBoptionalAlpha() { channels_ = Mask_RGB;
                                                keep_alpha_pulldown = true;
                                                return this; }

      /*! This deletes the iop! */
      ~NukeWrapper() override;

      // For peeking at the controls:
      ChannelMask channels() const { return channels_; }

      // GPU
      const char* gpuEngine_decl() const override;
      const char* gpuEngine_body() const override;
      Hash gpuEngine_shader_hash_at(double time) override;
      int gpuEngine_getNumRequiredTexUnits() const override;
      void gpuEngine_GL_begin(GPUContext* context) override;
      void gpuEngine_GL_end(GPUContext* context) override;

      /**
       * get the PlanarI for this NukeWrapper; this returns NULL if 
       * it prefers row-based access; or a pointer to the PlanarI
       * describing the plane preferences if it prefers planar access
       */
      PlanarI* getPlanarI() override;

      /**
       * get the image data as a plane; will pass-through if possible,
       * otherwise may fall back to row-based processing.
       */
      void doFetchPlane(DD::Image::ImagePlane& ip) override;

      /**
      * get a label to display in the library version info for Node
      */
      std::string getLibraryName() const override { return iop->getLibraryName(); }

      /**
      * get a version to display in library version info for the Node
      */
      Op::VersionInfo getLibraryVersion() const override { return iop->getLibraryVersion(); }

    private:
      /** is Mix Luminance going to modify the output */
      inline bool isMixLuminanceUsed() const { return enable_mix_luminance_ && mix_luminance_ > 0.f; }
      /** is any Mixing involved in modifying the output */
      inline bool isMixUsed() const { return mix_ < 1 || isMixLuminanceUsed(); }
      /** is this NukeWrapper just a simple passthrough to the inner Iop with the current knob settings */
      bool isPassThrough() const;

      PlanarI::PackedPreference packedPreference() const override;
      size_t getStripeCount() const override;
      Box getStripeBox(int idx) const override;
      size_t rowToStripeIndex(int y) const override;

      // Updates the GPU shader if required.
      void updateShader();

      std::unique_ptr<NukeWrapperImpl> _pImpl;
    };

  }
}

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
