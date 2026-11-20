// Copyright (c) 2011 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_AccountedVector_H
#define DDImage_AccountedVector_H

#include "DDImage/STLAllocator.h"

namespace DD {
  namespace Image {

    template<class Element>
    class AccountedVector : public STL<Element>::vector
    {
    public:
      using STL<Element>::vector::size;
      using STL<Element>::vector::empty;
      using STL<Element>::vector::operator[];

      void zero()
      {
        memset(data(), 0, size() * sizeof(Element));
      }
      
      Element* data()
      {
        return &operator[](0);
      }

      operator bool() const
      {
        return !empty();
      }
      
      static size_t elementSize()
      {
        return sizeof(Element);
      }
    };
  }
}

#endif
