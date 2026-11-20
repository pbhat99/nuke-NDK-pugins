// Copyright (c) 2013 The Foundry Visionmongers Ltd.  All Rights Reserved.

/*! \file StringUtils.h

  Utilities for evaluating strings containing expressions and for manipulating/querying the clipboard
 */

#ifndef StringUtils_h
#define StringUtils_h

#include "DDImage_API.h"
#include "DDImage/OutputContext.h"
#include <string>

namespace DD
{
  namespace Image
  {
    /*!
       "expand" the string by replacing tcl portions of the string
       with the final text. Returns true if successful, false otherwise.

       This function is not thread-safe.

       "[command]" is replaced with the result of executing that command, and
       "$variable" is replaced with the tcl variable. 

       The flags controls what is replaced. The following substitutions
       are done:

       If the flag eStringExpandFilename is set, a leading "~/" is replaced by the
       value of $env(HOME), and a leading "~foo" is replaced by a guess
       of the user foo's home directory done by replacing the end of
       $env(HOME) (nuke does not bother with the passwd file). And the
       result is passed through filename_fix, if the command is defined.

       If the flag eStringExpandFileNumber is set, a %nd and ### sequences are replaced
       by the frame number from the ScriptContext argument, and %V and %v
       are replaced with the name and initial of the view.

       "\x" (where x is any character including backslash) is replaced by
       "x". However the backslash prevents x from being interpreted in any
       way, so you can get square brackets or %% or other characters into
       the output.
       */

    enum StringExpandFlags
    {
      eStringExpandNoFlags = 0,
      eStringExpandFilename   = 1,
      eStringExpandFileNumber = 2,
    };

    class NodeI;

    extern DDImage_API bool (* evaluateString)(const char* string,  int expandFlags, NodeI* node, const OutputContext& oc, std::string& expandedString);

    /*!
      Return the contents of the clipboard
      */
    extern DDImage_API std::string (* getClipboardContents)();

    /*!
      Copy a string into the clipboard
      */
    extern DDImage_API void (* setClipboardContents)(std::string& text);
     
  }
}

#endif // StringUtils_h
