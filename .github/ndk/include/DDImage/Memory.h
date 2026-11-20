// Copyright (c) 2009 The Foundry Visionmongers Ltd. All Rights Reserved.
#ifndef DDIMAGE_MEMORY_H
#define DDIMAGE_MEMORY_H

#include <DDImage/DDImage_API.h>
#include <DDImage/Thread.h>
#include <DDImage/IAllocator.h>

#include <iostream>
#include <assert.h>
#include <string>
#include <sstream>
#include <vector>
#include <map>

// Set FN_TRACK_MEMORY_ALLOCATIONS to 1 to allow tracking of memory allocation/deallocation of Node, Op 
// and Knob objects, or 0 otherwise. When enabled, any such objects still allocated after a scriptClear() 
// operation will be listed in standard output.
//
// When FN_TRACK_MEMORY_ALLOCATIONS is set to 1, tracking will not actually be enabled unless the 
// environment variable NUKE_TRACK_OBJECT_ALLOCATIONS is also set to 1.
//
// Notes:
// 1) A small additional performance cost will be incurred when creating/destroying Node, Op and Knob
//    objects with tracking enabled.
// 2) Memory allocated on the heap by any of these objects will not be tracked - only the above objects 
//    themselves.
// 3) The DDImage API does not provide access to the memory tracking information (this is internal only).
#define FN_TRACK_MEMORY_ALLOCATIONS 1

namespace DD
{
  namespace Image
  {

    class Format;
    class Knob;
    class NodeI;
    class Op;
    class MemoryHolder;

    /** \class DD::Image::Memory

       Memory manager for cache-like objects that can be freed and
       recreated if necessary so that their memory can be used for
       other purposes. The output buffers on Iop and the 8-bit buffers
       used by the Nuke viewer, and other objects fall into this
       class.

       Each such object should call register_user() in it's constructor and
       unregister_user() in it's destructor.

       Any code that uses allocate() and deallocate() will trigger
       reduce_current_usage() if the total allocated by these calls goes
       over max_usage(). This will only work well if the majority of large
       objects allocated by Nuke and plugins calls these functions, so that
       current_usage() reflects the actual amount of memory being used.
       Code that uses new/delete (and malloc on Windows) will also trigger
       reduce_current_usage() when the std::new_handler() is called. But this
       only happens when you run out of memory which may be somewhat late
       for safe recovery.

       allocate() will also be 16-byte aligned to allow use of vector
       operations.

       Nuke has a choice of 3 allocators to use for allocating Memory that can
       be set by the environment variable `NUKE_ALLOCATOR` before starting the
       application. The allocators are:

        - "System" the Operating System allocator.

          The System allocator has been shown to use the least amount of memory
          as it releases memory to the OS immediately. However, this comes at a
          cost in performance and has been shown to be the slowest. This is the
          default allocator for Linux and Apple macOS®.

        - "TC" Google® Thread Cache fast, multi-threaded malloc implementation.

          The TC allocator has been shown to increase the memory usage but not
          as much as TBB, as well as increase performance but not as much as TBB
          and is a good middle ground between the System and TBB allocators.
          This is the default allocator for Microsoft Windows® where the System
          allocator has shown itself to be significantly slower in certain
          circumstances.

        - "TBB" Intel® Thread Building Blocks scalable malloc.

          The TBB allocator has been shown to have the best performance although
          it uses the most memory compared to the other allocators. If memory is
          not a concern this allocator may be the best choice.

        For example to use the TBB allocator set the environment variable:
        `NUKE_ALLOCATOR="TBB"` before launching Nuke.
     */
    class DDImage_API Memory
    {
    public:

      /** \class DD::Image::Memory::MemoryInfo
        MemoryInfo class provides a mechanism to fill in information about how much memory a given MemoryHolder is using,
        attributing it to a given "owner".

        There are currently three types of owner MemoryInfo's can be attributed to, these being Knobs, Nodes and Ops. If the memory is
        not part of these then it is acceptable to use NULL (although this means the a Node's memory usage will be incorrect)

        In addition to the memory usage, a user can also add key/value pairs for additional information, which is included in the memory
        dump output.
      */
      class DDImage_API MemoryInfo
      {
      public:

        enum OwnerType 
        {
          eNode = 0,
          eOp,
          eKnob
        };

        MemoryInfo(const NodeI* node, size_t totalUsage) : _node(node), _userData(), _totalUsage(totalUsage), _ownerType(eNode) { }
        MemoryInfo(const Op* op, size_t totalUsage) : _op(op), _userData(), _totalUsage(totalUsage), _ownerType(eOp) { }
        MemoryInfo(const Knob* knob, size_t totalUsage) : _knob(knob), _userData(), _totalUsage(totalUsage), _ownerType(eKnob) { }

        size_t getTotalUsage() const { return _totalUsage; }
        OwnerType getOwnerType() const { return _ownerType; }
        
        std::string getName() const;
        size_t getUserDataCount() const { return _userData.size(); }

        const std::string& getUserDataName(size_t index) const { return _userData[index].first; }
        const std::string& getUserDataValue(size_t index) const { return _userData[index].second; }

        template<class T>
        void addUserData(const std::string& name, const T& value)
        {
          std::stringstream v;
          v << value;
          _userData.push_back(NameValuePair(name, v.str()));
        }

        void addUserData(const std::string& name, const std::stringstream& value)
        {
          _userData.push_back(NameValuePair(name, value.str()));
        }

        const NodeI* getNode() const;

        friend std::ostream& operator<<(std::ostream& outstream, const MemoryInfo& memInfo);

      private:

        typedef std::pair<std::string, std::string> NameValuePair;
        typedef std::vector<NameValuePair> UserData;

        // to prevent incorrect instantiation
        MemoryInfo();

        // Owner getters
        const NodeI* getOwnerAsNode() const 
        {
          mFnAssert(_ownerType == eNode);
          return (_ownerType == eNode) ? _node : nullptr;
        }

        const Op* getOwnerAsOp() const 
        {
          mFnAssert(_ownerType == eOp);
          return (_ownerType == eOp) ? _op : nullptr;
        }

        const Knob* getOwnerAsKnob() const
        {
          mFnAssert(_ownerType == eKnob);
          return (_ownerType == eKnob) ? _knob : nullptr;
        }

        union {
          const NodeI*  _node;
          const Op*     _op;
          const Knob*   _knob;
        };

        UserData      _userData;
        size_t        _totalUsage;
        OwnerType     _ownerType;
      };

      typedef std::vector<MemoryInfo> MemoryInfoArray;

      typedef std::map<const NodeI*, MemoryInfoArray> MemoryInfoMap;

      /**
         Returns the number of register_user() calls that have not had
         an unregister_user() call done to it. Used for debugging.
       */
      static int count_users();

      template <typename T>
      static T* allocate( size_t num, size_t alignment = 0 )
      {
        return (T*)(allocate_void(num * sizeof(T), alignment));
      }

      template <typename T>
      static void deallocate( T* ptr )
      {
        deallocate_void((void*)ptr);
      }

      // calculates a map of memory infos to nodes, which can be used to determine memory usage
      static void generateMemoryInfo( MemoryInfoMap& outMemInfoMap, const void* restrict_to = nullptr );

      /**
         Prints a report about all the users and the total memory usage to
         the stream. Also takes a \a format_bytes argument which, if true, returns the bytes in
         a human readable format.

         If \a restrict_to is non-zero then only users whose info() command
         tests and matches the restrict_to() are printed, and no summary
         information is used. This is currently used to dump only the info
         about users that belong to a Node. Result may be a zero-length
         string if no users match.

         If \a includeNodeInfo is true, Node pointers associated with memory objects
         (eg Ops/Knobs) will be dererenced for name information.  This is unsafe during shutdown,
         where the associated Nodes may have been deleted.
       */
      static void dump_info( std::ostream& output, bool format_bytes, bool includeNodeInfo, const void* restrict_to );

      /**
          Registers an allocator with the Memory system, which allows us to track how
          much memory a given allocator is using.

          An allocator instance can only be registered once, and print an error if added
          twice. Returns if allocator was successfully registered
      */
      static bool register_allocator( IAllocator* pAllocator );

      /**
          Unregisters an allocator from the Memory system
          Will print an error message if trying to remove an allocator that's already been removed.
          Returns if allocator was successfully unregistered
      */
      static bool unregister_allocator( IAllocator* pAllocator );

      /**
          Gets the number of allocators currently registered with the memory system
      */
      static size_t get_num_registered_allocators();

      /**
          Gets the registered allocator at a given index
      */
      static IAllocator* get_allocator(size_t index);

      /**
          Finds the registered allocator by name and returns the pointer.
          returns NULL if it can't find one
      */
      static IAllocator* find_allocator( const char* pAllocatorName );

      /** 
          Helper function that creates and registers an allocator of type T, or if it already exists
          returns the allocator pointer.
      */
      template< typename T >
      static T* create_allocator( const char* pAllocatorName )
      {
        DD::Image::Guard guard(_lock);

        IAllocator* allocator = find_allocator(pAllocatorName);

        if ( allocator == nullptr ) {
          allocator = new T(pAllocatorName);
          register_allocator(allocator);
        }

        return IAllocator::getAllocator<T>(allocator);
      }

      /**
          Prints and XML report about all registered allocators, including
          the current memory usage and the highwater mark.

          Also takes a \a format_bytes argument which, if true, returns the bytes in
          a human readable format.
      */
      static void dump_allocatorinfoXML( std::ostream& output, bool format_bytes );

      /**
         Prints an XML report about all the users and the total memory usage to
         the stream. Also takes a \a format_bytes argument which, if true, returns the bytes in
         a human readable format.

         If \a restrict_to is non-zero then only users whose info() command
         tests and matches the restrict_to() are printed, and no summary
         information is used. This is currently used to dump only the info
         about users that belong to a Node. Result may be a zero-length
         string if no users match.

         If \a includeNodeInfo is true, Node pointers associated with memory objects
         (eg Ops/Knobs) will be dererenced for name information.  This is unsafe during shutdown,
         where the associated Nodes may have been deleted.
       */
      static void dump_infoXML( std::ostream& output, bool format_bytes, bool includeNodeInfo, const void* restrict_to );

      /*! Use this class to print memory usage to a stream, just write o << Memory::bytes(n) */
      class bytes
      {
      public:
        long long unsigned n;
        bool metric;
        bytes(long long unsigned a, bool m = true) : n(a), metric(m) {}
      };

      /** Prints a number of bytes in a user-friendly form, for instance 123.4MiB,
          1024KiB, or 513B. If \a metric is true then use powers of 10 rather than
          powers of 2, and remove the 'i' from the suffixes.
       */
      static void print_bytes(std::ostream& output, long long unsigned bytes, bool metric = true);

      /** Puts the same result as print_bytes() into the passed buffer, which
          must be at least 11 bytes long. Returns the number of bytes written,
          not counting the nul terminator.
       */
      static int format_bytes( char* buffer, long long unsigned bytes, bool metric = true );

      /**
         Sets total_ram() by asking the system for this information, and sets
         max_usage() to 50% of that. This is automatically done by many Memory
         calls, but you should do this if you wish to change the max_usage
         to some value other than this default.

         Also calls std::set_new_handler(). If you don't want this, call this
         and then call std::set_new_handler() to set the value back. By default
         running out of memory in new (and in malloc in Windows) will cause it
         to clean up memory so it will work, and will throw a bad_alloc exception
         if it does not work.
       */
      static void initialize();

      static size_t max_usage() { return _max_usage; }

      /*!
         Return number of bytes allocated by allocate_void().
         This function is not inline in case we need to add a lock around the reference
       */
      static size_t current_usage();
      static size_t total_ram() { return _total_ram; }
      static U64 total_ram_64() { return _total_ram_64; }
      static size_t total_vm() { return _total_vm; }
      static int free_count() { return _free_count; }
      static int new_handler_count() { return _new_handler_count; }

      /*!
         Reduce current_usage() to \a target or smaller. Returns true if
         it deleted anything (check current_usage() to see if you really hit the target)
       */
      static bool set_current_usage( size_t target, const char* message = nullptr );

      /*!
         Reduce the amount of memory being used. Returns true if anything is freed.
         The message, if not zero, should id the caller is used to print debugging info.
       */
      static bool reduce_current_usage( const char* message = nullptr );
      static void set_max_usage( size_t new_max );
      static void set_hard_max_usage( size_t final_max );
      static void set_free_count( int );
      static void set_new_handler_count( int );

      /*! TODO: REMOVE THIS FUNCTION! */
      static void* allocate_remember_size(size_t size)
      {
        return allocate_void(size);
      }

      /*! Allocate \a bytes bytes of memory and return as a void*. Optionally align memory (also rounds up allocated amount to multiple of alignment) */
      static void* allocate_void(size_t bytes, size_t alignment = 0);

      /*! TODO: REMOVE THIS FUNCTION! */
      static void deallocate_remember_size(void *ptr)
      {
        deallocate_void(ptr);
      }

      /*! Free memory created with allocate_void()
          TODO: REMOVE num ARG! */
      static void deallocate_void(void* ptr, size_t num = 0);

      //! Returns the size actually allocated for a given pointer
      //! NOTE: the size returned may be bigger than requested, implementation dependent.
      //! This effectively calls the following:
      //! Windows: HeapSize()
      //! OSX: malloc_size()
      //! Linux: malloc_usable_size()
      static size_t get_allocated_size(void* ptr);

      /*!
         Reduce width & height to a box so that can be allocated.

         Currently an absolute maximum of 1Mb on each dimension, and an absolute
         maximum of 64k^2 (4Gb) on the area. This has been chosen so that several
         dozen 1-D arrays of pointers or floats can be allocated without running
         out of memory.

         In addition each dimension is limited to 16k or the size of the format,
         whichever is larger. This smaller limit has been chosen so that the
         entire image can probably be allocated at once. But if the user chooses
         a very large format this limit is ignored and it may try to allocate
         far more memory than will fit. Many operations in Nuke will still work
         with these very large operations (especially in 64-bit versions).

         Returns true if it changes the width or height.
       */
      static bool clipToCacheLimit(int& width, int& height, const DD::Image::Format& format);

    private:
      // Make every MemRegistrant a friend
      template<class MemoryHolderT> friend class MemRegistrant;

      static void register_user( DD::Image::MemoryHolder* user );
      static void unregister_user( DD::Image::MemoryHolder* user );

      static DD::Image::RecursiveLock _lock;

      static size_t        _max_usage;
      static size_t        _total_ram;
      static size_t        _total_vm;
      static int           _free_count;
      static int           _new_handler_count;
      
      // Additional storage for 32 bit app
      // to store physical ram size of over 2GB
      static U64           _total_ram_64;
      // And a hard maximum
      static size_t        _final_max_usage;

      static void new_handler();

      Memory() {}
      ~Memory() {}

    };

    inline std::ostream& operator<<(std::ostream& o, const Memory::bytes& c)
    {
      Memory::print_bytes(o, c.n, c.metric);
      return o;
    }

  }
}

#endif
