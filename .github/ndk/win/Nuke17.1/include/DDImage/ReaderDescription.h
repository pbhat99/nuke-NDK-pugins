// Copyright (c) 2011 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_ReaderDescription_h
#define DDImage_ReaderDescription_h

#include "DDImage/Description.h"

namespace DD
{
  namespace Image
  {
    //! Base class for Reader plugin descriptions.
    class DDImage_API ReaderDescription : public DD::Image::Description
    {
    public:
      /*! Null separated list (as in "sgi\0rgb\0") of identifiers for this
        file type.
        
        If "xyz" is in this list then a filename of "*.xyz" or "xyz:*" are
        considered to be of this type.
        
        If test is not null this is only considered a hint. A filename of
        "*.xyz" is tested against this first, but all other types are
        tried if it fails. This allows a mis-named file to be read.
      */
      const char* names;
      
      /*! User-friendly version of the name. */
      const char* label;
      
      ReaderDescription(const char* n, const char* l) : names(n), label(l) { }
      
      static const Description* find(const DescriptionList& list, const char* name, const char* pluginSuffix, const char*& triedPlugin);
    };
  }
}

#endif
