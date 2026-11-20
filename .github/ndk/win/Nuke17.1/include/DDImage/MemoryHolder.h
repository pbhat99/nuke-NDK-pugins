// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.
#ifndef DDIMAGE_MEMORYHOLDER_H
#define DDIMAGE_MEMORYHOLDER_H

#include <DDImage/DDImage_API.h>
#include <DDImage/Memory.h>

namespace DD
{
  namespace Image
  {
    /**
     * Implement this interface to enable a class to be registered with the
     * Memory system for free, info and weight callbacks.
     *
     * If you derive from this class, you must create the objects with 
     * MemHolderFactory in order to register them with the
     * memory system.
     */
    class DDImage_API MemoryHolder
    {
    public:
      // Try to deallocate at least "amount_to_free" bytes of memory.
      // Returns true if *any* memory was actually freed.
      virtual bool  memoryFree(size_t amount) = 0;

      // This is called to determine how much memory is being used for a particular Node.
      // you can fill in as many as needed
      virtual void  memoryInfo(Memory::MemoryInfoArray& output, const void* restrict_to) const = 0;

      // Return a "weight" or importance to your data.  Higher numbers
      // mean "more important, keep this around longer."  See the "cache_report"
      // TCL command for an idea of the weights used by the cache, etc.
      virtual int   memoryWeight() const = 0;

    protected:
      MemoryHolder() {}
      // Require a virtual destructor
      // It is a requirement that this is called on deleting any MemoryHolder
      // object.
      virtual ~MemoryHolder() {}

    private:
      // not implemented
      MemoryHolder(const MemoryHolder& buffer);
      MemoryHolder& operator=(const MemoryHolder& buffer);
    };

  }
}

#endif
