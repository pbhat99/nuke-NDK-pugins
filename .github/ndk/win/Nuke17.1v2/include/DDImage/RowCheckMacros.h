// RowCheckMacros.h
// Copyright (c) 2015 The Foundry Visionmongers Ltd.  All Rights Reserved.
//
// For Foundry internal use only.
//

#ifndef DDImage_RowCheckMacros_h
#define DDImage_RowCheckMacros_h

#if defined( FN_DEBUG )
# define DD_ROW_CHECK_DEFAULT 1
#else
# define DD_ROW_CHECK_DEFAULT 0
#endif // defined( FN_DEBUG )

#if !defined( DD_ROW_CHECK )
#  define DD_ROW_CHECK DD_ROW_CHECK_DEFAULT
#endif // !defined( DD_ROW_CHECK )

// enable the extra/slower memory checks (Foundry internal only)
#if !defined( DD_ROW_CHECK_EXTRA )
#  define DD_ROW_CHECK_EXTRA DD_ROW_CHECK_DEFAULT
#endif // defined( !DD_ROW_CHECK_EXTRA )

#if !defined( DD_ROW_CHECK_LOGGING )
#  define DD_ROW_CHECK_LOGGING 0
#endif // !defined( DD_ROW_CHECK_LOGGING )

#endif // DDImage_RowCheckMacros_h

