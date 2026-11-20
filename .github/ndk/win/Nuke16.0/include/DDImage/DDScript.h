// Copyright (c) 2011 The Foundry Visionmongers Ltd.  All Rights Reserved.

/*! \file DDScript.h

   This header file only contains functions that can be used
   to run Python code.
 */

#ifndef DDScript_h
#define DDScript_h

#include "DDImage_API.h"

namespace DD
{
  namespace Image
  {
    extern DDImage_API bool (* scriptExec)(const char*, bool);
    extern DDImage_API bool (* scriptEval)(const char*);
    extern DDImage_API bool (* scriptSingle)(const char*);

    extern DDImage_API void (* showScriptErrorPopup)();
  }
}

#endif // DDScript_h
