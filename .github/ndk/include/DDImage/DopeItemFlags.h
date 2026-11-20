/*
** Copyright (c) 2013 The Foundry Visionmongers Ltd.  All Rights Reserved.
*/

#ifndef DOPE_ITEM_FLAGS_H
#define DOPE_ITEM_FLAGS_H

#include "DDImage/DDImage_API.h"

namespace DD
{
  namespace Image
  {
    namespace DopeItemFlags
    {
      typedef U64 Mask;

      static const Mask NONE                = 0x0000000000000000;
      static const Mask DOUBLE_HEIGHT       = 0x0000000000000001; //!< Show as a double-height DopeItem
      static const Mask SORT_TO_TOP         = 0x0000000000000002; //!< Sort this DopeItem above others that don't have this set
      static const Mask SHOWS_CLIPS         = 0x0000000000000004; //!< Shows clip range bars rather than keys
      static const Mask DONT_INHERIT_KEYS   = 0x0000000000000008; //!< Dope item does not inherit the keys of its children
      static const Mask KEYS_NOT_EDITABLE   = 0x0000000000000010; //!< Keys are for display only and cannot be dragged
      static const Mask CLIP_SLIPPABLE      = 0x0000000000000020; //!< Shows clip range bars that can be 'slipped' using the lower portion
      static const Mask CLIP_TRIMMABLE      = 0x0000000000000040; //!< Shows clip range bars that can be trimmed at their ends
    }
  }
}

#endif
