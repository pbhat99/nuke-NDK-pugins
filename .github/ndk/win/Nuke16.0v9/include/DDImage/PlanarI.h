#ifndef DDIMAGE_PLANARI_H
#define DDIMAGE_PLANARI_H

#include "DDImage/DDImage_API.h"
#include "DDImage/Box.h"
#include "DDImage/ChannelSet.h"
#include "DDImage/Deprecation.h"

#include <stdlib.h>

namespace DD {
  namespace Image {

    // Forward Declarations
    class LocalPlaneCache;

    /**
     * PlanarI is a mix-in class to be implemented by Iops which prefer to be accessed in a
     * planar way (i.e. with fetchPlane)
     *
     * It is not itself an Iop because the subclasses might want to inherit from some other
     * subclass of Iop. However doing so will disable caching and is not recommended; for
     * optimal perfomance always inherit from PlanarIop.
     */
    class DDImage_API PlanarI
    {
      friend class PlaneCacheI;

    protected:
      virtual ~PlanarI() = default;

    public:
      typedef ChannelSet PlaneID;
      typedef std::set<PlaneID> PlaneSet;

      enum PackedPreference {
        ePackedPreferenceNone,
        ePackedPreferencePacked,
        ePackedPreferenceUnpacked,
      };

      /** whether the Iop prefers to output packed or unpacked data.  If the
       *  Iop is neutral it should return ePackedPreferenceNone and then the
       *  decision will be made on whether other criteria, such as what the
       *  downstream ops prefer to consume */
      virtual PackedPreference packedPreference() const = 0;

      /**
       * get the number of blocks
       */
      virtual size_t getStripeCount() const = 0;

      /**
       * get the size of box /idx/
       */
      virtual Box getStripeBox(int idx) const = 0;

      /**
       * get the box number that row /y/ is in
       */
      virtual size_t rowToStripeIndex(int y) const = 0;

      /**
       * Return the full output box covered by the stripes.
       *
       * Uses getStripeBox() and getStripeCount().
       */
      Box getOutputBox() const;

      /**
       * Get the ChannelSet which is the intersection between the
       * the plane containing the channel passed in and the channels
       * that have been requested.
       */
      PlaneID getRequestedPlaneFromChannel(Channel channel);

      /**
       * Get the set of ChannelSets which are the intersection between
       * the ChannelSet passed in, the planes containing the channels
       * from that ChannelSet, and the channels that have been requested.
       */
      PlaneSet getAllPlanes(const ChannelSet& channels);

      /**
       * Get the ChannelSet which is the intersection between the
       * the ChannelSet passed in, the planes containing the channels
       * from that ChannelSet, and the channels that have been requested.
       */
      ChannelSet getAllChannels(const ChannelSet& channels);

      /**
       * Access the cache for this Planar interface
       * the default implementation returns NULL, implying no caching, which
       * can be very slow.
       */
      virtual LocalPlaneCache* getCache();

      /**
       * Return whether or not the stripes can be rendered on multiple threads in
       * parallel. If threading is done within the stripes, you should return false
       * from this to avoid an explosion in the number of threads being used for the
       * render.
       *
       * In future, if you return true from this you might be asked to render multiple
       * stripes at once on different threads. You should therefore take care that your
       * rendering function (renderStripe, in the case of a PlanarIop) is thread-safe.
       *
       * NOTE that this is provided for future compatibility only, and for the time being
       * if you want guaranteed multi-threading you should implement it yourself within
       * renderStripe and return false from this function.
       */
      virtual bool stripesCanBeRenderedInParallel() { return false; }

      /**
       * override this if you want your planes to be different to the default.
       *
       * by default Nuke will call renderStripe()/doFetchPlane() once for each layer.
       * You define its belief about what a 'layer' is here.  for example, if
       * you return info().channels(), it will render all your channels at once; if
       * you return chan it will render each channel separately.
       */
      virtual PlaneID getPlaneFromChannel(Channel chan);
    };

  }
}

#endif
