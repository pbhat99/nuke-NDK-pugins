// TransformReceiverI.h
// Copyright (c) 2010 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDIMAGE_TRANSFORMRECEIVERI_H
#define DDIMAGE_TRANSFORMRECEIVERI_H

#include "DDImage_API.h"

namespace DD
{
  namespace Image
  {
     /*!
     * This is a node that is able to accept a 4x4 transform matrix, and optionally 
     * apply it to something context specific i.e. to a particular named Roto shape
     */
    class DDImage_API TransformReceiverI
    {
      public:
        TransformReceiverI();
        virtual ~TransformReceiverI();
        /*!
         * Instruct the node to receive the transform, created for a given frame and view. 
         * Note that the node may receive the trasnform, yet decide
         * not to do anything with it, i.e. "receive()" and not "set()". Returns true if used the data, false
         * otherwise
         */
        virtual bool receiveTransform(double* transform, int frame, int view, const char* context)  = 0;
    };
  }
}

#endif

