// Copyright (c) 2024 The Foundry Visionmongers Ltd. All Rights Reserved.

#pragma once

#include <memory>

#include "usg/api.h"
#include "usg/base/Token.h"

namespace usg::hd {
  class USG_API DataSourceLocator
  {
  public:
    class Handle;
    class Impl;

    DataSourceLocator();
    DataSourceLocator(const Token& t1);
    DataSourceLocator(const Token& t1, const Token& t2);
    DataSourceLocator(const Token& t1, const Token& t2, const Token& t3);
    DataSourceLocator(const Token& t1, const Token& t2, const Token& t3, const Token& t4);
    DataSourceLocator(const Token& t1, const Token& t2, const Token& t3, const Token& t4,
                      const Token& t5);
    DataSourceLocator(const Token& t1, const Token& t2, const Token& t3, const Token& t4,
                      const Token& t5, const Token& t6);
    ~DataSourceLocator();

    DataSourceLocator::Handle* getHdDataSourceLocator(int usd_version);

    //------------------------------------------------------
    // For internal use only....
    //------------------------------------------------------
    //! Returns the handle to the wrapped object.
    DataSourceLocator::Impl* impl() { return _pImpl.get(); }
    const DataSourceLocator::Impl* impl() const { return _pImpl.get(); }

  protected:
    std::unique_ptr<DataSourceLocator::Impl, void (*)(DataSourceLocator::Impl*)> _pImpl;
  };

}
