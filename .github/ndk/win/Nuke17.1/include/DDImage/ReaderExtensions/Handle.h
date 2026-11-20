// Copyright (c) 2012 The Foundry Visionmongers Ltd.  All Rights Reserved.

//---------------------------------------------------------------------------------
//
// This file provides the definition and implementation of the handle classes
// used by the read/decode interface to the Reader base class added by ReaderExtensions.h
// and ReaderExtensions.cpp.
//
// NOTE:
// Nuke itself makes no use of these extensions but it needs to compile with them
// so that Reader-derived plugins are compatible with Hiero. Similarly, thirdparty
// plugins built with these extensions included will be compatible with Hiero.
// Note that Reader-derived plugins do not need to provide their own implementation
// of the extensions API, even for use in Hiero, as Reader provides a default
// implementation in terms of its existing virtual engine() interface.
//
//---------------------------------------------------------------------------------

#ifndef DDImage_ReaderExtensions_Handle_H
#define DDImage_ReaderExtensions_Handle_H

#include "DDImage/DDImage_API.h"
#include "DDImage/RefCountedObject.h"
#include <stddef.h>

namespace DD
{
  namespace Image
  {
    ////////////////////////////////////////////////////////////////////////////////
    /// Base class for an object that allocates data in some way.
    ///
    /// This class is meant to be derived from, the ctor of the
    /// derived class should set up the data pointer and size
    /// while it's dtor should destroy it. The destructor
    /// needs to be overridden for this to happen.
    ///
    /// The data is allocated on constuction of the object and
    /// deallocated on destruction.
    class DDImage_API BaseHandle : public RefCountedObject
    {
    protected:
      void* _data;    ///< our data blob
      const size_t _size;  ///< how big it is

    public:

      /// ctor, sets the data and size
      BaseHandle(void* data, size_t size)
        : _data(data)
        , _size(size)
      {}

      ~BaseHandle() override = 0;

      /// Get the data that has been allocated at construction.
      void* getAllocatedData() const { return _data; }

      /// get the amount of data that is pointed to by this handle
      size_t getDataSize() const { return _size; }
    };
    
    typedef RefCountedPtr<BaseHandle> HandlePtr;
    
    /// Empty function - C++ requires an implementation for pure virtual destructors.
    inline BaseHandle::~BaseHandle() {}

    /// Standard handle, which allocates via DD Images memory pool
    class Handle : public BaseHandle {
    public:
      
      Handle(size_t size)
      : BaseHandle(nullptr, size)
      {
        _data = DD::Image::Memory::allocate_void(size);
      }
      
      ~Handle() override
      {
        DD::Image::Memory::deallocate_void(_data, _size);
      }
      
      static HandlePtr Make(size_t size)
      {
        HandlePtr hp(new Handle(size));
        return hp;
      }
    };
    
  } // namespace Image
}   // namespace DD

#endif  // DDImage_ReaderExtensions_Handle_H

// Copyright (c) 2012 The Foundry Visionmongers Ltd.  All Rights Reserved.
