/*********************************************************************************************/
//
//  file    IAllocator.h
//
//  author  Tom Ward
//
//  brief   Allocator interface class, for cross-allocator type info
//
//  Copyright (c) Tom Ward 2011. ALL RIGHTS RESERVED.
//
/*********************************************************************************************/

#ifndef NUKE_IALLOCATOR_H
#define NUKE_IALLOCATOR_H

#include "DDImage/DDImage_API.h"

#include "Build/fnAssert.h"

#include <assert.h>
#include <stddef.h>


namespace DD
{
namespace Image
{

//! Allocator interface
class DDImage_API IAllocator
{
public:

  virtual ~IAllocator() {}

  //! Return the name for this allocator
  virtual const char* getName() const = 0;

  //! Return the type name for this allocator (e.g. "HeapAllocator")
  virtual const char* getType() const = 0;

  //! Returns the sum total requested by the user.
  virtual size_t getRequestedUsage() const = 0;

  //! Returns the sum total used by the allocator.
  //! This can differ from the requested in some allocators, i.e. BlockAllocator
  virtual size_t getAllocatorUsage() const 
  { 
    return getRequestedUsage(); 
  }

  //! Returns the high watermark for the requested allocations
  virtual size_t getRequestedHighWaterMark() const = 0;

  //! Returns the high watermark for the allocator. 
  //! This can differ from the RequestedHighwaterMark in some allocators, i.e. BlockAllocator
  virtual size_t getAllocatorHighWaterMark() const 
  { 
    return getRequestedHighWaterMark(); 
  }

  //! Resets the high watermark for the allocator
  virtual void resetHighWaterMark() = 0;

  //! Helper function for returning a concrete allocator type.
  template< typename T >
  static T* getAllocator( IAllocator* pAllocator )
  {
    mFnAssert( dynamic_cast<T*>(pAllocator) );
    return static_cast<T*>(pAllocator);
  }
};

} // namespace Image
} // namespace DD

#endif // NUKE_IALLOCATOR_H
