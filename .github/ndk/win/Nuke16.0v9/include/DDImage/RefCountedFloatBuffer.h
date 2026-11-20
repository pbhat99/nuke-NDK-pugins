#ifndef DDIMAGE_REF_COUNTED_FLOAT_BUFFER_H
#define DDIMAGE_REF_COUNTED_FLOAT_BUFFER_H

#include <cstddef>
#include <cstdint>

#include "DDImage_API.h"

namespace DD {
  namespace Image {

    class Box;

    class MemoryHandleWrapper;

    /**
     * Provides a reference counted float buffer.  Has a pointer to the actual underlying
     * buffer for the purpose of reference counting and also an offset which can be manipulated
     * by pointer arithmetic.
     */
    class DDImage_API RefCountedFloatBuffer
    {
    public:
      //! construct a NULL buffer
      RefCountedFloatBuffer();
      
      //! copy construct the pointer to the buffer
      RefCountedFloatBuffer(const RefCountedFloatBuffer& otherBuffer);

      //! destructor (will free the buffer if the reference count becomes zero)
      ~RefCountedFloatBuffer();

      //! allocate some new memory for a buffer, optionally align memory (also rounds up allocated amount to multiple of alignment)
      static RefCountedFloatBuffer allocate(const DD::Image::Box& bounds, int nComps, int pixelStep, int rowStep, int64_t componentStep, size_t alignment = 0);
           
      RefCountedFloatBuffer operator+(ptrdiff_t offset) const;

      //! assignment operator (will free existing buffer if any)
      RefCountedFloatBuffer& operator=(const RefCountedFloatBuffer& otherBuffer);
      
      //! convert to const float*
      operator const float*() const;
      
      //! convert to float* 
      operator float*();
      //! dump out debug information
      void print() const;

      //! reset to the NULL pointer
      void reset()
      {
        this->operator=(RefCountedFloatBuffer());
      }

      //! get the memory used for this buffer
      size_t usage() const;

      //! get the reference count for this buffer
      int refCount() const;

#ifdef Foundry_Device_fnDeviceMemoryHandle_h
      /**
       * private functions for RIP to access the memory buffer as its real underlying type
       *
       * this seems to be the only way of doing it without polluting the NDK
       * outside of a RIP context with these RIP headers.  Unfortunately we can't
       * even declare MemoryHandleIP as a forward declaration because it's actually
       * a typedef of a template.
       */
      const Foundry::Device::MemoryHandleIP& getBuffer() const;
      void setBuffer(const Foundry::Device::MemoryHandleIP& ptr);
#endif

    private:      
      /**
       * pointer to start of actual buffer. we use a char array here because the actual
       * type is an intrusive pointer to something that is not included with these header
       * files.
       */
      char _buffer[sizeof(void*)];

      //! offset within buffer
      std::ptrdiff_t _offset;

      MemoryHandleWrapper& getWrapper();
      const MemoryHandleWrapper& getWrapper() const;
      
      RefCountedFloatBuffer(const MemoryHandleWrapper& buffer, ptrdiff_t offset);
    };
  }
}

#endif
