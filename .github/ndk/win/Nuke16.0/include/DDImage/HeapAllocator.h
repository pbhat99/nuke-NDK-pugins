// HeapAllocator.h
// Copyright (c) 2012 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef NUKE_HEAP_ALLOCATOR_H
#define NUKE_HEAP_ALLOCATOR_H

#include "DDImage_API.h"
#include "IAllocator.h"
#include "DDImage/STLAllocator.h"
#include <map>

namespace DD
{
namespace Image
{

  class HeapAllocatorImpl;
  class AllocationTracker;

/**
    Heap Allocator

    This is a very simple, thread-safe Heap allocator which calls through to DD::Image::Memory
    and tracks how much memory its using.
    Additionally, if FN_TRACK_MEMORY_ALLOCATIONS is defined (in Memory.h), and the
    NUKE_TRACK_OBJECT_ALLOCATIONS environment variable is set to 1, allocations using this
    allocator will be tracked between checkpoints, so that leaks can be identified.
    See the _nuke_internal.memAllocCheckpoint()/memAllocInfo() Python API calls for details.
*/
class DDImage_API HeapAllocator : public IAllocator
{

public:
  HeapAllocator(
    const char* allocatorName );

  ~HeapAllocator() override;

  void* allocate( size_t size );

  template< typename T >
  inline T* allocate( size_t numItems );

  // helper function to allocate memory and call default constructor
  template< typename T >
  inline T* New();

  void deallocate( void* memory );

  //! Called from constructors of objects allocated using this allocator with their this pointer.
  //! This allows the allocation tracker to track the actual object address within an allocated block.
  void setObjAddress( void* address );

  // helper function to delete memory and explicitly call destructor
  // ( see http://www.parashift.com/c++-faq-lite/dtors.html#faq-11.11 )
  template< typename T >
  inline void Delete( T* memory );

  const char* getName() const override;
  const char* getType() const override;
  size_t      getRequestedUsage() const override;
  size_t      getRequestedHighWaterMark() const override;
  void        resetHighWaterMark() override;

  virtual const AllocationTracker* getTracker() const;

private:

  // prevent default instantiation and copying an allocator
  HeapAllocator();
  HeapAllocator(const HeapAllocator&);

  // implementation
  HeapAllocatorImpl* _heapAllocatorImpl;
};

//! Create STL allocator class using macro
mFnCreateInstanceSTLAllocator(STLHeapAllocator, HeapAllocator)

/********************************************************************************************/
//
/********************************************************************************************/

template< typename T >
inline T* 
HeapAllocator::allocate( size_t numItems )
{
  return (T*)allocate( sizeof(T) * numItems );
}

template< typename T >
inline T*
HeapAllocator::New()
{
  T* newObject = (T*)allocate( sizeof(T) );
  mFnAssert( newObject != NULL );

  if ( newObject )
  {
    newObject = new( newObject ) T;
  }

  return newObject;
}


template< typename T >
inline void 
HeapAllocator::Delete( T* memory )
{
  mFnAssert( memory != NULL );

  if (memory) {
    memory->~T();
    deallocate( memory );
  }
}

} // namespace Image
} // namespace DD

#endif // NUKE_HEAP_ALLOCATOR_H

// Copyright (c) 2012 The Foundry Visionmongers Ltd.  All Rights Reserved.
