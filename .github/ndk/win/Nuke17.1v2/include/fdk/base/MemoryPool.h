// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FDK_BASE_MEMORYPOOL_H
#define FDK_BASE_MEMORYPOOL_H

#include "fdk/api.h"

#include <algorithm>
#include <atomic>
#include <cassert>
#include <cstdint>
#include <mutex>
#include <vector>

namespace fdk {

  class SpinLock {
  public:
    void lock() {
      while (locked.test_and_set(std::memory_order_acquire)) {
        // Spin
      }
    }

    void unlock() {
      locked.clear(std::memory_order_release);
    }

    private:
      std::atomic_flag locked = ATOMIC_FLAG_INIT;
  };

  //! A no-locking policy for MemoryPool
  struct NoLockPool {
    using lock_type = int;
    using guard_type = int;
  };

  //! A locking policy for MemoryPool that uses a spin lock
  struct SpinLockPool {
    using lock_type = SpinLock;
    using guard_type = std::lock_guard<lock_type>;
  };

  //! A locking policy for MemoryPool that uses a mutex
  struct MutexPool {
    using lock_type = std::mutex;
    using guard_type = std::lock_guard<lock_type>;
  };

  //! A simple memory pool.
  //!
  //! Allocate memory by block, thus reducing malloc overhead. Memory is
  //! allocated and freed in O(1) (except when a new block is allocated).
  //!
  //! As long as no object is freed, memory is allocated sequentially, except
  //! at block boundaries. If a few objects have been freed, they will be
  //! recycled the next time some memory is allocated. In some sense, when
  //! an object is freed it is pushed on top of a stack of available objects.
  //! When alloc is called, the top-most freed object is recycled. It means
  //! that objects are reallocated in the reverse order they are freed. It
  //! might be important to take this into consideration if you want to
  //! allocate objects sequentially.
  //!
  //! The memory pool store the stack of freed objects as a linked-list.
  //! Pointers to the next object in the stack are stored directly in the
  //! freed object memory. It means that the minimum object size is
  //! `sizeof(void*)`, so MemoryPool will waste memory if you try to store
  //! objects smaller than that.
  //!
  //! It is safe to delete the memory pool without calling `free()` on all
  //! the object as long as you don't need to call their destructors.
  //!
  //! Handles are 32-bit integers. The pool can store at most 4G-1 objects.
  //!
  //! BLOCKBITS defnes the number of objects per block: 1 << BLOCKBITS. The default
  //! is 10 which gives 1024 objects per block.
  template<class TYPE, class LOCKPOLICY = SpinLockPool, int BLOCKBITS = 10>
  class MemoryPool {
  public:
    using Handle = unsigned int;

    static constexpr Handle nullhandle = -1U;

    //! Create a memory pool that allocate fixed-size objects of size
    //! `objectSize`. Allocate memory by blocks of `blockSize` objects.
    MemoryPool()
      : _objectSize(std::max(sizeof(TYPE), sizeof(Handle)))
      , _blockSize(1 << BLOCKBITS)
      , _end(_blockSize)
    {
    }

    //! Destroy the memory pool and free memory. Do *not* call destructors
    //! on allocated objects !
    ~MemoryPool()
    {
      for (size_t i = 0; i < _blocks.size(); ++i) {
        ::free(_blocks[i]);
      }
    }

    //! Return the object for handle \a h
    TYPE& operator[](Handle h)
    {
      return *pointer<TYPE>(h);
    }

    //! Return the object for handle \a h
    const TYPE& operator[](Handle h) const
    {
      return *pointer<TYPE>(h);
    }

    //! Allocate memory for an object of type `TYPE` and returns it. Do *not*
    //! call a constructor !
    //!
    //! Returns nullhandle if out-of-memory.
    Handle allocateRaw()
    {
      typename LOCKPOLICY::guard_type guard(_mutex);
      Handle h;
      if (_firstFree != nullhandle) {
        h = _firstFree;
        _firstFree = *pointer<Handle>(_firstFree);
      }
      else {
        if (_end == _blockSize) {
          if (!allocBlock()) {
            return nullhandle;
          }
        }
        h = ((_currentBlock - 1) << BLOCKBITS) | _end;
        ++_end;
      }

      return h;
    }

    //! Free memory pointed by `ptr. Do *not* call the
    //! destructor. `ptr` *must* have been allocated by `this` !
    void freeRaw(Handle h)
    {
      typename LOCKPOLICY::guard_type guard(_mutex);
      *pointer<Handle>(h) = _firstFree;
      _firstFree = h;
    }

    //! Allocate and construct an object of type `TYPE`. Note that you *must*
    //! destroy the object with `free` so that the destructor is called.
    //! `clear()` and `~MemoryPool` won't call the destructor !
    template <typename... Args >
    inline Handle allocate(Args&&... args)
    {
      Handle h = allocateRaw();
      new(pointer<TYPE>(h)) TYPE(std::forward<Args>(args)...);
      return h;
    }

    //! Destroy and free `ptr`. Should be called on object allocated with
    //! `allocate()`;
    inline void free(Handle h)
    {
      if (h != nullhandle) {
        pointer<TYPE>(h)->~TYPE();
        freeRaw(h);
      }
    }

    //! Free all objects allocated by this memory pool, but do not release
    //! the memory, so the pool can be reused. Do *not* call any destructor.
    void clearRaw()
    {
      _currentBlock = 0;
      _end = 0;
    }

  private:

#if defined(FN_OS_LINUX)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnull-dereference"
#endif
    //! Returns a pointer to the object for handle \a h as a T*
    template<class T>
    inline T* pointer(Handle h)
    {
      if (h == nullhandle) {
        return nullptr;
      }

      TYPE* block = _blocks[h >> BLOCKBITS];
      return reinterpret_cast<T*>(&block[h & ((1 << BLOCKBITS) - 1)]);
    }
#if defined(FN_OS_LINUX)
#pragma GCC diagnostic pop
#endif
    //! Allocate a block, or return an already existing one if clear has been
    //! called. Updates `_currentBlock` and `_end`.
    //!
    //! Returns null if out-of-memory.
    inline bool allocBlock()
    {
      assert(_end == _blockSize);
      TYPE* block = nullptr;
      if (_currentBlock == _blocks.size()) {
        block = reinterpret_cast<TYPE*>(malloc(_objectSize * _blockSize));
        if (!block) {
          return false;
        }
        _blocks.push_back(block);
      }
      else {
        block = _blocks[_currentBlock];
      }

      ++_currentBlock;
      _end = 0;

      return true;
    }

  protected:
    size_t _objectSize; //! The size of one object, in bytes.
    size_t _blockSize; //! The number of object to store in one block.
    std::vector<TYPE*> _blocks; //! The list of blocks.
    Handle _firstFree = nullhandle; //! The first free object in the allocated segment. Forms a linked list.
    size_t _currentBlock = 0; //! The block containing the end of the allocated segment.
    size_t _end; //! The index of the past-the-end object in the current segment.
    typename LOCKPOLICY::lock_type _mutex;
  };
}

#endif
