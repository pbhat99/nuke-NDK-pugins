
// MemRegistrant.h
// Copyright (c) 2019 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_MemRegistrant_H
#define DDImage_MemRegistrant_H

#include "Memory.h"
#include "MemoryHolder.h"

#include <type_traits>

namespace DD
{
  namespace Image
  {

    template<class MemoryHolderT>
    class MemHolderFactory;

    // This class is created automatically and should not be instantiated directly
    template<class MemoryHolderT>
    class MemRegistrant : public MemoryHolderT // Note that this is generated locally and should not be exported
    {
      static_assert(std::is_base_of<MemoryHolder, MemoryHolderT>::value,
        "MemoryHolderT type does not inherit from MemoryHolder");

    private:
      MemRegistrant(const MemRegistrant&)            = delete;
      MemRegistrant& operator=(const MemRegistrant&) = delete;

      template<class...Args>
      MemRegistrant(Args&&... args)
      : MemoryHolderT(std::forward<Args>(args)...)
      {
        Memory::register_user(this);
      }
      ~MemRegistrant() override
      {
        Memory::unregister_user(this);
      }

      // Allow the respective smart pointer to create registrants
      friend class MemHolderFactory<MemoryHolderT>;
    };

  } // namespace Image
} // namespace DD

#endif // DDImage_MemoryRegistrant_H

