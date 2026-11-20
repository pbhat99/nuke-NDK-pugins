// FileWriter.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_FileWriter_h
#define DDImage_FileWriter_h

#include "DDImage/Writer.h"
#include <string>

namespace DD
{
  namespace Image
  {

    /*! \relates FileWriter
       This type may need to be replaced on 64-bit file systems. */
    typedef unsigned long FILE_OFFSET;

    /** \class DD::Image::FileWriter
        Writer subclass to write a single Unix file.
        You must subclass this and implement several virtual functions to
        write pieces of a file.  This takes care of opening and closing the
        file.
     */
    class DDImage_API FileWriter : public Writer
    {
    protected:
      void* file; //!< Actually a FILE* but I don't want the <stdio.h> header!
      FILE_OFFSET offset; //!< Current offset into file

      /*! Open the output file.  This file is created with a temporary name
          so that the original file is not replaced until the write is
          successful.  This returns false and calls iop->error() if it
          can't open the file.
       */
      bool open();

      /*! Close the output file.  If there have not been any errors this will
         mv the temporary file to the correct place. */
      bool close();

      /*! Write \a length bytes to the file.  Call iop->error() on any errors
          and returns false.  Returns true on success.
       */
      bool write(const void*, FILE_OFFSET);

      /*! Same as "seek(off) && write(buf, length)", this writes \a length
          bytes to the file at \a offset and leaves the pointer after that. */
      bool write(FILE_OFFSET, const void*, FILE_OFFSET);

      /*! Move the write pointer to \a offset.  This is done efficiently if
          the pointer is already at that point.  Calls iop->error()
          on any errors and returns false.  Returns true on success. */
      bool seek(FILE_OFFSET);

      /*! \fn FILE_OFFSET FileWriter::tell() const
          Returns where the pointer currently is.
       */
      FILE_OFFSET tell() const { return offset; }

      //! Returns a temporary file name for the current output file (e. g. filename.ext.tmp)

      //! Returns a temporary file name for the given actual file name.
      //! e. g. filename.ext -> filename.ext.tmp
      //! Pass in NULL to use the current filename().
      std::string getTempFileName(const char* pActualFileName = nullptr) const;

    public:

      /*! This does not open the file. Instead a subclass should call open()
         as the first statement in execute() and then return if open fails. */
      FileWriter(Write*);

      /*! Destruction closes the temporary file if execute() did not call
          close(). */
      ~FileWriter() override;

      //! Called when processing is finished for the current file
      void finish() override;
    };

  }
}

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
