//------------------------------------------------------------------------------
// DDImage/include/DDImage/STLAllocator.h
//
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
//------------------------------------------------------------------------------

#ifndef __DDImage_STLAllocator_h__
#define __DDImage_STLAllocator_h__

#include <vector>
#include <deque>

#include <assert.h>
#include <stddef.h>

#include "Build/fnAssert.h"
#include "DDImage/Memory.h"
#include "Deprecation.h"

//! Macro that creates an STL allocator class with pointer to instance of an allocator
#define mFnCreateInstanceSTLAllocator(STLInstanceClassName, AllocatorType)                \
  template < class T >                                                                    \
  class                                                                                   \
  mFnDeprecatedStlAllocatorInNuke17(#STLInstanceClassName " has been deprecated. Use std::allocator instead.") \
  STLInstanceClassName                                                                    \
  {                                                                                       \
  public:                                                                                 \
      typedef size_t        size_type;                                                    \
      typedef ptrdiff_t     difference_type;                                              \
      typedef T*            pointer;                                                      \
      typedef const T*      const_pointer;                                                \
      typedef T&            reference;                                                    \
      typedef const T&      const_reference;                                              \
      typedef T             value_type;                                                   \
                                                                                          \
      STLInstanceClassName() { _AllocatorInstance = NULL; }                               \
                                                                                          \
      STLInstanceClassName( AllocatorType* allocator )                                    \
      {                                                                                   \
        mFnAssert( allocator != NULL );                                                   \
        _AllocatorInstance = allocator;                                                   \
      }                                                                                   \
                                                                                          \
      STLInstanceClassName(const STLInstanceClassName& other)                             \
      {                                                                                   \
        mFnAssert( other._AllocatorInstance != NULL );                                    \
        _AllocatorInstance = other._AllocatorInstance;                                    \
      }                                                                                   \
                                                                                          \
      pointer allocate(size_type n, const void* = 0)                                      \
      {                                                                                   \
        mFnAssert( _AllocatorInstance != NULL );                                          \
        return (T*)_AllocatorInstance->allocate(n * sizeof(T));                           \
      }                                                                                   \
                                                                                          \
      void deallocate(void* p, size_type n)                                               \
      {                                                                                   \
        if (p) {                                                                          \
          mFnAssert( _AllocatorInstance != NULL );                                        \
          _AllocatorInstance->deallocate(p);                                              \
        }                                                                                 \
      }                                                                                   \
                                                                                          \
      pointer           address(reference x) const { return &x; }                         \
      const_pointer     address(const_reference x) const { return &x; }                   \
      STLInstanceClassName<T>&  operator=(const STLInstanceClassName&) { return *this; }  \
      void              construct(pointer p, const T& val) { new ((T*) p)T(val); }        \
      void              destroy(pointer p) { p->~T(); }                                   \
                                                                                          \
      size_type         max_size() const { return size_type(-1); }                        \
                                                                                          \
      template <class U>                                                                  \
      struct rebind                                                                       \
      { typedef STLInstanceClassName<U> other; };                                         \
                                                                                          \
      template <class U>                                                                  \
      STLInstanceClassName(const STLInstanceClassName<U>& other)                          \
      {                                                                                   \
        mFnAssert( other._AllocatorInstance != NULL );                                    \
        _AllocatorInstance = other._AllocatorInstance;                                    \
      }                                                                                   \
                                                                                          \
      template <class U>                                                                  \
      STLInstanceClassName& operator=(const STLInstanceClassName<U>&) { return *this; }   \
                                                                                          \
    private:                                                                              \
                                                                                          \
      /** Pointer to allocator instance **/                                               \
      AllocatorType*  _AllocatorInstance;                                                 \
                                                                                          \
    };


//! Macro that creates a templated class and uses a global pointer to an allocator for allocations
//! Note: equality operators added for C++11 support. Allocators of the same type can always
//! compare equal, since they use the same AllocatorPtr.
#define mFnCreateGlobalSTLAllocator(STLAllocatorName, AllocatorPtr)                       \
  template < class T >                                                                    \
  class                                                                                   \
  mFnDeprecatedStlAllocatorInNuke17(#STLAllocatorName " has been deprecated. Use std::allocator instead.") \
  STLAllocatorName                                                                        \
  {                                                                                       \
  public:                                                                                 \
    typedef size_t        size_type;                                                      \
    typedef ptrdiff_t     difference_type;                                                \
    typedef T*            pointer;                                                        \
    typedef const T*      const_pointer;                                                  \
    typedef T&            reference;                                                      \
    typedef const T&      const_reference;                                                \
    typedef T             value_type;                                                     \
                                                                                          \
    STLAllocatorName() {}                                                                 \
    STLAllocatorName(const STLAllocatorName&) {}                                          \
                                                                                          \
    pointer allocate(size_type n, const void* = 0)                                        \
    {                                                                                     \
      return (T*)(AllocatorPtr)->allocate(n * sizeof(T));                                 \
    }                                                                                     \
                                                                                          \
    void deallocate(void* p, size_type n)                                                 \
    {                                                                                     \
      if (p) {                                                                            \
        (AllocatorPtr)->deallocate(p);                                                    \
      }                                                                                   \
    }                                                                                     \
                                                                                          \
    pointer           address(reference x) const { return &x; }                           \
    const_pointer     address(const_reference x) const { return &x; }                     \
    STLAllocatorName<T>&  operator=(const STLAllocatorName&) { return *this; }            \
    void              construct(pointer p, const T& val) { new ((T*) p)T(val); }          \
    void              destroy(pointer p) { p->~T(); }                                     \
                                                                                          \
    size_type         max_size() const { return size_type(-1); }                          \
                                                                                          \
    template <class U>                                                                    \
    struct rebind { typedef STLAllocatorName<U> other; };                                 \
                                                                                          \
    template <class U>                                                                    \
    STLAllocatorName(const STLAllocatorName<U>&) {}                                       \
                                                                                          \
    template <class U>                                                                    \
    STLAllocatorName& operator=(const STLAllocatorName<U>&) { return *this; }             \
                                                                                          \
    bool operator==(const STLAllocatorName& other) const { return true; }                 \
    bool operator!=(const STLAllocatorName& other) const { return !operator==(other); }   \
  };

namespace DD
{
  namespace Image
  {

    class DDImage_API DefaultAllocator
    {
    public:

      void* allocate(size_t size)
      {
        return Memory::allocate_void(size);
      }

      void deallocate(void* ptr)
      {
        Memory::deallocate_void(ptr);
      }

      //! return static instance of default allocator
      static DefaultAllocator* getInstance()
      {
        static DefaultAllocator _instance;
        return &_instance;
      }

    };

    mFnCreateGlobalSTLAllocator(STLAllocator, DefaultAllocator::getInstance())

    //Templated namespace
    template<class T >
    class STL
    {
    public:
      typedef std::vector<T, STLAllocator<T> > vector;
      typedef std::deque<T, STLAllocator<T> > deque;
    };
  }
}

#endif // __DDImage_STLAllocator_h__
