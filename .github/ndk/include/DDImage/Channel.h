// Channel.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_Channel_h
#define DDImage_Channel_h

#include "DDImage_API.h"
#include <iostream>

namespace DD
{
  namespace Image
  {

    /*!
       Channel ID numbers. These are used to index arrays of pixel data, such as in
       the DD::Image::Row object.

       A plugin can use these pre-defined ones. Nuke will also create new ones that
       are outside this set. DD::Image::getName() will return a string name of these.

       A plugin can create channels by using DD::Image::getChannel().  DD::Image::findChannel()
       looks up channels by name but without creating them.

       See also the class DD::Image::ChannelSet.
     */
    enum Channel {
      Chan_Black = 0,
      Chan_Red   = 1,
      Chan_Green = 2,
      Chan_Blue  = 3,
      Chan_Alpha = 4,
      Chan_Z     = 5, //! used mostly for 2d ops, 1/distance (so, 0 = infinitely far away)
      Chan_U     = 6,
      Chan_V     = 7,
      Chan_Backward_U = 8,
      Chan_Backward_V = 9,
      Chan_Stereo_Disp_Left_X = 10,
      Chan_Stereo_Disp_Left_Y = 11,
      Chan_Stereo_Disp_Right_X = 12,
      Chan_Stereo_Disp_Right_Y = 13,
      Chan_Mask  = 14,
      Chan_RotoPaint_Mask  = 15,
      Chan_DeepFront = 16, //! used mostly for deep ops, distance to front of sample (0=at camera, inf=infinitely far away)
      Chan_DeepBack = 17,  //! used mostly for deep ops, distance to front of sample (0=at camera, inf=infinitely far away)
      Chan_Mask_PlanarTrack = 18,
      Chan_Mask_SplineWarp  = 19,
      Chan_Coverage         = 20, //! pixel or sample coverage weight
      Chan_Cutout_Alpha     = 21, //! used to preserve alpha while performing cutout operations
      Chan_Cutout_Depth     = 22, //! used to preserve depth while performing cutout operations
      //
      Chan_Unused = 23,
    };

    /*!
       Adds one to the Channel and returns it. */
    inline void incr(Channel& c) { c = Channel(c + 1); }

    /*!
       This prints getName(channel), but prints the number if the name is null.
     */
    DDImage_API std::ostream& operator<<(std::ostream&, Channel);

    /*!

       Find or create a channel with this name. Plugins can call this to
       get the number for any channels they want to output.

       For instance if you output something called "shadow" for each pixel,
       you may want to call this with "shadow" as an argument in order to
       get what channel to write to. All calls to get "shadow" will return
       the same number. If you output an xyz triplet called "position" you
       may want to call this with "position.x", "position.y", and
       "position.z".

       Text before the last period is used to create or find the layer. If
       there is no period, it makes a layer called "other" (this may change
       in future versions). If \a sort is true, then the channel is
       insertion-sorted into the layer so that r,g,b,a are first and then
       the rest of the channels are in alphabetical order. If \a sort is
       false then the channel is appended to the end of the layer. (this
       sorting was needed because the exr library produces the channels in
       a random order due to the use of a hash table).
     */
    DDImage_API Channel getChannel(const char* name, bool sort = true);

    /*!
       Get the number of channels currently registered.
     */
    DDImage_API size_t getChannelCount();

    /*!

       Find a channel with this name. Plugins can call this to
       get the number for any channels they want to output.
     */
    DDImage_API Channel findChannel(const char* name);

    /*!
       Returns a string name to print for the channel, or NULL if it
       cannot figure it out. This is a full "layer.chan" name.
     */
    DDImage_API const char* getName(Channel);

    /*!
       Returns the name of the layer this channel belongs to, or NULL
       if it can't find a layer.
     */
    DDImage_API const char* getLayerName(Channel);

    /*!
       Returns 0 if this channel is "red", 1 if it is "green", 2 if it is
       "blue", and 3 for all other possibilities.
     */
    DDImage_API unsigned colourIndex(Channel, const char* layer = nullptr);

    /*!

       Return a channel such that colourIndex(n) returns the given index,
       and it belongs to the same layer as \a channel. This is used to
       find the related colours. You can also use this to find all the
       channels in a layer by incrementing the index until this returns
       Chan_Black.

       Returns Chan_Black if no such channel exists.

       Also see DD::Image::ChannelSet::addBrothers().
     */
    DDImage_API Channel brother(Channel, unsigned index);


    /*!
       Compares two channels to determine ordering based on purpose.
       For example, a channel with purpose Chan_Red will order before one
       with purpose Chan_Green, regardless of the actual channel map ordering.
     */
    DDImage_API bool compareChannels(Channel a, Channel b);

    /*!
       Bitmasks of various combinations of the predefined Channel numbers.
       These values can be used in most places a ChannelSet is expected.
       They will be cast to ChannelSet or used directly by the code.
     */
    enum ChannelSetInit {
      Mask_None  = 0,
      Mask_Red   = 1 << (Chan_Red - 1),
      Mask_Green = 1 << (Chan_Green - 1),
      Mask_Blue  = 1 << (Chan_Blue - 1),
      Mask_Alpha = 1 << (Chan_Alpha - 1),
      Mask_Z     = 1 << (Chan_Z - 1),
      
      Mask_DeepBack = 1 << (Chan_DeepBack - 1),
      Mask_DeepFront = 1 << (Chan_DeepFront - 1),

      Mask_Deep = (Mask_DeepBack | Mask_DeepFront),

      Mask_U     = 1 << (Chan_U - 1),
      Mask_V     = 1 << (Chan_V - 1),
      Mask_Backward_U = 1 << (Chan_Backward_U - 1),
      Mask_Backward_V = 1 << (Chan_Backward_V - 1),

      Mask_Stereo_Disp_Left_X     = 1 << (Chan_Stereo_Disp_Left_X - 1),
      Mask_Stereo_Disp_Left_Y     = 1 << (Chan_Stereo_Disp_Left_Y - 1),
      Mask_Stereo_Disp_Right_X     = 1 << (Chan_Stereo_Disp_Right_X - 1),
      Mask_Stereo_Disp_Right_Y     = 1 << (Chan_Stereo_Disp_Right_Y - 1),

      Mask_RGB   = Mask_Red | Mask_Green | Mask_Blue,
      Mask_RGBA  = Mask_RGB | Mask_Alpha,
      Mask_UV    = Mask_U | Mask_V,
      Mask_MoVecForward = Mask_U | Mask_V,
      Mask_MoVecBackward = Mask_Backward_U | Mask_Backward_V,
      Mask_MoVec = Mask_U | Mask_V | Mask_Backward_U | Mask_Backward_V,

      Mask_Stereo_Disp_Left = Mask_Stereo_Disp_Left_X | Mask_Stereo_Disp_Left_Y,
      Mask_Stereo_Disp_Right = Mask_Stereo_Disp_Right_X | Mask_Stereo_Disp_Right_Y,
      Mask_Stereo_Disp = Mask_Stereo_Disp_Left | Mask_Stereo_Disp_Right,

      Mask_Mask  = 1 << (Chan_Mask - 1),
      Mask_RotoPaint_Mask  = 1 << (Chan_RotoPaint_Mask - 1),
      Mask_PlanarTrack = 1 << (Chan_Mask_PlanarTrack - 1),
      Mask_SplineWarp = 1 << (Chan_Mask_SplineWarp - 1),

      Mask_Coverage = 1 << (Chan_Coverage - 1),
      Mask_Cutout_Alpha = 1 << (Chan_Cutout_Alpha - 1),
      Mask_Cutout_Depth = 1 << (Chan_Cutout_Depth - 1),
      Mask_Cutouts = Mask_Cutout_Alpha | Mask_Cutout_Depth,

#ifdef NUKE_OBJECT_ID
      Mask_Object_ID  = 1 << (Chan_Object_ID - 1),
#endif

      Mask_Builtin = Mask_RGBA | Mask_Z | Mask_Deep | Mask_MoVec | Mask_Stereo_Disp |
                     Mask_Mask | Mask_RotoPaint_Mask | Mask_PlanarTrack | Mask_SplineWarp |
                     Mask_Coverage | Mask_Cutouts
#ifdef NUKE_OBJECT_ID
        | Mask_Object_ID
#endif
      ,

      Mask_All   = 0xFFFFFFFF //!< Special value that makes the ChannelSet contain all possible channels
    };

    inline ChannelSetInit operator|(ChannelSetInit a, ChannelSetInit b)
    {
      return ChannelSetInit(unsigned(a) | unsigned(b));
    }

    inline ChannelSetInit operator& (ChannelSetInit a, ChannelSetInit b)
    {
      return ChannelSetInit(unsigned(a) & unsigned(b));
    }

    inline ChannelSetInit operator& (ChannelSetInit a, Channel b)
    {
      return b ? ChannelSetInit(unsigned(a) & (1U << (b - 1))) : Mask_None;
    }

  }
}

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
