// Copyright (c) 2025 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef NDK_GRAPH_GRAPHSCOPE_H
#define NDK_GRAPH_GRAPHSCOPE_H

#include "ndk/api.h"
#include "ndk/graph/GsvTypes.h"

#include "fdk/math/Vec2.h"
#include "fdk/base/Hash.h"
#include "fdk/base/StringTypes.h"

#include <memory>
#include <string>
#include <optional>
#include <ostream>

namespace ndk {

  //! Print out the contents of a GsvSets pack.
  NDK_API void PrintGsvSets(const GsvSets& gsvSets, std::ostream& os, const std::string& padding);

  //----------------------------------------------------------------------

  //! A GraphScope is an immutable object that holds GsvSets. A Gsv is a Graph
  //! Scope Variable that is defined by a Node and set to a GraphScope during
  //! the Op tree generation.
  //! GSVs can be used in expressions, for example, a GSV named 'dir' used in
  //! a Read node file path like "/Volume/%dir" and can be resolved to
  //! different values depending on the GraphScope used in the context.
  //! A GraphScope can have a parent scope, which allows for the GraphScope to be
  //! extended and overriden.
  //! A GraphScope can be directly constructed via a GraphScopeRepository, or
  //! indirectly via an existing GraphScope that will call the GraphScopeRepository
  //! under the hood.
  //! The access to a GraphScope should be done via a GraphScopePtr returned
  //! either by a GraphScopeRepository::build() or GraphScope::build() function.
  //! If a GraphScopePtr is empty, the scope should be considered nonexisting.
  class NDK_API GraphScope : public std::enable_shared_from_this<GraphScope> {
  public:
    GraphScope() = delete;
    ~GraphScope();

    bool isEmpty() const;

    //! Get the flattened set of all vars with expanded names. Vars in custom GvsSets
    //! have names expanded to '<setname>.<varname>'.
    const GsvFlattenedSet& allVars() const;

    //! Get the list of all vars in this scope including the inherited ones.
    fdk::StringSet varNames() const;

    //! Returns true if the named var is in the scope, either locally or inherited.
    bool hasVar(const std::string& varname) const;

    //! Returns true if the named var is a local var in the scope.
    bool hasLocalVar(const std::string& varname) const;

    //! Returns a GSV value from the named var (<setname>.<varname>).
    std::optional<std::string> valueOf(const std::string& varname) const;

    //! Returns a GSV value from a GsvSet in this scope.
    //! \a set is the name of the GsvSet to be searched for \a key.
    //! \a key is the GSV name to be searched.
    //! Returns an optional string with the GSV value, if found, or
    //! std::nullopt otherwise.
    std::optional<std::string> valueOf(const std::string& set, const std::string& key) const;

    //! Returns the unique ID for this scope which will change if any of its
    //! local or inherited vars change in name or value.
    const GraphScopeId& id() const;

    //! Returns the hash of all of its data, including parents.
    const fdk::Hash& hash() const;

    //! Returns the hash of the local vars in this scope.
    const fdk::Hash& localHash() const;

    //! Returns the parent of the GraphScope.
    GraphScopePtr parent() const;

    //! Returns the local override GsvSets held by the GraphScope.
    GsvSets localGsvSets() const;

    //! Returns a GsvSet with the given GsvSets and parented by the callee GsvSet.
    //! \a data is the GsvSets to be held by the GraphScope.
    const GraphScopePtr build(const GsvSets& gsvSets);

    //! Returns all the GSV data used to create this scope in a flattened fashion.
    //! The key in this map is in the form <gsv-set-name>.<variable-name>.
    //! The value for each key is a vector with all the values the variable had
    //! during the scope construction.
    //! The order of the values is the reverse order of construction (most recent
    //! overrides comes first).
    const GsvFlattenedAllDataSet& data() const;

    //! Find and replaces Graph Scope Variables by their values on a given string.
    //! The variable begin is marked by an '%' character optionally followed by '{'.
    //! A variable name is defined as an uninterrupted sequence of valid characters
    //! (i.e. the set [a-zA-Z0-9_]) until a character that is not valid is found. In
    //! the case '{' is part of the variable being marker, '}' must be the
    //! terminating character. Once a variable name is identified, the variable
    //! value is retrieved and the variable name along its preceding '%', and '{}'
    //! if present, are replaced by its value.
    //!
    //! Recursive expressions are permited and expressions with infinite recursion
    //! are ignored. For example, with two variables 'shot' and 'seq' with values
    //! 'shotValue' and 'this_is_%{shot}' respectively, the evaluated value of 'seq'
    //! would be 'this_is_shotValue'.
    //!
    //! Some variable names are ignored so they wont clash with existing features.
    //! They are: v, V and d.
    //!
    //! It returns flag to indicate what happened during the expansion. See
    //! eGsvUnchanged, eGsvExpanded, eGsvUndefined and eGsvInfiniteRecursion.
    int evaluate(const std::string& expression, std::string& expanded) const;

    //! Returns a shared pointer to the GraphScope.
    GraphScopePtr ptr();

  private:
    friend class GraphScopeRepository;
    friend class GraphScopeImpl;
    GraphScope(class GraphScopeImpl&& data);
    //! For debugging only, do not use for serialization
    friend NDK_API std::ostream& operator<<(std::ostream& os, const GraphScope& scope);

    std::unique_ptr<const GraphScopeImpl> _pImpl;
  };


  //----------------------------------------------------------------------


  //! GraphScopeRepository constructs and store GraphScope objects.
  class NDK_API GraphScopeRepository : public std::enable_shared_from_this<GraphScopeRepository> {
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


} // namespace ndk

#endif
