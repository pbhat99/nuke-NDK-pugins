// Copyright (c) 2012 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_MemoryBuffer_H
#define DDImage_MemoryBuffer_H

#include "DDImage/DDImage_API.h"
#include "DDImage/Memory.h"
#include "DDImage/HeapAllocator.h"
#include <memory>

namespace DD
{
  namespace Image
  {

    // forward declaration
    class MemoryBufferImpl;
    class IBufferFill;

    // Memory Buffer is a helper class registered to Nuke Memory manager.
    // It provides a memory buffer that is automatically released by the Memory Manager 
    // when Nuke needs more memory. The class is thread safe and internally 
    // provides a locking mechanism for multiple readers.
    class DDImage_API MemoryBuffer
    {
      friend class MemoryBufferGuard;

      std::unique_ptr<MemoryBufferImpl> _pImpl;
      
      // Not implemented
      MemoryBuffer(const MemoryBuffer& buffer);
      MemoryBuffer& operator=(const MemoryBuffer& buffer);
      
      // Read write buffer access
      void* readLock();
      
      // Unlock the buffer from a read or write access
      void unLock();
         
    public:

      // Constructor
      MemoryBuffer(IBufferFill* autofill, size_t size=0);
      
      // Destructor
      ~MemoryBuffer();
      
      // Set buffer size
      void resize(size_t size);

      // Get buffer size
      size_t size() const;
      
      // Invalidate the buffer force to regenerate the internal buffer
      void invalidate();
      
    };

    // This is the requested callback interface used to generate and fill the internal buffer data.    
    class DDImage_API IBufferFill 
    {
    public:
      
      virtual ~IBufferFill()
      {
      }

      // Allocate the buffer
      virtual void* allocate(size_t size)
      {
        return Memory::allocate_void(size);
      }

      // Deallocate the buffer
      virtual void deallocate(void* data)
      {
        Memory::deallocate_void(data);
      }
      
      // Callback to regenerate the buffer data.
      // Is thread safe and synchronizes all the reading access.
      // This method should return true when the buffer is been correctly initialized.
      // Returning false keeps that buffer in an invalid state, in this case the MemoryBufferGuard
      // can return a NULL pointer.
      virtual bool fillBuffer(void* data) = 0;

      // Called before freeing the buffer data
      // It can be used for serialization purposes.
      virtual bool freeBuffer(void* data)
      {
        return true;
      }

      // Called to get memory priority of the buffer allocation
      virtual int memoryWeight() const
      {
        return 100;
      }

      // Called to determine how much memory is being used.
      virtual void memoryInfo(Memory::MemoryInfoArray& output, const void* restrict_to) const = 0;

    };

    // A specialization of IBufferFill that uses a standard heap to allocate the buffer
    class DDImage_API IBufferFillHeap : public IBufferFill
    {
      HeapAllocator* _allocator;
    public:

      IBufferFillHeap(const char* allocatorName)
      : _allocator(Memory::create_allocator<HeapAllocator>(allocatorName))
      {}

      void* allocate(size_t size) override
      {
        return _allocator->allocate(size);
      }

      void deallocate(void* data) override
      {
        _allocator->deallocate(data);
      }
    };

    // This guard is the only way to get read only permission to the internal 
    // memory buffer. Multiple threads can read in parallel without locking each other.
    // Inside the guard scope the buffer is guaranteed to be ready and never deallocated.
    class DDImage_API MemoryBufferGuard
    {
      MemoryBuffer& _buffer;
      void* _data;
    public:

      MemoryBufferGuard(MemoryBuffer& buffer) 
      : _buffer(buffer)
      , _data(nullptr)
      {
        _data = _buffer.readLock();
      }
        
      ~MemoryBufferGuard()
      {
        _buffer.unLock();
      }
      
      // Return the buffer memory pointer.
      void* buffer() const
      {
        return _data;
      }      
    };
  }
}

#endif
