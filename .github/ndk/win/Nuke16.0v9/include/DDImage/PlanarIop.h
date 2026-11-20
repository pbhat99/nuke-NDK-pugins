#ifndef PlanarIop_h
#define PlanarIop_h

#include "DDImage/Iop.h"
#include "DDImage/ImagePlane.h"
#include "DDImage/RequestData.h"
#include "DDImage/PlanarI.h"
#include "DDImage/RequestOutput.h"

namespace DD {
  namespace Image {

    class LocalPlaneCache;
    class ImagePlaneCacheImpl;

    /// Base class for an Iop which strongly wishes to output planar data,
    /// possibly with striping.  This implements PlanarI to hint as to how the
    /// data should be accessed, and adds ImagePlaneCache (privately) to provide
    /// a cache.
    class DDImage_API PlanarIop : public Iop, public PlanarI
    {
    public:
      PlanarIop(Node* node);
      ~PlanarIop() override;

      /// The function _request() on Iops is eventually going to be deprecated,
      /// because it is stateful.
      /// 
      /// For PlanarIops you should never override it.  Instead, please implement
      /// this getRequests() function.  It should place the regions to be requested
      /// to the RequestOutput passed in.  This will be used for more complex render
      /// management in future.
      /// 
      /// Overrides of this function should have no side-effects other than filling in reqData.
      virtual void getRequests(const Box& box,
                               const ChannelSet& channels,
                               int count,
                               RequestOutput &reqData) const
      {
        reqData.request(&input0(), box, channels, count);
      }

      /// This is the actual rendering function that subclasses should implement.
      ///
      /// The subclass is responsible for filling plane to the extent of its bbox and
      /// channels.  It is guaranteed that the plane will never have more than 4 channels,
      /// and that its bounding box will be one of those that getStripeBox() returns.
      /// The ChannelSet will be a full layer, but no more than one layer.
      /// It might be packed or not depending upon the results of negotiation.
      ///
      /// It will only be called for channels in the out_channels as set by _validate.
      virtual void renderStripe(ImagePlane& plane) = 0;

      PlanarI* getPlanarI() override
      {
        return this;
      }

      /// This should be overridden by subclasses if they want striping.
      virtual bool useStripes() const
      {
        return false;
      }

      /// This should be overridden by subclasses to return the desired stripe height.
      virtual size_t stripeHeight() const
      {
        return 256;
      }

      PackedPreference packedPreference() const override
      {
        return ePackedPreferenceNone;
      }

      /// Whether this PlanarIop needs to render in full planes.
      virtual bool renderFullPlanes() const
      {
        return false;
      }

      /// This expands the bounding box so as to cover the full extent of all overlapping tiles.
      Box expandToFullTiles(const Box& box) const;

      /// This expands a channel set to include all channels in the planes that
      /// contain any of the channels passed in, not just those that have been requested.
      ChannelSet expandChannelsToFullPlanes(const ChannelSet& channelSet);

      /// This expands a channel set to include all channels in the planes that
      /// contain any of the channels passed in, not just those that have
      /// been requested, and returns them as a plane set.
      PlanarI::PlaneSet expandToFullPlanes(ChannelSet channelSet);

      /// Return the planar cache for this iop.
      LocalPlaneCache* getCache() override;

      /// Used to allow/prevent Nuke from freeing the internal planar cache.
      class DDImage_API AutoMemoryFreeDisabler
      {
      public:
        // Disable memory free on the given cache.
        explicit AutoMemoryFreeDisabler(PlanarIop* op);

        // Enable memory free on the given cache.
        ~AutoMemoryFreeDisabler();

        // Implement move constructor to ensure correct behaviour when storing in collections
        AutoMemoryFreeDisabler(AutoMemoryFreeDisabler&& other);

      private:
        PlanarIop* _op = nullptr;
      };

      /// Function to be called by Nuke before the image is rendered (e.g. in Viewer_Image and Execute)
      /// to enable plane interests so that the planar cache is not cleared while rows are reading from it.
      static void EnablePlaneInterests();

      /// Function to be called by Nuke after the image is rendered to make sure that
      /// the planar cache's memory can be released (e.g. in Viewer_Image and Execute).
      static void DisablePlaneInterests();

    protected:
      /// Protected implementation of this Iop function. Will initialise the planar cache as necessary.
      void _open() override;

      /// Protected implementation of engine(). Should not be overriden further (but some plugins do).
      void engine(int y, int x, int r, ChannelMask channels, Row& row) override;

      /// Helper function used by engine() to lock the planar cache and call through renderStripe().
      void getRowFromPlanes(int y, int x, int r, const DD::Image::ChannelSet& channels, DD::Image::Row& row);

      /// This should not be overriden by subclasses (but some plugins do, notoriously Read).
      void _request(int x, int y, int r, int t, ChannelMask chans, int count) override;

      /// Private implementation of this Iop function.
      /// Calls renderStripe to do actual work, which is the one that subclasses should implement.
      void doFetchPlane(ImagePlane& plane) override;

      /// Render the given stripe and planeID and return the ImagePlane.
      virtual ImagePlane renderStripe(size_t stripe, const ChannelSet& planeID);

      /// Implementation of this function based on useStripes() and stripeHeight().
      size_t getStripeCount() const override;

      /// Implementation of these functions based on useStripes() and stripeHeight().
      Box getStripeBox(int idx) const override;
      size_t rowToStripeIndex(int y) const override;

      /// Determines the set of channels to pass through.
      ChannelSet getRawChannels(const ChannelSet& channels) const;

      /// Get all the planes that we need to go through to render the given channels.
      PlaneID getPlaneIDToRender(const ChannelSet& channels);
      PlaneSet getPlaneIDsToRender(const ChannelSet& channels);

    private:
      /// The lock for the purposes of rendering: the render thread will hold this.
      Lock _renderLock;

      /// The planar cache
      ImagePlaneCacheImpl* _cacheImpl;
    };

  }
}

#endif
