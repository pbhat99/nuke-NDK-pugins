
// Copyright (c) 2011 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_DeepOp_H
#define DDImage_DeepOp_H

#include "DDImage/Op.h"
#include "DDImage/Iop.h"
#include "DDImage/DeepPlane.h"
#include "DDImage/Box.h"
#include "DDImage/DeepInfo.h"
#include "DDImage/RequestData.h"
#include "DDImage/ConvertibleToIop.h"

#include <vector>

namespace DD {
  namespace Image {

    class DeepOpImpl;

    /**
     * Deep Operator interface.
     *
     * This is analogous to Iop, except that rows are replaced with deepdata tiles (DeepPlane).
     * Subclasses need to implement three phases
     * 
     *  _validate, which should fill in the _deepInfo
     *
     *  getDeepRequests(), which should calculate the region of interest and channels needed on
     *    the inputs given a region of interest and channels requested on the deep op.
     *
     *  doDeepEngine(), which should do the actual calculation.
     *
     * Is an interface so that there can be dual DeepOp/Iops.
     */
    class DDImage_API DeepOp : public ConvertibleToIop
    {
      std::unique_ptr<DeepOpImpl> _pImpl;

      /* not implemented */
      DeepOp(const DeepOp& other);

      /* not implemented */
      void operator=(const DeepOp& other);

    protected:
      /** The deep info.  Analogous to Iop::info_, but rather simpler at present, consisting only 
         of a channelset and a bounding box.  */
      DeepInfo _deepInfo;

      /**
         Calculate the actual deep image data for the given box and channels, and place it 
         in plane.  Plane is not yet initialised, and the implementation should fill it in.
         It is permitted to return a plane which is larger or has more channels than the specified
         area.  Also, it permitted to not assign plane at all, in which case an empty (0-sample)
         plane will be returned to the caller.
         
         Returns false if processing was halted due to an abort.
       */
      virtual bool doDeepEngine(DD::Image::Box box, const DD::Image::ChannelSet& channels, DeepOutputPlane& plane) = 0;

      /**
       * rather then have a DeepOp::_request() function that recursively calls request() on its inputs,
       * this function, the DeepOp version of _request, should place the regions to be requested into
       * the vector passed in.  This will be used for more complex render management in future.
       *
       * Implementations of this function should have no side-effects other than filling in reqData.
       */
      virtual void getDeepRequests(DD::Image::Box box,
                                   const DD::Image::ChannelSet& channels,
                                   int count,
                                   std::vector<RequestData> &reqData) = 0;

      /**
       * Validate the op, which should result in a valid bbox and channelset being placed in _deepInfo.
       * n.b. if you are an Iop you should make sure to call the Iop::_validate function too.  If you 
       * copy your deepInfo from an input, call Op::validate on that input.
       * 
       * The forReal parameter specifies whether exact information is required.  If forReal is false,
       * then it is permitted to not open the file to find out what bbox/channels are present,
       * and rely on expected values from knobs.
       */
      virtual void _validate(bool forReal) = 0;

    public:
      DeepOp();

      ~DeepOp() override;
      
      /**
       * Return the actual Op that this DeepOp interface is on.  Implementations should just "return this;"
       */
      virtual Op* op() = 0;
      
      /**
       * Get the current info (bbox/channelset)
       */
      const DeepInfo& deepInfo() const
      {
        mFnAssert(const_cast<DeepOp*>(this)->op()->valid());
        return _deepInfo;
      }
      
      /**
       * Validate the op, setting the DeepInfo as a side-effect.
       */
      void validate(bool forReal)
      {
        op()->validate(forReal);
        mFnAssert(_deepInfo.format());
      }
      
      /**
       * Request a given area.  This should only be used by code requesting deep data for conversion
       * into some other form: getDeepRequests() should be used for the requests of deep data or
       * Iop data by deep nodes.
       */
      void deepRequest(DD::Image::Box box, const DD::Image::ChannelSet& channels, int count = 1);
      
      /**
       * Get the given area and box.  Could result in more channels and area being placed in /plane/,
       * especially if the cache is in use.
       *
       * Returns false if processing was halted due to an abort.
       */
      bool deepEngine(DD::Image::Box box, const DD::Image::ChannelSet& channels, DeepPlane& plane);

      /**
         Get a given row

         Returns false if processing was halted due to an abort.
      */
      bool deepEngine(int y, int x, int r, const DD::Image::ChannelSet& channels, DeepPlane& plane)
      {
        return deepEngine(DD::Image::Box(x, y, r, y+1), channels, plane);
      }

      /**
       * This pre-fills the cache, spawning threads to do its work.  Only call this if
       * you are in the main thread, or have a lock against the other threads.
       */
      void fillCache();

      /**
       * This pre-fills the cache, spawning threads to do its work.  Only call this if
       * you are in the main thread, or have a lock against the other threads.
       *
       * If you want the code to pop up a cancellable non-modal progress dialog, set
       * /cancellable/ to true, and then the function will return false if cancelled or aborted,
       * cancelReason (if specified) will be set to either eAborted or eCancelled, depending
       * upon what happened.
       */
      bool fillCache(bool cancellable, Op::PrevalidateResult* cancelReason);

      const char* conversionHelperNodeClass() const override;      

      const Format* convertibleFormat() const override;

      //! return the node shape to be used by Deep & related operators
      static const char* DeepNodeShape();
    };
    
    /**
     * An Op which is only a DeepOp, to inherit from for deep-specific nodes (e.g. ToDeep, MergeDeep, etc)
     */
    class DDImage_API DeepOnlyOp : public Op, public DeepOp, public Info2DProvider
    {
    public:
      DeepOnlyOp(Node* node) : Op(node) { }
      Op* op() override;
      ~DeepOnlyOp() override;

      const Info2D& info2D() const override
      {
        return _deepInfo;
      }
      const GeneralInfo& generalInfo() const override
      {
        return _deepInfo;
      }

      DeepOnlyOp* deepOnlyOp() override;
      const DeepOnlyOp* deepOnlyOp() const override;

    };

  }
}

#endif
