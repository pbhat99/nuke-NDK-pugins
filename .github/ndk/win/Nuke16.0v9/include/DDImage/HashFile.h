/*
** HashFile.h: Includes functions to generate and check a hash for a given file.
**
** Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
*/

#ifndef D_HASHFILE_H
#define D_HASHFILE_H

#include "DDImage/Hash.h"

//------------------------------------------------------------------------------

namespace DD
{

namespace Image
{

//! Includes functions to generate and check a hash for a given file.
namespace HashFile
{

  //! Possible error return values
  enum ErrorValue {
    kErr_OK = 0,          //!< No problems
    kErr_FileNotFound,    //!< File couldn't be opened - not found
    kErr_OpenFailed,      //!< File couldn't be opened - any other error (including sharing)
    kErr_ReadFailed,      //!< Couldn't read all expected data
    kErr_HashNotFound,    //!< File doesn't contain a hash value (and we expected one)
    kErr_HashFound,       //!< File contains a hash value (and we didn't expect one)
    kErr_HashMismatch,    //!< Hash found, but doesn't match the expected value
    kErr_InvalidParam,    //!< Parameter invalid - e. g. null pointer
  };

  //! Writes a hash value for the data in the given file to the pointer
  DDImage_API ErrorValue generateHashForFile(const char* lpFileName, Hash* lpHashOut);

  //! Writes a hash value for the specified tag string and data in the given file to the pointer
  DDImage_API ErrorValue generateTaggedHashForFile(const char* lpFileName, const char* lpTag, Hash* lpHashOut);

  //! Writes a hash value for the specified tag string and node name to the pointer
  DDImage_API ErrorValue generateTaggedHashForNodeName(const char* lpNodeName, const char* lpTag, Hash* lpHashOut);

  //! Checks that the hash value for the data in the file matches the given value.
  //! This expects that the file does not have a hash embedded
  DDImage_API ErrorValue checkSpecifiedHashWithFile(const char* lpFileName, Hash lExpectedHash);

  //! Checks that the hash value stored in the given file is correct
  DDImage_API ErrorValue checkEmbeddedHashInFile(const char* lpFileName);

  //! Embeds the hash in the file, appending to it directly.
  //! It returns an error if a hash is already present
  DDImage_API ErrorValue embedHashInFile(const char* lpFileName, Hash lHash);

  //! Embeds the hash in the file, appending to it directly.
  //! This version generates the hash from the file first
  DDImage_API ErrorValue embedHashInFile(const char* lpFileName);

  //! Returns an appropriate error string for the given error value
  DDImage_API const char* getErrorMessage(ErrorValue lErrorValue);

  //! @name Debug
  //! These functions are only valid in debug mode, and indicate the last hashes used in various contexts.
  //! In release mode they will just return uninitialised hashes.
  //@{
  Hash lastEmbeddedHash();
  Hash lastDetectedHash();
  Hash lastGeneratedHash();
  //@}

} // namespace HashFile

} // namespace Image

} // namespace DD

//------------------------------------------------------------------------------

#endif  // D_HASHFILE_H
