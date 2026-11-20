// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

/*! \file DDString.h

   \brief Cross-platform string manipulation functions.

   You should use this file instead of <string.h>, <stdlib.h>, or
   <stdio.h>. This defines replacements for several standard library
   functions to produce the same behavior on all platforms (mostly to
   copy Linux features to Windows). It also defines functions from
   BSD that make string manipulation that does not overflow buffers
   much easier.

 */

#ifndef DDstring_h
#define DDstring_h

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "DDImage_API.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
  #define strcasecmp(s, t)    _stricmp((s), (t))
  #define strncasecmp(s, t, n)    _strnicmp((s), (t), (n))

  #if ( defined(_MSC_VER) && _MSC_VER < 1900 )
    #define snprintf        _snprintf
    #ifndef vsnprintf
      #define vsnprintf        _vsnprintf
    #endif //*#ifndef */vsnprintf
  #endif
#elif defined(__linux) || defined(__FreeBSD__) || defined(__APPLE__)
/* linux has vsnprintf */
#elif _COMPILER_VERSION >= 730
/* newer SGI compilers have vsnprintf */
#else
/* all other machines need our emulation function: */
  #define DDvsnprintf 1

/*! Print at most \a size characters (including a trailing null) into
    the buffer pointed to by \a str. Returns the number of bytes written
    (not including the nul). If the printout is truncated this is supposed
    to return the number of bytes that \e would have been printed, however
    this only works on newer Linux machines, other systems return -1 or zero.

    Linux provides this function. On NT it calls _vsnprintf which is almost
    correct. On Irix and some other systems an emulation function is
    used. This emulation is rather simple and has the following bugs:

    \li Field width & Precision is ignored for %%, \%c, and \%s.

    \li All other formats are printed to an internal buffer that can
    be overflowed by use of large percision or field width values.
    For instance %110f will overflow.

    \li Only handles formats that are both documented in the glibc man page
    for printf and also handled by your system's sprintf().
 */
int vsnprintf(char* str, size_t size, const char* fmt, va_list ap);

/*! Print at most \a size characters (including a trailing null) into
    the buffer pointed to by \a str.  See vsnprintf for bugs. */
int snprintf(char* str, size_t size, const char* fmt, ...);
#endif

#if !defined(__FreeBSD__) && !defined(__APPLE__)

/*! BSD no-buffer-overflow string concatanation function.  Appends \a
    src to buffer \a dst of size \a siz (unlike strncat(), \a siz is
    the full size of \a dst, not space left).  At most \a siz-1
    characters will be copied.  Always NUL terminates (unless \a siz
    == 0).  Returns strlen(initial dst) + strlen(src); if retval >=
    siz, truncation occurred.  */
DDImage_API size_t strlcat(char* dst, const char* src, size_t siz);

/*! BSD no-buffer-overflow string copy function.  Copy \a src to
    buffer \a dst of size \a siz.  At most \a siz-1 characters will be
    copied.  Always NUL terminates (unless \a siz == 0).  Returns
    strlen(src); if retval >= siz, truncation occurred.  */
DDImage_API size_t strlcpy(char* dst, const char* src, size_t siz);

#endif

/**
   Equivalent to strdup() except the C++ new[] operator is used. A
   block of memory strlen(from)+1 is allocated and the \a from
   string is copied to it. Notice that you must use delete[] to
   destroy the returned value, not free()!

   If NULL is passed, a NULL is returned.

   It is a good idea to use this instead of strdup() so that all
   your memory allocations pass through the C++ new-handler.

   <DDImage/DDString.h> defines newstring() as a macro that calls
   this. This is to avoid duplicate-symbol conflicts with libraries
   that define functions by the same name.
 */
DDImage_API char* DDImage_newstring(const char*);
#undef newstring
#define newstring(x) DDImage_newstring(x)

/*! Replacement for strtod() from stdlib.h

    Skips all whitespace and then attempts to convert the start of the
    next text into a double. \a endptr (if not null) is changed to
    point at the character it stopped at. This is the same as the
    standard strtod() but it also recognizes:

    - Infinity as printed by Linux, Windows, and Irix
    - NaN as printed by Linux, Windows, and Irix
    - Hex constants like 0xabcde
    - "true" (1)
    - "false" (0)

    These changes allow it to read numbers printed to a file from any
    platform, and to read constants that were intended for other data
    types.

    <DDImage/string.h> defines strtod() as a macro that calls
    this, thus replacing the C standard library function.
 */
DDImage_API double DDImage_strtod(const char* p, char** endptr);
#undef strtod
#define strtod(a, b) DDImage_strtod(a, b)

#ifdef __cplusplus
}
#endif

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
