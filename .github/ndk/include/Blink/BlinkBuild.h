// Copyright (c) 2013 The Foundry Visionmongers Ltd.  All Rights Reserved.

/** \file BlinkBuild.h
 * \brief Definitions required for building with the Blink API.
*/

#ifndef Blink_Library_BlinkBuild_h
#define Blink_Library_BlinkBuild_h
#ifndef BLINK_SHARED_API
  #ifdef _WIN32
    #if defined(BLINK_EXPORTS)
      #define BLINK_SHARED_API __declspec(dllexport)
    #else
      #define BLINK_SHARED_API __declspec(dllimport)
    #endif
  #else
    #if defined(BLINK_EXPORTS)
      #define BLINK_SHARED_API __attribute__((visibility("default")))
    #else
      #define BLINK_SHARED_API
    #endif
  #endif
#endif
#endif
