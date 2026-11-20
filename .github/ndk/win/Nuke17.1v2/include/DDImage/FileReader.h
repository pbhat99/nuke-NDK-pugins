// FileReader.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_FileReader_h
#define DDImage_FileReader_h

#include "DDImage/Reader.h"
#include "DDImage/Thread.h"
#include "DDImage/Read.h"

namespace DD
{
  namespace Image
  {

    /*! \relates FileReader
       This type may need to be replaced on 64-bit file systems. */
    typedef unsigned long FILE_OFFSET;

    /** \class DD::Image::FileReader
        Reader subclass to read Unix files.
        This Reader subclass will read Unix files as blocks of memory and
        tries to be somewhat multi-threaded and have as little overhead
        as possible.  This is used by most of the file formats.
     */
    class DDImage_API FileReader : public Reader
    {
      unsigned char* buffer;
      FILE_OFFSET mask;
      FILE_OFFSET buffer_size;
      FILE_OFFSET locked_start, locked_end;
      FILE_OFFSET file_offset; // where file pointer was left last

      /*! Internal read function that assumes the lock is held. Calls Iop::error
          if anything goes wrong reading the min_length of bytes. The buffer is
          padded to the given length with zero if fewer than length bytes are read.
       */
      int _read(void*, FILE_OFFSET offset, int l, int m);
      int fd;
      SignalLock lock_;
      unsigned readers; // number of concurrent read_lock()s
      bool waiting;

    public:

      /*! The constructor takes the block of memory already read by Read
          and preserves it. */
      FileReader(Read *, int fd,
                 const unsigned char* buffer, FILE_OFFSET buflength);

      /*! The destructor closes the file. */
      ~FileReader() override;

      /*! lock() makes byte(n) work for any value between \a offset and
          \a offset+length. All bytes between the last one in the file
          and \a length are set to zero. If less than \a min_length bytes
          are available it produces an error.  You must call unlock exactly
          once to free the block (and cannot call lock() again until then).
       */
      int lock(FILE_OFFSET offset, int min_length, int length);
 
      /*! \fn int FileReader::lock(FILE_OFFSET offset, int l)
         Same as lock(offset,l,l), the minimum length needed to produce an error
         is equal to the maximum length.
       */
      int lock(FILE_OFFSET offset, int l) { return lock(offset, l, l); }

      /*! \fn int FileReader::lock(FILE_OFFSET offset, unsigned l)
         Same as lock(offset, int(l)) to avoid type warnings for code that uses
         unsigned values as offsets into the file.
       */
      int lock(FILE_OFFSET offset, unsigned l) { return lock(offset, int(l), int(l)); }

      /*! \fn const unsigned char& FileReader::byte(FILE_OFFSET n) const;

          After calling lock() you can access any byte in the locked range
          by calling this fast inline function. Attempts to access bytes
          outside the currently lock()'ed range will return garbage.

          To get 16-bit unsigned quantities, use msb16(at(n)) or
          lsb16(at(n)). To get 32-bit unsigned quantities use msb32(at(n))
          or lsb32(at(n)).
       */
      const unsigned char& byte(FILE_OFFSET n) const { return buffer[n & mask]; }

      /*! \fn const unsigned char* FileReader::at(FILE_OFFSET n) const;

          Return a pointer to an object in the locked region. This will work
          as long as you know the object does not cross a "page boundary",
          in the current version these pages are 32K in size.
       */
      const unsigned char* at(FILE_OFFSET n) const { return buffer + (n & mask); }

      /*! Call this when you are done using byte() to call a locked region
          of the file. This will allow this or other threads to lock a
          different region.
       */
      void unlock();

      /*! Read directly from the file into some memory of your own.  If
          less than \a length bytes are available the block is padded with zero.
          The number of bytes read (0 if there is an error) is returned.
          If less than \a min_length are available it produces an error.
       */
      int read(void* p, FILE_OFFSET offset, int min_, int max_);

      /*! \fn int FileReader::read(void* p, FILE_OFFSET offset, int l)
         Same as read(p,offset,l,l), the minimum length needed to produce an error
         is equal to the maximum length.
       */
      int read(void* p, FILE_OFFSET offset, int l) { return read(p, offset, l, l); }

      /*! \fn int FileReader::read(void* p, FILE_OFFSET offset, unsigned l)
         Same as read(p,offset,int(l)) to avoid type warnings for code that uses
         unsigned values as offsets into the file.
       */
      int read(void* p, FILE_OFFSET offset, unsigned l) { return read(p, offset, int(l), int(l)); }

      /*! \fn bool readAllLinesRequested() const
         Whether reading all lines (as opposed to reading individual scanlines) has been
         requested.
         For performance reasons, it is sometimes preferable to read from the input file
         in larger chunks. When this function returns true, the user has indicated that 
         they would prefer to read all of the requested lines from the file in one go.
         It's then up to the individual FileReader whether it chooses to obey this request
         or not.
       */
      bool readAllLinesRequested() const;

    };

  }
}

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
