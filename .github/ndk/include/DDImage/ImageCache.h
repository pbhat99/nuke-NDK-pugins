/*
 * ImageCache.h
 *
 * See fnImageCache.cpp for more information
 *
 * Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
 */

#ifndef Image_Cache_H
#define Image_Cache_H

#include "DDImage/Thread.h"
#include "DDImage/Hash.h"
#include <stdio.h>
#include <map>

namespace DD
{
  namespace Image
  {

    class Image_Cache_Entry
    {
    public:
      unsigned blocks;
      unsigned age;
      Image_Cache_Entry(unsigned b, unsigned a)
      {
        blocks = b;
        age = a;
      }
    };

    typedef std::map<DD::Image::Hash, Image_Cache_Entry> Cache_Entry_Map;

    //! Holder of temporary data to read files from cache:
    class DDImage_API ImageCacheReadI
    {
    protected:
      virtual ~ImageCacheReadI() { }
    public:
      /**
         Read a block of data from a file opened with Image_Cache::open(). Returns number
         of bytes read.
      */
      virtual size_t read(void* to, size_t n) = 0;

      /**
       * Try to read a certain number of bytes.  If it fails to read that number, return false.
       */
      bool tryRead(void* t, size_t n)
      {
        return read(t, n) == n;
      }

      /**
         Move the file pointer to an absolute position in the file.
      */
      virtual void seek(size_t to) = 0;

      /**
         Move the file pointer by \a n bytes.
      */
      virtual void seekRelative(off_t offset) = 0;

      /**
         Close the file and delete the \a r data structure.
      */
      virtual void close() = 0;

      /**
       * get a unique ID for the memory-cached data, or 0 if the data was not in
       * the memory cache.  if the contents of the memory-cached data change (or if
       * the data is removed from the memory cache and then reloaded) this number will increase.
       */
      virtual int uniqueID() const = 0;
    };

    //! Holder of temporary data to write files to cache:
    class DDImage_API ImageCacheWriteI
    {
    protected:
      virtual ~ImageCacheWriteI() { }

    public:
      /**
         Write a block of data to the file.
      */
      virtual size_t write(const void* data, size_t n) = 0;
      
      /**
         Close the file and delete the \a write data structure. At this point
         the add_file is done.
      */
      virtual void close() = 0;      
    };

    /** \class DD::Image::Image_Cache
       Manages a large LRU cache of files on disk. The files are identified
       by a DD::Image::Hash. The caller is responsible for the data in the
       files.
     */
    class DDImage_API Image_Cache
    {
      Cache_Entry_Map map;
      unsigned max_blocks, blocks, age;
      std::string _path;
      bool _active;
      bool _read;
      bool _written;
      void create_cache_dirs();   // Turn on _written
      void map_disk_cache();      // Turn on _read
      void scan_disk_cache();
      Lock mlock; // Lock thread data access, lock map access

    public:

      void rescan_disk_cache();
      void touch_root();

      Image_Cache();

      /**
         Return the cache used by the Nuke viewer. You probably want to reuse
         this for any caching unless you actually have an api for the user to
         name and allocate space for a different cache.
       */
      static Image_Cache& mainCache();

      /**
         Set the base directory filename and the size in gigabytes.

         A blank filename or zero size will deactivate the cache so that it
         does not do anything (call remove_all_files() first if you really
         want the disk space released).
       */
      void initialize(const std::string& path, int sizegb);
      bool is_active() { return _active; }    //!< True if there has not been a failure and size is non-zero
      bool is_read() { return _read; }        //!< True if read() has been called and thus it has loaded map
      bool is_written() { return _written; }  //!< True if write() has been called and it created cache directories

      void remove_oldest_files(int targetblocks);
      void remove_all_files();

      /**
         Remove the file (if it exists) for the given hash. This is a good
         idea if you attempt to read it and then determine that the data
         is no good, perhaps an obsolete version of the data.
         Returns true if has_file() was true.
       */
      bool remove(const DD::Image::Hash&);
      void print();

      //! ImageCacheReadI cache files
      /**
         Open a file of cached data.

         Allocates and returns a pointer to a data structure that you must
         pass to the read() and close() methods. Returns NULL if the cached
         data does not exist.
       */
      ImageCacheReadI* open(const DD::Image::Hash&, bool onlyInMemory = false);

      //! ImageCacheWriteI cache files
      /**
         Create (or replace) an existing cache file. \a size must be the number
         of bytes you will write. The file will not appear in the cache until
         the close() method is done.
       */
      ImageCacheWriteI* create(const DD::Image::Hash&);

      // Let calling code read/write files themselves:
      bool has_file(const DD::Image::Hash&);
      std::string filename(const DD::Image::Hash&);
      void add_file(const DD::Image::Hash&, off_t size);

      //! Return true if the file for the given hash is in the in-memory ImageCache cache
      bool hasInMemory(const DD::Image::Hash& hash);

      //! Get the size of the file on disk, or 0 if absent
      size_t fileSize(const DD::Image::Hash& hash);

      //! Set the maximum number of bytes for the in-memory ImageCache cache
      static void setMaxMemoryCache(size_t usage);

      //! Get the maximum size (in bytes) for the in-memory ImageCache cache
      static size_t getMaxMemoryCache();

      //! Clear the in-memory cache of ImageCache files
      static void clearMemoryCache();

      static void waitEmpty();
    };
  }
}
#endif

/*
** Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
*/
