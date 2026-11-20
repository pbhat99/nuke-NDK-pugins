// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

/*! \file Thread.h

   Defines the classes Thread, Lock, and SignalLock. These provide
   portable wrappers around native thread functions (pthreads on
   Unix and CriticalRegion and Events on Windows)

   Do not include this file if you do not need it. On Windows
   it includes <windows.h> which causes considerable havoc by
   polluting the namespace with macros and symbols.
 */

#ifndef Thread_H
#define Thread_H

#include "DDImage_API.h"

#if !defined(_WIN32)
////////////////////////////////////////////////////////////////
// pthreads:

#include <pthread.h>
#include <stdexcept>
#include <string>

#else
////////////////////////////////////////////////////////////////
// _WIN32:

// Properly includes windows.h with the essential #define _WIN32_WINNT.
#include "DDWindows.h"

#endif

namespace DD
{

namespace Image
{

#ifdef SPROC_PID
  typedef pid_t ThreadId;
#elif defined(_WIN32)
  typedef unsigned ThreadId;
#else
  typedef pthread_t ThreadId;
#endif


#if !defined(_WIN32)
////////////////////////////////////////////////////////////////
// pthreads:

class DDImage_API Lock
{
  friend class SignalLock;
  pthread_mutex_t mutex;
  Lock(const Lock &);
  Lock& operator=(const Lock&);
protected:
  Lock(const pthread_mutexattr_t * a) { pthread_mutex_init(&mutex, a); }
public:
  #if defined(__linux)
  static pthread_mutexattr_t attrib;
  Lock() { pthread_mutex_init(&mutex, &attrib); }
  #else
  Lock() { pthread_mutex_init(&mutex, nullptr); }
  #endif
  pthread_mutex_t& get_mutex() { return mutex; }
  void lock() { pthread_mutex_lock(&mutex); }
  void spinlock() { while (pthread_mutex_trylock(&mutex))
                      sched_yield();}
  void unlock() { pthread_mutex_unlock(&mutex); }
  bool trylock() { return pthread_mutex_trylock(&mutex) == 0; }
  void spinsignal() {}
  void spinwait() { unlock();
                    sched_yield();
                    spinlock(); }
  virtual ~Lock() { pthread_mutex_destroy(&mutex); }
};

class DDImage_API SignalLock : public Lock
{
  pthread_cond_t cond;
public:
  SignalLock() : Lock() { pthread_cond_init(&cond, nullptr); }
  ~SignalLock() override { pthread_cond_destroy(&cond); }

  void signal() { pthread_cond_broadcast(&cond); }
  void signal_one() { pthread_cond_signal(&cond); }
  bool wait(unsigned long timeoutms = 0) {
    bool waitOk = true;
    if ( timeoutms == 0 )
      pthread_cond_wait(&cond, &mutex);
    else {
      struct timespec ts;
      ts.tv_sec = 0;
      ts.tv_nsec = timeoutms * 1000;
      if ( pthread_cond_timedwait( &cond, &mutex, &ts) != 0 ) {
        waitOk = false;
      }
    }
    return waitOk;
  }
  void spinsignal() {}
  void spinwait() { unlock();
                    sched_yield();
                    spinlock(); }
};


///
/// @brief Read/Write Lock for shared access
///
/// A Read/Write lock that allows multiple readers to gain access to the lock,
/// but only one writer at a time.
///
/// This lock is not recursive.
///
class DDImage_API ReadWriteLock
{
private:
  // hide CC and assign operator
  ReadWriteLock(const ReadWriteLock &) ;
  ReadWriteLock& operator= (const ReadWriteLock&) ;

protected:
  pthread_rwlock_t _lock; ///< my pthread rwlock that implements this

public:
  ReadWriteLock()
  {
    const int error = pthread_rwlock_init(&_lock, nullptr);
    if (error != 0) {
      throw std::runtime_error("DD::Image::ReadWriteLock "
        "pthread_rwlock_init failed with code " + std::to_string(error));
    }
  }

  ~ReadWriteLock()
  {
    pthread_rwlock_destroy(&_lock);
  }

  ///
  /// Acquire lock for reading, this only blocks writes not other reads.
  ///
  /// @return true if successfully acquired the lock.
  ///
  bool readLock()
  {
    return pthread_rwlock_rdlock(&_lock) == 0;
  }

  /// Try the lock for reading, this only block writes
  bool tryReadLock()
  {
    return pthread_rwlock_tryrdlock(&_lock) == 0;
  }

  /// Lock for writing, this blocks reads and all writes
  bool writeLock()
  {
    return pthread_rwlock_wrlock(&_lock) == 0;
  }

  /// Try the lock for writing, this blocks reads and all writes
  bool tryWriteLock()
  {
    return pthread_rwlock_trywrlock(&_lock) == 0;
  }

  ///
  /// Unlock an already acquired lock.
  ///
  /// @return true if successfully unlocked the lock.
  ///
  bool unlock()
  {
    return pthread_rwlock_unlock(&_lock) == 0;
  }
};

// Try to detect if recursive locks are supported:
  #if defined(__linux) || defined(PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP)

class DDImage_API RecursiveLock : public Lock
{
  static pthread_mutexattr_t attrib;
public:
  RecursiveLock() : Lock(&attrib) {}
};

  #elif defined(PTHREAD_MUTEX_RECURSIVE)

class DDImage_API RecursiveLock : public Lock
{
public:
  RecursiveLock();
};

  #else // If not, implement recursive locks ourselves:

    #error Too much code assumes RecursiveLock and Lock are the same thing!
// This could be made to work if we fixed the Guard objects to be able to
// correctly handle the different types of lock.

class DDImage_API RecursiveLock : public Lock
{
  pthread_t owner;
  int counter;
public:
  RecursiveLock() : Lock(), counter(0) {}
  void lock()
  {
    if (!counter || owner != pthread_self()) {
      Lock::lock();
      owner = pthread_self();
      counter = 1;
    }
    else {
      ++counter;
    }
  }
  void spinlock()
  {
    if (!counter || owner != pthread_self()) {
      Lock::spinlock();
      owner = pthread_self();
      counter = 1;
    }
    else {
      ++counter;
    }
  }
  bool trylock()
  {
    if (!counter || owner != pthread_self()) {
      if (!Lock::trylock())
        return false;
      owner = pthread_self();
    }
    counter++;
    return true;
  }
  void unlock()
  {
    if (!--counter)
      Lock::unlock();
  }
};

  #endif

#else
////////////////////////////////////////////////////////////////
// _WIN32:

class DDImage_API Lock
{
protected:
  CRITICAL_SECTION cs;

  //! The Windows critical section code doesn't give access to its internal lock count for the same thread,
  //! so we store it separately here.  The setting isn't thread-safe as such, but is only used when the
  //! thread has the lock.
  int _lockCount;

public:
  Lock() {
    InitializeCriticalSectionAndSpinCount(&cs, 4000);
    _lockCount = 0;
  }

  void lock()
  {
    // This will allow recursive locks on Windows, but as the alternative is deadlock it shouldn't be a concern.
    EnterCriticalSection( &cs );
    _lockCount++;
  }

  void spinlock()
  {
    // NOTE: if errors come up on the following line but not before, then windows.h was included without an
    // appropriate _WIN32_WINNT version set. Insert #include "DDImage/DDWindows.h" at the top of your file is
    // the easiest way to fix this.
    while ( !TryEnterCriticalSection( &cs ) )
      SwitchToThread();
    _lockCount++;
  }

  //! Don't allow multiple locks per thread in this function, to give the same behaviour as other platforms
  virtual bool trylock()
  {
    if (_lockCount > 0)
      return false;
    if (TryEnterCriticalSection( &cs )) {
      _lockCount++;
      return true;
    }
    return false;
  }

  void unlock()
  {
    _lockCount--;
    LeaveCriticalSection(&cs);
  }

  void spinsignal() {}

  void spinwait()
  {
    unlock();
    SwitchToThread();
    spinlock();
  }

  virtual ~Lock() { DeleteCriticalSection(&cs); }
};

class DDImage_API SignalLock : public Lock
{
public:
  HANDLE event;
  SignalLock() : Lock() { event = CreateEvent(0, TRUE, FALSE, 0); }
  virtual ~SignalLock() { CloseHandle(event); }

  void signal() { SetEvent(event); }
  void signal_one() { signal(); }
  bool wait(unsigned long timeoutms = 0);
  void spinsignal() {}
  void spinwait() { unlock();
                    SwitchToThread();
                    spinlock(); }
};

// Recursive lock: standard Windows functionality
class DDImage_API RecursiveLock : public Lock
{
public:
  //! Override the base class to allow recursive locks
  virtual bool trylock()
  {
    if (TryEnterCriticalSection( &cs )) {
      // We don't actually care about the lock count in this child class,
      // but will maintain it here to avoid inconsistency and confusion
      _lockCount++;
      return true;
    }
    return false;
  }
};

/// A read/write lock that allows multiple readers to
/// gain access to the lock, but only one writer
/// at a time.
///
/// The current implementation may not be recursive
/// as I can't figure out how to make it so or if
/// it is so by default.
class DDImage_API ReadWriteLock
{
private:

  HANDLE _access;
  HANDLE _writer;
  HANDLE _canread;
  HANDLE _canwrite;
  int _readers;

  // hide CC and assign operator
  ReadWriteLock(const ReadWriteLock &) ;
  ReadWriteLock& operator= (const ReadWriteLock&) ;

  // wait for objects
  bool wait( int count, HANDLE* obj, DWORD timeout );

  // lock and unlock methods
  bool lockRead ( DWORD timeout );
  bool lockWrite( DWORD timeout );
  bool unlockRead( DWORD timeout, bool lockaccess );
  bool unlockWrite( DWORD timeout, bool lockaccess );

public:
  /// ctor creates with defaults
  ReadWriteLock();

  /// dtor destroys it
  ~ReadWriteLock();

  /// Lock for reading, this only block writes, not other reads
  /// @return true if successfully acquired the lock
  bool readLock();

  /// Try the lock for reading, this only block writes,
  bool tryReadLock();

  /// lock for writing, this blocks reads and all writes
  bool writeLock();

  /// lock for writing, this blocks reads and all writes
  bool tryWriteLock();

  /// Unlock an already acquired lock
  /// @return true if successfully unlocked the lock
  bool unlock();
};

#endif

////////////////////////////////////////////////////////////////

/// Base class for Read/Write Guard classes.
class DDImage_API BaseReadWriteGuard
{
protected:
  BaseReadWriteGuard(ReadWriteLock & m) : _lock(m) {}

  ReadWriteLock& _lock;
};

/// guard that performs a read lock on ctor and
/// on lock on dtor. Use for scoping a read/write
/// lock to get automatic unlocking
class DDImage_API ReadGuard : public BaseReadWriteGuard
{
public:
  const bool isReadLocked;

  ReadGuard(ReadWriteLock & m)
    : BaseReadWriteGuard(m)
    , isReadLocked(_lock.readLock())
  {
  }

  ~ReadGuard()
  {
    if (isReadLocked) {
      _lock.unlock();
    }
  }
};

/// guard that performs a write lock on ctor and
/// on lock on dtor. Use for scoping a read/write
/// lock to get automatic unlocking
class DDImage_API WriteGuard : public BaseReadWriteGuard
{
public:
  const bool isWriteLocked;

  WriteGuard(ReadWriteLock & m)
    : BaseReadWriteGuard(m)
    , isWriteLocked(_lock.writeLock())
  {
  }

  ~WriteGuard()
  {
    if (isWriteLocked) {
      _lock.unlock();
    }
  }
};

/// guard that performs a try write lock on ctor and
/// on lock on dtor. Use for scoping a read/write
/// lock to get automatic unlocking
class DDImage_API TryWriteGuard : public BaseReadWriteGuard
{
public:
  const bool isWriteLocked;

  TryWriteGuard(ReadWriteLock & m)
    : BaseReadWriteGuard(m)
    , isWriteLocked(_lock.tryWriteLock())
  {
  }

  ~TryWriteGuard()
  {
    if (isWriteLocked) {
      _lock.unlock();
    }
  }
};

/// base lock guard, so multiple types of guards
/// can be passed to a function
class DDImage_API BaseLockGuard
{
protected:
  BaseLockGuard(Lock & m) : _lock(m) { }

  Lock& _lock;
};

class DDImage_API Guard : public BaseLockGuard
{
public:
  Guard(Lock & m) : BaseLockGuard(m) { _lock.lock(); }
  Guard(Lock * m) : BaseLockGuard(*m) { _lock.lock(); }
  ~Guard() { _lock.unlock(); }
};

class DDImage_API SpinGuard : public BaseLockGuard
{
public:
  SpinGuard(Lock & m) : BaseLockGuard(m) { _lock.spinlock(); }
  SpinGuard(Lock * m) : BaseLockGuard(*m) { _lock.spinlock(); }
  ~SpinGuard() { _lock.unlock(); }
};

class DDImage_API TryLockGuard : public BaseLockGuard
{
public:
  TryLockGuard(Lock & m) : BaseLockGuard(m) { _islocked = _lock.trylock(); }
  TryLockGuard(Lock * m) : BaseLockGuard(*m) { _islocked = _lock.trylock(); }
  bool IsLock() { return _islocked; }
  ~TryLockGuard() { if (_islocked)
                      _lock.unlock(); }

private:
  bool _islocked;
};

/*! \class Thread

   Namespace of functions to create and wait for parallel threads.

   "Real" threads as seen by the operating system are reused. After the function
   passed to spawn() returns, the thread will wait until another spawn() is called
   and will pick up the function from that. Therefore the operating system
   overhead of creating and destroying threads is avoided and you can call
   spawn() as much as you want even with short functions.

   For synchronization see the classes Lock, SignalLock, and Guard.
 */
class DDImage_API Thread
{
  Thread(); // there is no object, just static methods
public:
  typedef void (*ThreadFunction)(unsigned index, unsigned nThreads, void* userData);
  struct ThreadInfo
  {
    ThreadFunction function;
    unsigned index, nThreads;
    void* userData;
  };

  struct RunningThreadsInfo 
  {
    int numRunningThreads;
    int numThreads;
  };

  static bool spawn(ThreadFunction, unsigned nThreads, void* userData);

  static ThreadId GetThreadId();

  /*!
     Returns a pointer to a structure containing the arguments passed to the spawn()
     that launched this thread, and the index number passed to the function for this
     thread. This pointer is good until the thread function exits.
  
     Returns null if this thread does not appear to have been launched by spawn(),
     this is usually because you are in the main thread.
   */
  static const ThreadInfo* thisThread();
  static int thisIndex() { const ThreadInfo* i = thisThread();
                           return i ? int(i->index) : -1; }
  /// returns true if we're on the main-thread, false otherwise
  static bool OnMainThread();

  static void terminate_all();
  static bool running(void* userData);
  static void wait(void* userData);
  static RunningThreadsInfo getRunningThreadsInfo();

  static unsigned numCPUs; //!< recommended count for compute-only threads
  static unsigned numThreads; //!< recommended count for threads that will do I/O
  static unsigned numThreadsPerReadWriteTask; //!< number of threads to use when multi-threading read or write ops in NukeEngine

  /// the minimum stack size each pthread can have
  static size_t gMinStackSize;

  /// absolute minimum stack size permissible
  static const size_t kAbsoluteMinStackSize = 1024 * 1024;

};

/*! This is for back compatibility only */
typedef SignalLock SlowSignalLock;

void DDImage_API SleepMS( int ms );
inline void sleepFor( double seconds ) { SleepMS( int(seconds * 1000 + .5) ); }

//! Threading needs to know whether the GUI is active for priority settings
DDImage_API void setThreadGUIMode(bool guiMode);

} // namespace Image

} // namespace DD

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
