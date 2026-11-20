// MemHolderFactory.h
// Copyright (c) 2019 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_MemHolderFactory_H
#define DDImage_MemHolderFactory_H

#include "MemRegistrant.h"

namespace DD
{
  namespace Image
  {

    template<class MemoryHolderT>
    class MemHolderFactory // Note that this is generated locally and should not be exported
    {
    public:
      template<class...Args>
      static MemoryHolderT* create(Args&&... args) {
        return new MemRegistrant<MemoryHolderT>(std::forward<Args>(args)...);
      }

      MemHolderFactory()                                       = delete;
      MemHolderFactory(const MemHolderFactory& rhs)            = delete;
      MemHolderFactory& operator=(const MemHolderFactory& rhs) = delete;
      ~MemHolderFactory()                                      = delete;
    };

  } // namespace Image
} // namespace DD

#endif

