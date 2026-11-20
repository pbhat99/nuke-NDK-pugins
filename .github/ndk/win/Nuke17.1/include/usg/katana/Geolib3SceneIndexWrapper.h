// Copyright (c) 2024 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef USG_KATANA_KATANASCENEINDEX_H
#define USG_KATANA_KATANASCENEINDEX_H

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "usg/api.h"
#include "usg/base/Token.h"

namespace Foundry::Katana::HdSceneInterface {
  class Geolib3ScenePropagator;
}

namespace usg::katana {

  //! class Geolib3SceneIndexWrapper
  //! Wrapper class for FnHdBridge3::Geolib3SceneIndexWrapper
  class USG_API Geolib3SceneIndexWrapper
  {
  public:
    class Impl; //!< Wrapper abstraction for implementation type

    //! Constructs a Geolib3SceneIndexWrapper.
    Geolib3SceneIndexWrapper();
    Geolib3SceneIndexWrapper(const Geolib3SceneIndexWrapper&) = delete;
    Geolib3SceneIndexWrapper& operator=(const Geolib3SceneIndexWrapper&) = delete;

    virtual ~Geolib3SceneIndexWrapper();

    //! Returns true if Geolib3SceneIndexWrapper::Impl is valid (has a valid object binding.)
    bool isValid() const;
    explicit operator bool() const { return this->isValid(); }

    // Shader and render context configuration
    void setShaderSourceTypes(const usg::TokenArray& sourceTypes);
    void setRenderContexts(const usg::TokenArray& renderContexts);
    void setSupportedSPrims(const usg::TokenArray& supportedSPrims);

    // Location change notifications
    virtual void
    sendLocationsAdded(const std::vector<std::pair<std::string, std::string>>& locations);
    virtual void sendLocationsDirtied(
      const std::unordered_map<std::string, std::vector<std::string>>& dirtyLocationAttrMap);
    virtual void sendLocationsRemoved(const std::vector<std::string>& locations);

    //! Set or clear the Geolib3ScenePropagator reference held by the underlying scene index
    virtual void
    setPropagator(const Foundry::Katana::HdSceneInterface::Geolib3ScenePropagator* scene);

    // Initialize FnHdBridge3
    static void Initialize(void* host);

    //------------------------------------------------------
    // For internal use only....
    //------------------------------------------------------
    Geolib3SceneIndexWrapper::Impl* impl() { return _pImpl.get(); }
    const Geolib3SceneIndexWrapper::Impl* impl() const { return _pImpl.get(); }

  protected:
    std::unique_ptr<Geolib3SceneIndexWrapper::Impl, void (*)(Geolib3SceneIndexWrapper::Impl*)>
      _pImpl;
  };

} // namespace usg::katana

#endif // USG_KATANA_KATANASCENEINDEX_H
