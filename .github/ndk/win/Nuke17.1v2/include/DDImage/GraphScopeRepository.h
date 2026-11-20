// Copyright (c) 2024 The Foundry Visionmongers Ltd. All Rights Reserved.
#ifndef DDIMAGE_GRAPHSCOPEREPOSITORY_H
#define DDIMAGE_GRAPHSCOPEREPOSITORY_H

#include "DDImage/DDImage_API.h"
#include "DDImage/GsvSet.h"

#include <memory>

namespace DD {
  namespace Image {
    //! GraphScopeRepository constructs and store GraphScope objects.
    class DDImage_API GraphScopeRepository : public std::enable_shared_from_this<GraphScopeRepository> {
      struct Impl;
    public:
      ~GraphScopeRepository();

      //! Creates a new GraphScopeRepository. The repository initially contains an empty
      //! GraphScope, that will be the parent of all scopes created.
      static GraphScopeRepositoryPtr create();

      //! Returns a GraphScope with \a gsvSets and \a parent as its parent GraphScope.
      //! If an existing GraphScope matches the arguments it will be returned
      //! instead of creating a new one.
      GraphScopePtr build(const GsvSets& gsvSets, const GraphScopePtr parent = nullptr);

      //! Retrieves a GraphScope from the repository. If no GraphScope is found, an
      //! empty shared pointer is returned.
      GraphScopePtr get(const GraphScopeId& scopeId) const;

      //! Returns the number of GraphScopes in the repository.
      size_t size() const;

      //! Returns a shared pointer to the GraphScopeRepository.
      GraphScopeRepositoryPtr ptr();

    private:
      GraphScopeRepository();
      std::unique_ptr<Impl> _pImpl;
    };
  }
}
#endif
