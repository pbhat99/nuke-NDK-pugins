// Copyright (c) 2011 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_DeepFilterOp_H
#define DDImage_DeepFilterOp_H

#include "DDImage/DeepOp.h"

namespace DD {
  namespace Image {

    /**
     * Helper class for Deep operators.  The DeepFilterOp is intended
     * for use as a base class by Ops that have only one input, which is
     * a DeepOp, and themselves output Deep Data only (e.g. DeepSelect),
     *
     * _validate gets the deep info from the input, and getDeepRequests()
     * forwards the request to the input (adding Mask_Z).  The only
     * remaining function for subclasses to implement is doDeepEngine().
     */
    class DDImage_API DeepFilterOp : public DeepOnlyOp
    {
    public:
      DeepFilterOp(Node* node) : DeepOnlyOp(node)
      {
      }
      
      ~DeepFilterOp() override;
      
      /**
       * Only allow DeepOps to be connected as inputs
       */
      bool test_input(int idx, Op*op) const override
      {
        return (dynamic_cast<DeepOp*>(op)!=nullptr);
      }
      
      /**
       * Copy the input deepInfo as this Op's deepInfo, otherwise
       * start a new (empty) one.
       */
      void _validate(bool for_real) override
      {
        if (input0()) {
          input0()->validate(for_real);
          _deepInfo = input0()->deepInfo();
        } else {
          _deepInfo = DeepInfo();
        }
      }
      
      /**
       * Pass through the request, but additionally add on a request for
       * the Z channel
       */
      void getDeepRequests(DD::Image::Box box, const DD::Image::ChannelSet& channels, int count, std::vector<RequestData>& requests) override
      {
        if (input0())
          requests.push_back(RequestData(input0(), box, channels + Mask_Deep, count));
      }
      
      /**
       * Get the first input, as a DeepOp, or NULL if it is not connected.
       */
      DeepOp* input0()
      {
        return dynamic_cast<DeepOp*>(Op::input(0));
      }
    };

  }
}

#endif
