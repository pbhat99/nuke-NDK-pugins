// BlockAllocator.h
// Copyright (c) 2012 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef NUKE_BLOCK_ALLOCATOR_H
#define NUKE_BLOCK_ALLOCATOR_H

#include "DDImage_API.h"
#include "IAllocator.h"
#include "DDImage/SmallBlockDefinition.h"
#include "DDImage/STLAllocator.h"

namespace DD
{
namespace Image
{

  class BlockAllocatorImpl;

/**
    Block Allocator

    This is a thread-safe allocator that, for particular sizes defined on construction,
    will allocate in blocks a given size and then use these instead of calling malloc

    This stops lots of malloc calls, which will speed up slow system memory calls in most cases, at the
    expense of a memory overhead.

    If a call is made to allocate outside the small block size, then it will call directly through
    to DD::Image::Memory::allocate()

    It also tracks how much memory is currently being used by the given allocator instance
*/
class DDImage_API BlockAllocator : public IAllocator
{

public:

  BlockAllocator(
    const char* allocatorName, const SmallBlockDefinition definition = SmallBlockDefinition() );

  ~BlockAllocator() override;

  void* allocate( size_t size );

  template< typename T >
  inline T* allocate( size_t numItems );

  // helper function to allocate memory and call default constructor
  template< typename T >
  inline T* New();

  void deallocate( void* memory );

  // helper function to delete memory and explicitly call destructor
  // ( see http://www.parashift.com/c++-faq-lite/dtors.html#faq-11.11 )
  template< typename T >
  inline void Delete( T* memory );

  const char* getName() const override;
  const char* getType() const override;
  size_t getAllocatorUsage() const override;
  size_t getAllocatorHighWaterMark() const override;

  size_t getRequestedUsage() const override;
  size_t getRequestedHighWaterMark() const override;
  void resetHighWaterMark() override;
  
private:

  // prevent default instantiation and copying an allocator
  BlockAllocator() { _blockAllocatorImpl = nullptr; }
  BlockAllocator(const BlockAllocator&) {}

  // implementation
  BlockAllocatorImpl* _blockAllocatorImpl;
};

//! Create STL allocator class using macro
mFnCreateInstanceSTLAllocator(STLBlockAllcoator, BlockAllocator)

/********************************************************************************************/
//
/********************************************************************************************/

template< typename T >
inline T* 
BlockAllocator::allocate( size_t numItems )
{
  return (T*)allocate( sizeof(T) * numItems );
}

template< typename T >
inline T*
BlockAllocator::New()
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
BlockAllocator::Delete( T* memory )
{
  mFnAssert( memory != NULL );

  if (memory) {
    memory->~T();
    deallocate( memory );
  }
}

} // namespace Image
} // namespace DD

#endif // NUKE_BLOCK_ALLOCATOR_H

// Copyright (c) 2012 The Foundry Visionmongers Ltd.  All Rights Reserved.
