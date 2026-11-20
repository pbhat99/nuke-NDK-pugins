// Copyright (c) 2024 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef NDK_ASSETINFO_H
#define NDK_ASSETINFO_H

#include "ndk/api.h"

#include <string>
#include <string_view>
#include <set>

#include "ndk/asset/AssetData.h"

namespace ndk { namespace asset {

  using AssetTraits = std::set<std::string>;

  class NDK_API AssetInfo
  {
  public:

    AssetInfo();
    AssetInfo(const AssetInfo& other);
    AssetInfo(AssetInfo&& other);

    AssetInfo(const AssetTraits& traits);
    AssetInfo(const std::string& path, const AssetTraits& traits);
    virtual ~AssetInfo();

    AssetInfo& operator=(const AssetInfo& other);
    AssetInfo& operator=(const AssetInfo&& other);

    bool operator==(const AssetInfo& other) const;

    struct NDK_API HashFunction
    {
      size_t operator()(const AssetInfo& assetInfo) const;
    };

    std::string path() const;
    const AssetTraits& traits() const;
    size_t hash() const;

    bool isAsset() const;
    static bool isAsset(std::string_view path);

    //! Return the trait data indicated by our traits property from the resolved asset.
    //! If the asset needs to be (re)resolved first then this will be done and the relevant
    //! data cached for later use.
    //! Returns a block entity trait data containing the requested traits if they exist on the asset.
    AssetData getResolvedData() const;

    //! Resolve the specified asset path to a set of asset data, as defined by our traits set.
    //! Returns the local file path of the asset if it has one. Any other requested traits will
    //! be retained in the resolved assets cache for later retrieval.
    //! Updates the current path to the new one.
    bool resolveLocation(std::string path, std::string& resolvedPath);

  private:

    void updateHash();

    std::string _path;
    AssetTraits _traits;
    size_t _hash;

    bool _isAsset;
  };

}}

#endif
