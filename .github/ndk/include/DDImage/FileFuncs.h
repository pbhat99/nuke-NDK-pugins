//------------------------------------------------------------------------------
// FileFuncs: Miscellaneous file functions in DDImage.
//
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
//------------------------------------------------------------------------------

#ifndef DDIMAGE_FILEFUNCS_H
#define DDIMAGE_FILEFUNCS_H

//------------------------------------------------------------------------------

#include "DDImage/DDImage_API.h"
#include <string>

//------------------------------------------------------------------------------

namespace DD
{
  namespace Image
  {
    class Op;
    
    //------------------------------------------------------------------------------
    
    //! Renames a temporary file to its final name.  On failure, it sets an appropriate error message on the supplied op.
    //! \return True iff the rename succeeded.  If false, an error message will be set.
    DDImage_API bool RenameTempToFinal(const char* lpTempName, const char* lpFinalName, Op* lpOp);

    //! Apply path remapping configured in Nuke's preferences to a path, and return the result.
    DDImage_API std::string RemapPath(const std::string& path);
  } // Image
} // DD

//------------------------------------------------------------------------------

#endif  // DDIMAGE_FILEFUNCS_H


