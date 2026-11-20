// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef USG_ENGINE_GEOMSTATETARGET_H
#define USG_ENGINE_GEOMSTATETARGET_H

#include "usg/engine/api.h"

#include "usg/base/Token.h"

#include "fdk/base/Hash.h"

#include <unordered_map>
#include <unordered_set>
#include <vector>


namespace usg {

  class GeomStateTarget;
  using GeomStateTargetPtrSet  = std::unordered_set<GeomStateTarget*>;
  using GeomStateTargetPtrMap  = std::unordered_map<usg::Token, GeomStateTarget*>;
  using GeomStateTargetPtrList = std::vector<GeomStateTarget*>;


  /*! A named 'target' that values are contributing to.

      A 'target' can be abstractly thought of as something that a GeomEngine affects or
      will process, and its dirty/clean state is changed by arbitrary local values,
      input values, input GeomEngine, etc.

      Examples of named targets could be 'define-geometry', 'modify-attribute',
      'prim-xform', 'image-size', 'blur-channel', 'image-position', etc.

      Interconnected targets form a group hierachy and all the targets inside a group
      contribute to the groups's hash and thus form a simple dependency graph.

      Some example target paths:
        define-geometry
        define-geometry:define-mesh
        define-geometry:define-mesh:define-points
        define-geometry:define-mesh:define-texcoords
        define-geometry:import-layer
        modify-values:modify-points
        modify-values:modify-texcoords

      The total hash result is not cached in this class as caching the result would mean
      keeping track of the hierarchy state somehow. It is assumed that this object will
      be managed by GeomEngines or some other state tracking mechanism that can call reset(),
      or resetAll() to clear the state.
  */
  class USG_ENGINE_API GeomStateTarget
  {
  public:
    GeomStateTarget(const usg::Token& identifier);

    //! Identifier string
    const usg::Token&  identifier() const { return _identifier; }

    //! Return the full path of this target and all its parents.
    std::string        fullPath() const;

    //-------------------------------------------------------------

    //! Connect this target to larger group.
    void               setParent(GeomStateTarget& parent);

    //! Return the parent target if there is one.
    GeomStateTarget*   parent() const { return _parent; }
    //! Return the parent target id if there is one, otherwise an empty token.
    const usg::Token&  parentId() const;

    //! Top parent target, walks up graph.
    GeomStateTarget*   root();

    /*! Returns a list of this target and its parents, where the root target
        is entry 0 and the last entry is this target.
    */
    void               getParents(GeomStateTargetPtrList& parents) const;

    //! Child targets. All child hashes are included in this target's total hash.
    const GeomStateTargetPtrSet&  children() const { return _children; }

    /*! Removes child targets and their associated connections in parent/child targets.
        Does not affect local hash.
    */
    void               clearConnections();

    //-------------------------------------------------------------

    /*! Total hash of target including all children.
        This recursively calls all the child hash() methods and the final result is
        returned but not cached in this class. It's assumed to be cached and
        dirty-managed elsewhere.

        Warning, no recursion or loop testing is performed!
    */
    fdk::Hash         hash() const;

    //! Current local hash of target without inclusion of parents.
    const fdk::Hash&  localHash() const { return _localHash; }

    //! Set the target's local hash to a specific value.
    void              setLocalHash(const fdk::Hash& h) { _localHash = h; }

    //! Append a hash to the target's local hash.
    void              appendLocalHash(const fdk::Hash& h) { _localHash.appendIfNotDefault(h); }

    //! Resets (sets to default value) the local hash.
    void              resetLocalHash() { _localHash.reset(); }

    //---------------------------------------------------

    //! Total version hash including all children. Built the same way as hash().
    fdk::Hash         versionHash() const;

    //! Current version hash of the local hash.
    fdk::Hash         localVersionHash() const { return _localVersionHash; }

    //! Set the target's local version hash to a specific value.
    void              setLocalVersionHash(const fdk::Hash& h) { _localVersionHash = h; }

    //! Append a hash to the target's local version hash.
    void              appendLocalVersionHash(const fdk::Hash& h) { _localVersionHash.appendIfNotDefault(h); }

    //! Resets (sets to default value) the local version hash.
    void              resetLocalVersionHash() { _localVersionHash.reset(); }

    //---------------------------------------------------

    //! Is this target or any of the contributing children animating?
    bool  isAnimating() const;

    //! Is the local hash *expected* to animate over time?
    bool  isLocalHashAnimating() const { return _animating; }

    //! Set the local animation indicator to a specific state.
    void  setIsLocalHashAnimating(bool isAnimating) { _animating = isAnimating; }

    //! Enable the local animation indicator.
    void  enableVaryingOutputHash() { _animating = true; }


    //! Resets the local hash and animation state, and all child targets recursively.
    void  resetAll();


  protected:
    usg::Token            _identifier;        //!< Should be unique so that shared target identifiers can be used
    fdk::Hash             _localHash;         //!< Target's local hash contribution
    fdk::Hash             _localVersionHash;  //!< Local version number hash
    bool                  _animating;         //!< Is the target's local hash expected to change over time
    GeomStateTarget*      _parent;            //!< If a child of another target
    GeomStateTargetPtrSet _children;          //!< Child targets contributing to this target's hash

  };

  USG_ENGINE_API std::ostream& operator << (std::ostream& o, const GeomStateTarget&);

  using GeomStateTargetArray  = std::vector<GeomStateTarget>;
  using GeomStateTargetIdxMap = std::unordered_map<usg::Token, uint32_t>;


  //--------------------------------------------------------------------------------

#if 0
  class USG_ENGINE_API GeomStateTargetClip
  {
  public:
    usg::Token        name;       //!< Name of clip.
    fdk::TimeValueSet times;      //!< Time set

    GeomStateTargetClip(const GeomStateTarget&    target,
                        const KnobBindingMapList& view_bindings,
                        int32_t                   view = -1);

    // Equality only tests name.
    bool operator == (const GeomStateTargetClip& b) const { return (name == b.name); }
    bool operator != (const GeomStateTargetClip& b) const { return (name != b.name); }
    // Greater/less-than only tests name.
    bool operator <  (const GeomStateTargetClip& b) const { return (name < b.name); }
    bool operator >  (const GeomStateTargetClip& b) const { return (name > b.name); }


  protected:
    friend USG_ENGINE_API std::ostream& operator << (std::ostream&, const GeomStateTargetClip&);
  };

  USG_ENGINE_API std::ostream& operator << (std::ostream& o, const GeomStateTargetClip&);
#endif


} // namespace usg


#endif
