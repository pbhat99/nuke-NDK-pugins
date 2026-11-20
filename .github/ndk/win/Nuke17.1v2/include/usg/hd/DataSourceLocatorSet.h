// Copyright (c) 2024 The Foundry Visionmongers Ltd. All Rights Reserved.

#pragma once

#include <memory>

#include "usg/api.h"
#include "usg/hd/DataSourceLocator.h"

namespace usg::hd {
  class USG_API DataSourceLocatorSet
  {
  public:
    class Handle;
    class Impl;

    DataSourceLocatorSet();
    DataSourceLocatorSet(const DataSourceLocator& locator);
    // Copy constructor
    DataSourceLocatorSet(const DataSourceLocatorSet&);

    ~DataSourceLocatorSet();

    void append(const DataSourceLocator& locator);

    DataSourceLocatorSet::Handle* getHdDataSourceLocatorSet(int usd_version);

    //------------------------------------------------------
    // For internal use only....
    //------------------------------------------------------
    DataSourceLocatorSet(const DataSourceLocatorSet::Impl& impl);
    //! Returns the handle to the wrapped object.
    DataSourceLocatorSet::Impl* impl() { return _pImpl.get(); }
    const DataSourceLocatorSet::Impl* impl() const { return _pImpl.get(); }

  protected:
    std::unique_ptr<DataSourceLocatorSet::Impl, void (*)(DataSourceLocatorSet::Impl*)> _pImpl;
  };

}
