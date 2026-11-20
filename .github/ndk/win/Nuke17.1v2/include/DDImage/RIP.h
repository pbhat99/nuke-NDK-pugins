// Copyright (c) 2011 The Foundry Visionmongers Ltd.  All Rights Reserved.

// interface for accessing Rip contexts from within Nuke

#ifndef RIPContext_h
#define RIPContext_h

#include "DDImage_API.h"

namespace DD {
  namespace Image {
    /**
     * create and register the DDImage RIP Context with RIP
     */
    DDImage_API void createRIPContext();

    /**
     * Destroy the DDImage RIP Context
     */
    DDImage_API void destroyRIPContext();

  } // namespace Image
} // namespace DD

#endif // RIPContext_h

