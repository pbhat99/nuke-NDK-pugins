/* Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved. */

/*! \file DDWindows.h
   Correctly include windows.h with necessary macros defined so that
   the set of symbols we need get declared. This header file purposly
   does nothing on non-Windows platforms.

   If you get strange compilation errors on Windows only the easiest fix
   is to insert the inclusion of this file at the very start of your
   source file. The likely problem is that some other header file is
   including windows.h.

   Please do not include this file unless windows.h is absolutely
   necessary. It pollutes the namespace with lots of unwanted symbols
   and makes portability of the resulting source very difficult.
 */

#ifndef DDImage_DDWindows_H
#define DDImage_DDWindows_H

#ifdef _WIN32
#include <windows.h>
#endif

#endif

/* Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved. */
