// FileSet.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef FileSet_h
#define FileSet_h

#include "DDImage/Thread.h"
#include "DDImage_API.h"

#include <vector>

namespace DD
{
  namespace Image
  {

    /*! \class FileSet

       Rather simple caching mechanism to see if a file you are interested
       in exists. Each time you look for a file, the entire directory that
       the file is in is added to the set. This makes checking for another
       file in the same directory much faster.

       Warning: strict equality (like Unix) of the filenames is required.
     */
    class DDImage_API FileSet
    {
      struct Dir
      {
        const char* name;
        int namelen;
        bool loaded;
        std::vector <const char*> files;
      };
      std::vector <Dir> directories;  //< information about directories and teh files they contain
      DD::Image::Lock _lock;          //< enable thread safety for the above structure as Reads may flush the directory info if they can't find a plugin for a file-type.

      void load(Dir&);
    public:

      /*! Return true if the file exists. */
      bool find(const char* pathname);

      /*! Return finddir of the directory that starts the name. Everything
         up to the last slash is used. */
      int finddirfor(const char* pathname);

      /*!
         Return an index number to identify the given directory. This can
         be used to look up names in that directory without having to
         paste them together in a buffer.

         \a length is the offset of the '/' or one character after it that
         ends the directory name, this allows you to request a directory
         from a full path name without having to copy and null-terminate it.
         If \a length is zero then strlen(name) is used.

         If this is a new directory, the actual scan of the disk is not done
         until a find() is done for a file that matches this directory.
       */
      int finddir(const char* dirname, int length = 0);

      /*!
         Return true if the file with the given name exists in the directory
         identified by \a dirnumber, this is a value returned by finddir().
       */
      bool find(int dirnumber, const char* filename);

      /*!
         Return a vector of all the filenames (except "." and "..") in the
         given directory. Non-existent and empty directories both return an
         empty list. This is very fast the second time the directory is
         asked for.
       */
      const std::vector <const char*>& list(int dirnumber);
      const std::vector <const char*>& list(const char* dirname)
      {
        return list(finddir(dirname));
      }

      /*! Cause it to reload all directories in the FileSet. */
      void reload();
      FileSet() {};
      ~FileSet();
    };

  }
}

#endif
