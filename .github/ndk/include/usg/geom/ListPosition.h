// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef USG_GEOM_LISTPOSITION_H
#define USG_GEOM_LISTPOSITION_H

namespace usg
{
  //! List position for ordered list operations
  enum class ListPosition {
    PrependFront,
    PrependBack,
    AppendFront,
    AppendBack,

    // The following four values are not defined in UsdListPosition.
    DeleteFront,
    DeleteBack,
    ExplicitFront,
    ExplicitBack
  };

} // namespace usg

#endif
