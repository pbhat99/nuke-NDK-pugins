// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

/*! \file ARRAY.h

   Defines a portable replacement for run-time sized local arrays and
   for alloca().
 */

#ifndef ARRAY_h
#define ARRAY_h

#ifndef _COMPILER_VERSION
  #define _COMPILER_VERSION 0
#endif

#if defined(__sgi) && _COMPILER_VERSION < 730 || (defined(__GNUC__) && !defined(__INTEL_COMPILER)) || defined(__APPLE__) || defined(DOXYGEN) // probably others
// I had to put the !__INTEL_COMPILER into the line above because somewhere
// somehow, __GNUC__ gets defined, even when using icc instead of gcc
// The answer to the above is: for the time being we configure icc to pull in
// gcc's config (on Linux and Mac - Linux explicitly and Mac implicitly). What
// we should do here is use <Build/fnBuild.h> and the associated FN_ macros
// instead. /FH

/*!
   Makes a run-time sized array for local storage.

   In gcc and other compilers that support it, this turns into
   "type name[n]".

   In other systems this turns into calls to alloca().

   In others still (Windows) it turns into
   C++ templates that rely on the destructor to get the same result,
   though much less efficiently.
 */
  #define ARRAY(_t, _n, _m) _t _n [_m]

// the _WIN32 alloca does not seem to work in multithreaded?
//#elif defined(_WIN32)
//#include <malloc.h>
//#define ARRAY(type, name, n) type*const name = (type*)_alloca(n*sizeof(type))

#elif defined(__DECCXX) //|| defined(__sgi)
  #include <alloca.h>
  #define ARRAY(type, name, n) type * const name = (type*)alloca(n * sizeof(type))

#else
// C++ hack that kind of works:

template <class T>
class Alloca
{
  T* p;
public:
  Alloca(int n) { p = new T[n]; }
  operator T*() { return p; }
  ~Alloca() { delete [] p; }
};
  #define ARRAY(type, name, n) Alloca < type > name(n)

#endif

#endif // ARRAY_h

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

