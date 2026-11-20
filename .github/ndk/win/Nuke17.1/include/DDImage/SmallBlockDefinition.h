// BlockAllocator.h
// Copyright (c) 2012 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef NUKE_SMALL_BLOCK_DEFINITION_H
#define NUKE_SMALL_BLOCK_DEFINITION_H

#include "DDImage_API.h"
#include <stdint.h>

namespace DD
{
namespace Image
{

// default values for smallblock definition
#define SBA_DEFAULT_CONTIGUOUS_BUCKETS 128
#define SBA_DEFAULT_MIN_BLOCK_SIZE 1
#define SBA_DEFAULT_MAX_BLOCK_SIZE 4096

/**
    SmallBlock Definition

    This is a simple class that passes arguments through to be used by a small block allocator
*/
class DDImage_API SmallBlockDefinition
{
public:

  SmallBlockDefinition(
    uint16_t numContiguousBuckets = SBA_DEFAULT_CONTIGUOUS_BUCKETS,
    uint16_t minSmallBlockSize = SBA_DEFAULT_MIN_BLOCK_SIZE,
    uint16_t maxSmallBlockSize = SBA_DEFAULT_MAX_BLOCK_SIZE ) 
  {
    _numContiguousBuckets = numContiguousBuckets;
    _minSmallBlockSize = minSmallBlockSize;
    _maxSmallBlockSize = maxSmallBlockSize;
  }

  inline uint16_t
  getNumContiguousBuckets() const
  {
    return _numContiguousBuckets;
  }

  inline uint16_t
  getMinSmallBlockSize() const
  {
    return _minSmallBlockSize;
  }

  inline uint16_t
  getMaxSmallBlockSize() const
  {
    return _maxSmallBlockSize;
  }

private:

  // Maximum number of contiguous buckets (allocated together).
  // Raising this will reduce the number of malloc calls but
  // increases the overhead of each malloc allocations
  // (since the all the buckets are added to the free list
  // during the single call to allocate a new block).
  // If the time taken in building lists is significant then the 
  // small block allocator could be adjusted to use a hierarchical structure.
  // A larger number here will also reduce the number of intervals,
  // allowing a faster search for the BucketInfo on free.
  uint16_t    _numContiguousBuckets;

  // Maximum and minimum size for the small block allocator
  //
  // The allocator is actually designed for medium sized blocks (9 bytes - 1 kilobyte).
  // If a large number of very small blocks are required (1-8 bytes) then a more
  // efficient allocator (without list information in the header) should be used.
  // Since this hasn't yet been implemented minimum size is 1.
  uint16_t    _minSmallBlockSize;
  uint16_t    _maxSmallBlockSize;
};

} // namespace Image
} // namespace DD

#endif // NUKE_SMALL_BLOCK_DEFINITION_H

// Copyright (c) 2012 The Foundry Visionmongers Ltd.  All Rights Reserved.
