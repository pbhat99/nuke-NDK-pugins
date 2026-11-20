//------------------------------------------------------------------------------
// Application: Application flags for access in Nuke and plugins.
//
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
//------------------------------------------------------------------------------

#ifndef DDIMAGE_APPLICATION_H
#define DDIMAGE_APPLICATION_H

#include "DDImage_API.h"

namespace DD
{
  namespace Image
  {

    //! Contains information about the application that is using DDImage
    namespace Application
    {

      DDImage_API extern bool gui;      //!< True when a GUI is active
                                        //!< Deprecated.  Please use IsGUIActive() instead.

      DDImage_API bool IsGUIActive();                   //!< Returns true when a GUI is active, or false in terminal mode
      DDImage_API bool UsingGUILicense();               //!< True when in GUI mode, or when in terminal mode but using a GUI license (Nuke option -i)
      DDImage_API bool IsNukeX();                       //!< Returns true iff Nuke is running as NukeX (different licence, bundled plugins, etc)
      DDImage_API bool IsNukeAssist();                  //!< Returns true iff Nuke is running as Nuke Assist (different licence, bundled plugins, etc)
      DDImage_API bool IsNukePLE();                     //!< Returns true iff Nuke is running as PLE (different licence, bundled plugins, etc)
      DDImage_API bool IsNukeIndie();                   //!< Returns true iff Nuke is running as Indie (different licence, bundled plugins, etc)
      DDImage_API const char* GetProductName();         //!< Returns the title of the application - e. g. "Nuke", "NukeX"
      DDImage_API const char* GetStandardLicenseName(); //!< Returns the name of the standard (non-NukeX) license required for the product
                                                        //!< - e. g. "nuke", "nukebeta"
      DDImage_API const char* GetLicenseName();         //!< Returns the name of the license (NukeX or otherwise) required for the product
                                                        //!< - e. g. "nuke", "nukex", "nukebeta", "nukexbeta".

      //! Property set functions.  These properties are copies; modifying them will not affect the operation of Nuke.
      //\{
      DDImage_API void SetGUIActive(bool lActive);
      DDImage_API void SetUsingGUILicense(bool lActive);
      DDImage_API void SetNukeX(bool lActive);
      DDImage_API void SetNukeAssist(bool lActive);
      DDImage_API void SetNukePLE();
      DDImage_API void SetNukeIndie();
      DDImage_API void SetProductName(const char* lpProductName);
      DDImage_API void SetStandardLicenseName(const char* lpLicenseName);
      DDImage_API void SetLicenseName(const char* lpLicenseName);

      //\}
    }
  }
}

#endif  // DDIMAGE_APPLICATION_H
