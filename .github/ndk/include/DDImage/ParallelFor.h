// Copyright (c) 2018 The Foundry Visionmongers Ltd.  All Rights Reserved.

/*! \file ParallelFor.h

 An implementation of parallel for loops using Nuke's thread pool. Use the functions in this file for
 multithreading loops. The main use for this in Nuke is for multithreading ParticleOps and GeoOps as Iops
 are already multithreaded, although it could be useful for rendering PlanarIops
 */


#ifndef DD_Image_ParallelFor_H
#define DD_Image_ParallelFor_H

#ifndef FN_PARALLEL_FOR_THREADS
#define FN_PARALLEL_FOR_THREADS 1 // Use this to turn off multithreading for ease of debugging
#endif

#include "DDImage/Thread.h"
#include <atomic>

namespace DD {
  namespace Image {
    namespace detail {
      // Private implementation details
      template <typename Function>
      class parallel_for_impl {
      public:
        parallel_for_impl( Function f, unsigned int numItems, unsigned int maxThreads, unsigned int minItems )
          : _numItems(numItems)
          , _numThreads(1)
          , _function(f)
          , _exception(nullptr)
          , _exceptionLock()
        {
          _exceptionLock.clear();
#if FN_PARALLEL_FOR_THREADS
          if ( maxThreads != 1 && _numItems >= minItems ) {
            _numThreads = std::min(std::max(_numItems, 1U), Thread::numThreads);
            if ( maxThreads != 0 )
              _numThreads = std::min(_numThreads, maxThreads);
          }
#else
          (void)maxThreads;   // Unused
          (void)minItems; // Unused
#endif
        }

        template <typename InternalFunctor>
        void loop( InternalFunctor threadFunc )
        {
          if ( _numThreads > 1 ) {
            Thread::spawn(threadFunc, _numThreads, const_cast<parallel_for_impl*>(this));
            Thread::wait(this);
          }
          else
            threadFunc(0, 1, const_cast<parallel_for_impl*>(this));
          if ( _exception )
            std::rethrow_exception( _exception );
        }

        static void UnitThreadFunc(unsigned int threadIndex, unsigned int nThreads, void* data)
        {
          parallel_for_impl *workload = reinterpret_cast<parallel_for_impl*>(data);
          try {
            std::pair<unsigned int, unsigned int> range = workload->range(threadIndex, nThreads);
            for ( int i = range.first; i < range.second; i++ )
              workload->_function(i);
          }
          catch (...) {
            if ( !workload->_exceptionLock.test_and_set(std::memory_order_relaxed) )
              workload->_exception = std::current_exception();
          }
        };

        static void RangeThreadFunc(unsigned int threadIndex, unsigned int nThreads, void* data)
        {
          parallel_for_impl *workload = reinterpret_cast<parallel_for_impl*>(data);
          try {
            std::pair<unsigned int, unsigned int> range = workload->range(threadIndex, nThreads);
            workload->_function(range.first, range.second);
          }
          catch (...) {
            if ( !workload->_exceptionLock.test_and_set(std::memory_order_relaxed) )
              workload->_exception = std::current_exception();
          }
        };

      private:
        std::pair<unsigned int, unsigned int> range(unsigned int threadIndex, unsigned int nThreads) const
        {
          const unsigned int blockSize = (_numItems+nThreads-1)/nThreads;
          const unsigned int start = blockSize*threadIndex;
          return std::pair<unsigned int, unsigned int>( start, std::min(start+blockSize, _numItems) );
        }

        unsigned int _numItems;
        unsigned int _numThreads;
        Function     _function;
        std::exception_ptr _exception;
        std::atomic_flag _exceptionLock;
      };
    }


    //! Loop through the given number of items, calling the supplied function for each item with the index as the parameter.
    //! If maxThreads != 1, this will spawn multiple threads. If maxThreads == 0 (the default), it will
    //! use as many threads as are available. If there are less than minItems, it won't spawn any threads
    //! to avoid the overhead of thread spawning. It may be worth reducing this for expensive per-item
    //! operations.
    template <typename Function>
    void ParallelFor( unsigned int numItems, Function f, unsigned int maxThreads = 0, unsigned int minItems = 1000 )
    {
      using Impl = detail::parallel_for_impl<Function>;
      Impl impl(f, numItems, maxThreads, minItems );
      impl.loop(Impl::UnitThreadFunc);
    }

    //! Sometimes you need to set up a cache or other state which requires per-thread storage, in which case
    //! ParallelFor won't help. ParallelForRange takes one step back and lets your function do the looping.
    //! All it does is divide the items into ranges and then calls the supplied function for each range.
    //! If you have N threads in the thread pool, the items will be divided into N ranges, and the function
    //! called once for each range. If threading is turned off, this is just the same as calling
    //! f(0, numItems);
    template <typename Function>
    void ParallelForRange( unsigned int numItems, Function f, unsigned int maxThreads = 0, unsigned int minItems = 1000 )
    {
      using Impl = detail::parallel_for_impl<Function>;
      Impl impl(f, numItems, maxThreads, minItems );
      impl.loop(Impl::RangeThreadFunc);
    }

  }
}

#endif
