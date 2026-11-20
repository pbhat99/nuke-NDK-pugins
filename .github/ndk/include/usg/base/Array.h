// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef USG_BASE_ARRAY_H
#define USG_BASE_ARRAY_H

#include <cstddef>
#include <vector>

namespace usg
{

  //! A class for arrays in attribute data. This is currently aliased to std::vector but you should not rely on this.
  //! The implementation may change in the future to provide more efficient bridging of arrays to USD.
  //! The purpose of this template is to discourage use of std::vector in the code and force compiler
  //! or linker errors if this happens.
  template <typename T>
  class Array : public std::vector<T> {
  public:
    Array() {}
    explicit Array(std::size_t size) : std::vector<T>(size) {}
    Array(std::size_t size, const T& v) : std::vector<T>(size, v) {}
    Array(const std::initializer_list<T>& list) : std::vector<T>(list) {}
    template<typename InputIt>
    Array(InputIt first, InputIt last) : std::vector<T>(first, last) {}
    
    // TODO(rk): Remove this size() after updating pybind11 to 2.12 or higher.
    // This overriding size() is a workaround for the following issue resolved the version.
    // https://github.com/pybind/pybind11/pull/4952
    typename std::vector<T>::size_type size() const { return std::vector<T>::size(); }
  };

}

#endif
