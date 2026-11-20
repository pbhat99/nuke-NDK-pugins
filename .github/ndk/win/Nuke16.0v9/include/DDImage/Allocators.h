//------------------------------------------------------------------------------
// DDImage/include/DDImage/Allocators.h
//
// Copyright (c) 2012 The Foundry Visionmongers Ltd.  All Rights Reserved.
//------------------------------------------------------------------------------

#ifndef NUKE_ALLOCATORS_H
#define NUKE_ALLOCATORS_H

#include "DDImage/BlockAllocator.h"
#include "DDImage/HeapAllocator.h"
#include "DDImage/STLAllocator.h"

namespace DD
{
namespace Image
{
  class DDImage_API Allocators
  {
  public:

    // Creates default allocators
    static void createDefaultAllocators();
    static void destroyDefaultAllocators();

    static BlockAllocator*  g3DAllocator;
    static HeapAllocator*   gNodeAllocator;
    static HeapAllocator*   gOpAllocator;
    static HeapAllocator*   gKnobAllocator;
  };
  
  // STL Class definitions, using global pointers
  mFnCreateGlobalSTLAllocator(STL3DAllocator,    Allocators::g3DAllocator)
  mFnCreateGlobalSTLAllocator(STLNodeAllocator,  Allocators::gNodeAllocator)
  mFnCreateGlobalSTLAllocator(STLOpAllocator,    Allocators::gOpAllocator)
  mFnCreateGlobalSTLAllocator(STLKnobAllocator,  Allocators::gKnobAllocator)

} // namespace Image
} // namespace DD

#endif // NUKE_ALLOCATORS_H
