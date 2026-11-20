// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef USG_GEOM_PRIMRANGE_H
#define USG_GEOM_PRIMRANGE_H

#include "usg/geom/Prim.h"

#include <memory>

namespace usg
{
  class Stage;
  using StageRef = std::shared_ptr<Stage>;
  class PrimRange;


  //! class PrimRange
  //! A forward-iterable 'range' of Prims.
  //! In reality this class does not store a list of Prims, it only contains a 'begin' Prim and a
  //! 'past-the-end' indicator pseudo-Prim which a PrimRange::iterator can walk through in a
  //! depth-first manner (ie it visits all children first before moving back up to the next sibling prim.)
  //!
  //! Since it only contains a couple Prim references a PrimRange is cheap to construct and fast to walk
  //! through compared to getting a list of Prims via Stage::findPrimsOfType() or Stage::getPrimPaths()
  //! since it allows a loop to perform checks on each Prim in turn during traversal.
  //!
  //! Example stage traversal loop which finds the first CameraPrim in a stage:
  //!  Prim firstCamera;
  //!  const usg::PrimRange traverser(stage->getRootPrim());
  //!  for (usg::PrimRange::const_iterator it = traverser.begin(); it != traverser.end(); ++it) {
  //!    if ((*it).isA<usg::CameraPrim>()) {
  //!      firstCamera = *it;
  //!      break;
  //!    }
  //!  }
  //!
  class USG_API PrimRange
  {
  public:
    class Impl; //!< Wrapper abstraction for implementation type
    class Handle; //!< Abstraction handle for casting to UsdPrimRange

    //! Create a PrimRange wrapper from a PrimRange::Impl.
    PrimRange(const PrimRange::Impl&);

    //! Create a PrimRange starting at prim \a start which will traverse prims matching the \a mask argument.
    PrimRange(const Prim&                 start,
              const Prim::FlagsPredicate& mask = Prim::DefaultPredicate);

    ~PrimRange();


    //! Return true if range is empty.
    bool empty() const { return begin() == end(); }
    explicit operator bool() const { return !empty(); }

    //! Return the first element in this range, or an invalid Prim if range is empty.
    Prim front() const;


    bool operator == (const PrimRange& b) const;
    bool operator != (const PrimRange& b) const { return !(*this == b); }


    static PrimRange preAndPostVisit(const Prim&                 start,
                                     const Prim::FlagsPredicate& mask = Prim::DefaultPredicate);
    static PrimRange allPrims(const Prim& start);
    static PrimRange allPrimsPreAndPostVisit(const Prim& start);

    static PrimRange stage(const Stage&                stage,
                           const Prim::FlagsPredicate& mask = Prim::DefaultPredicate);

    //-------------------------------------------------------------------

    //! class iterator
    //! Forward-only iterator valid only for the PrimRange it was created on.
    //! Iterator will only walk down the scenegraph, depth first (ie visits all
    //! children first before moving back up to next upper-level prim.)
    class USG_API iterator
    {
    public:
      class Impl; //!< Wrapper abstraction for implementation type

      //! Create a PrimRange::iterator wrapper from a PrimRange::iterator::Impl.
      iterator(const PrimRange::iterator::Impl&);
      ~iterator();

      //! True if iterator points to a prim that has been visited a second time
      //! (in post order) for a pre- and post-order iterator.
      bool isPostVisit() const;

      //! Stop the iterator from proceeding any further down from its current position.
      void pruneChildren();
 
      bool operator==(const iterator& b) const;
      bool operator!=(const iterator& b) const { return !(*this == b); }

      iterator& operator++();
      iterator  operator++(int);

      //! Currently visited prim.
      Prim currentPrim() const;

      //! Currently visited prim.
      Prim operator*() { return currentPrim(); }


    protected:
      friend class PrimRange;
      explicit iterator(const PrimRange* range);

      std::unique_ptr<PrimRange::iterator::Impl, void(*)(PrimRange::iterator::Impl*)> _pImpl;
    };

    using const_iterator = iterator;

    //-------------------------------------------------------------------

    iterator       begin() const;
    const_iterator cbegin() const;
    iterator       end() const;
    const_iterator cend() const;


  protected:
    std::unique_ptr<PrimRange::Impl, void(*)(PrimRange::Impl*)> _pImpl;
  };


} // namespace usg

#endif
