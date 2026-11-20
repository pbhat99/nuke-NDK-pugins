// FileIop.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef FILEIOP_H
#define FILEIOP_H

#include "DDImage/Iop.h"
#include "DDImage/Knob.h"
#include "DDImage/FileOp.h"

#define FILEIOP_CSTRING_SIZE 4096

namespace DD
{
  namespace Image
  {

    typedef void (*callback_change_knobs)(void* v, Knob_Callback f);

    /*
     * This class is a base class for Read and Write.  It allows file-format-specific
     * knobs to be substituted in to a particular place more easily
     */
    class DDImage_API FileIop : public Iop, public FileOp
    {

    public:

      /** constructor */
      FileIop(Node * node) : Iop(node) {
        setPopUpScheme(ePU_Once);
      }

      /** destructor */
      ~FileIop() override { }

      /** attempts to rename a file from oldFileName to newFileName **/
      static bool renameFile( const char* oldFileName, const char* newFileName );

      /** attempts to remove the file with fileName **/
      static bool removeFile( const char* fileName );

      /** attempts to open the file with fileName **/
      static FILE* openFile( const char* fileName, const char* mode );

    };
  }
}

#endif
