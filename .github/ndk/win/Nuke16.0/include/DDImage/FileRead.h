// Copyright (c) 2010 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_FileRead_H
#define DDImage_FileRead_H

#include <set>
#include <string>

#include "DDImage/DDImage_API.h"
#include "DDImage/Thread.h"

namespace DD
{
  namespace Image
  {

    /** Interface that is implemented by objects that retain an open
        file handle to some file, so that Execute can make sure that
        all open handles on a file are closed before it is written to.
        Read and ReadGeo inherit from this.
    */
    class DDImage_API FileRead
    {
      ///! all existing _fileRead objects
      static Lock _lock;
      static std::set<FileRead*> _fileReads;

    protected:

      ///! should be overriden to close the file
      virtual void close() = 0;

      FileRead();
      virtual ~FileRead();

    public:

      ///! should be overriden to return the filename currently in use.
      ///! can return NULL if not present
      virtual const char* getFilename() const = 0;

      /// call close() on all FileRead objects whose filename is
      /// /filename/
      static void closeFile(const std::string& filename);
    };
 
 }
}

#endif
