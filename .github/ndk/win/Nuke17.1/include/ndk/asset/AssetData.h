// Copyright (c) 2024 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef NDK_ASSETDATA_H
#define NDK_ASSETDATA_H

#include "ndk/api.h"

#include <memory>

namespace openassetio { inline namespace v1 { namespace trait { class TraitsData; }}}

namespace ndk { namespace asset {

  //! AssetData provides a wrapper around an OpenAssetIO TraitsData object to allow
  //! it to be used in public plugins without needing to distribute OpenAssetIO source code.
  class NDK_API AssetData
  {
  public:
    AssetData(std::shared_ptr<openassetio::trait::TraitsData> traitsData);
    AssetData(const AssetData& other);
    AssetData(AssetData&& other);
    virtual ~AssetData();

    AssetData& operator=(const AssetData& other);
    AssetData& operator=(const AssetData&& other);

    bool isValid() const;
    
    std::shared_ptr<openassetio::trait::TraitsData> traitsData() const;

  private:
    std::shared_ptr<openassetio::trait::TraitsData> _traitsData;
  };

}}

#endif
