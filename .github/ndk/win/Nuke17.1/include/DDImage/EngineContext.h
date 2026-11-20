// Copyright (c) 2011 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_EngineContext_h
#define DDImage_EngineContext_h

#include "DDImage_API.h"

namespace DD {
  namespace Image {

    class OpTree;

    /**
     * Context for engine() threads, so that aborted() check can be
     * made on the correct opTree, rather than all opTrees that an
     * Op is part of.
     */
    class DDImage_API EngineContext
    {
      OpTree* _opTree;
      EngineContext* _oldContext;
    public:
      /** push this EngineContext onto the thread-local stack */
      EngineContext(OpTree* opTree);

      /** pop the EngineContext from the thread-lock stack */
      ~EngineContext();
 
      /** is the current OpTree in an aborted state? */
      bool aborted() const;
    };

  }
}

#endif
