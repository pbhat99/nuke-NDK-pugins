#ifndef DDIMAGE_REF_COUNTED_OBJECT_H
#define DDIMAGE_REF_COUNTED_OBJECT_H

#include "DDImage_API.h"

// Copyright (c) 2011 The Foundry Visionmongers Ltd.  All Rights Reserved.

namespace DD {
  namespace Image {

    /**
     * Base class for a reference counted object to be accessed via a RefCountedPtr
     */
    class DDImage_API RefCountedObject
    {
      int _refs;      

      //! not implemented
      RefCountedObject(const RefCountedObject& other);
      void operator=(const RefCountedObject& other);

    public:
      RefCountedObject() : _refs(0) { }

      virtual ~RefCountedObject();
      
      //! increment the reference count
      void ref()
      {
        _refs++;
      }
      
      //! decrement the reference count.  delete object if this becomes zero.
      void unref()
      {
        if ((--_refs) == 0)
          delete this;
      }

      //! get the reference counter
      int refcount() const
      {
        return _refs;
      }
    };
    
    /**
     * Template for a reference counted ptr.  Template argument T should be a 
     * subclass of RefCountedObject, or provide a similar interface.
     */
    template<class T> class RefCountedPtr
    {
      T* _object;
    public:

      /**
       * Create a NULL pointer.
       */
      RefCountedPtr<T>() : _object(nullptr)
      { 
      }

      /**
       * Create a new RefCountedPtr around an existing object, and increment
       * its reference count.
       */
      explicit RefCountedPtr<T>(T* obj) : _object(obj)
      { 
        if (obj)
          obj->ref();
      }
      
      /**
       * Copy an existing RefCountedPtr and increment the reference
       */
      RefCountedPtr<T>(const RefCountedPtr<T>& other) : _object(other._object)
      {
        if (_object)
          _object->ref();
      }
      
      /**
       * Destroy the RefCountedPtr and if needed delete the underlying object
       */
      ~RefCountedPtr<T>()
      {
        if (_object)
          _object->unref();
      }
      
      /**
       * Copy an existing RefCountedPtr, incrementing and decrementing
       * references as appropriate.
       */
      void operator=(const RefCountedPtr<T>& other)
      {
        if (_object == other._object)
          return;

        if (_object)
          _object->unref();
        _object = other._object;
        if (_object)
          _object->ref();
      }
      
      /**
       * Get the underlying object
       */
      T& operator*()
      {
        return *_object;
      }

      /**
       * Get the underlying object
       */
      const T& operator*() const
      {
        return *_object;
      }
      
      /**
       * Get the underlying object
       */
      T* operator->()
      {
        return _object;
      }

      /**
       * Get the underlying object
       */
      const T* operator->() const
      {
        return _object;
      }

      /**
       * Return the reference count of the underlying object
       */
      int refcount() const 
      {
        if(_object)
          return _object->refcount();
        return 0;
      }

      /**
       * Use dynamic_cast to downcast to another, more specific, RefCountedPtr type
       */
      template<class U> static RefCountedPtr<T> downCast(RefCountedPtr<U> other)
      {
        if (T* obj = dynamic_cast<T*>(&*other))
          return RefCountedPtr<T>(obj);
        return RefCountedPtr<T>();
      }
      
      /**
       * Test whether the pointer is pointing at an object right now.  True if it is.
       */
      operator bool() const
      {
        return _object != nullptr;
      }
      
      /**
       * Allocate a new object and make this pointer be a reference to it.
       */
      void allocate()
      {
        if (_object) {
          _object->unref();
        }
        _object = new T();
        _object->ref();
      }

      /**
       * Allocate a new object and make this pointer be a reference to it.
       */
      template<class P1> void allocate(const P1& p1)
      {
        if (_object) {
          _object->unref();
        }
        _object = new T(p1);
        _object->ref();
      }
      
      /**
       * Allocate a new object and make this pointer be a reference to it.
       */
      template<class P1, class P2> void allocate(const P1& p1, const P2& p2)
      {
        if (_object) {
          _object->unref();
        }
        _object = new T(p1, p2);
        _object->ref();
      }
      
      /**
       * Allocate a new object and make this pointer be a reference to it.
       */
      template<class P1, class P2, class P3> void allocate(const P1& p1, const P2& p2, const P3& p3)
      {
        if (_object) {
          _object->unref();
        }
        _object = new T(p1, p2, p3);
        _object->ref();
      }
      

      /**
       * Unreference the existing object (possibly deleting it)
       */
      void clear()
      {
        if (_object) {
          _object->unref();
        }
        _object = nullptr;
      }
    };

  }
}

#endif
