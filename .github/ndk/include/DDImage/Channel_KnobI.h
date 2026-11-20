// Copyright (c) 2020 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef DDIMAGE_CHANNEL_KNOBI_H
#define DDIMAGE_CHANNEL_KNOBI_H

#include "DDImage/ChannelSet.h"

namespace ndk {
  class LayerAlias;
  class ChannelAlias;
  class ChannelManager;  // In NdkLib
}

namespace DD {
  namespace Image {
    /*! \class DD::Image::Channel_KnobI
     *  \brief The Channel_KnobI class is used to expose the selection handle in DDImage
     */
    class Channel_KnobI
    {
    public:
      Channel_KnobI() = default;
      virtual ~Channel_KnobI() = default;

      //! The current selection's name. If in layer mode it's a layer name, or if single-channel mode it's
      //! the channel name.
      virtual const std::string& selectionName() const = 0;

      //! The current layer description object if in layer select mode.
      //! Note - valid only when bound to an ndk::ChannelManager.
      virtual const ndk::LayerAlias* layerAlias() const = 0;

      //! The current channel description object if in single-channel select mode.
      //! Note - valid only when bound to an ndk::ChannelManager.
      virtual const ndk::ChannelAlias* channelAlias() const = 0;

      enum class SelectionHandling {
        Default,               //!< Channels that are not in the node input get put in the "Other" menu
        AllButNonInputGreyed,  //!< User can select any channel, but those not in the input are greyed out
        AllNormal,             //!< User can select any channel normally
        InputOnly,             //!< Users can only select node input channels
        InternalOnly,          //!< Users can only select internally-created node channels
        InternalOnlyEditing,   //!< Users can select or edit internally-created node channels
      };

      //! Set the selection handling before the Widget is constructed
      virtual void setSelectionHandling(const SelectionHandling) = 0;
      virtual SelectionHandling selectionHandling() const = 0;

      //! Assign a custom ChannelManager for the knob to use, if desired. By default
      //! the knob uses the global default ChannelManager.
      //! For internal use only!
      virtual void setChannelManager(ndk::ChannelManager* manager) = 0;

      //! Get the ChanneSetContext assigned to the knob.
      //! For internal use only!
      virtual ndk::ChannelManager* channelManager() const = 0;

      //! Is the knob using the global Nuke ChannelManager or a custom one?
      //! For internal use only!
      virtual bool usingDefaultGlobalManager() const = 0;


      enum class ChannelIoHandling {
        CollapseAndValidate,  //! Write only the layer name if all channels are in the same layer, and validate names on read.
        AutoCreate,           //! Write all channel names, auto-create channels on read
      };

      //! Control how the knob should write/read serialized channel/layer names. Set this during knob
      //! creation (makeKnobs() = true) to make sure knob loading is affected.
      //! By default the knob does 'CollapseAndValidate' which tries to write only the layer name if all
      //! channels are part of the same layer, and on read the names are checked against the global set.
      //! However only writing the layer name loses individual channel name information and requires the
      //! 'add_layer' and 'add_channel' script directives to supply this information to Nuke -before- the
      //! knob is loaded. In internal-only mode this is not possible because those channel names will not
      //! be part of the global set, so to avoid this the knob uses 'AutoCreate' which writes all channel
      //! names explicitly and skips the name validation on read, auto-creating the channels.
      //! These default behaviors can be changed via this method.
      //! For internal use only!
      virtual void setChannelIoHandling(const ChannelIoHandling) = 0;
      virtual ChannelIoHandling channelIoHandling() const = 0;

      //! Set the subset of channels that the knob can select from. This is useful when the knob
      //! is accessing an internal channel manager that is not coming from any input but created
      //! internally. This is typically used in conjunction with setChannelIoHandling()=AutoCreate
      //! because the channels created by an internal manager are not exposed in the global
      //! channel set, which means the normal method to pre-create channels & layers for knobs does
      //! not work, the channel creation needs to be within the scope of the node the knob is on.
      //! For internal use only!
      virtual void setAvailableChannels(const ChannelSet&) = 0;

      //! Returns the currently assigned available channels set.
      //! This should query the SelectionHandling value to determine what is the source of the
      //! channels presented on the menu for selection.
      //! For internal use only!
      virtual ChannelSet availableChannels() const = 0;
    };
  }
}

#endif
