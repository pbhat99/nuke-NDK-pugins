//------------------------------------------------------------------------------
// OpTreeHandler: Handles tree-related data and methods for ops.
//                Separated via pImpl for binary compatibility between 6.x
//                versions.
//
// Copyright (c) 2010 The Foundry Visionmongers Ltd.  All Rights Reserved.
//------------------------------------------------------------------------------

#ifndef DDIMAGE_OPTREEHANDLER_H
#define DDIMAGE_OPTREEHANDLER_H

#include <vector>
#include <map>

#include "DDImage/OpMessage.h"
#include "DDImage/OpMessageObserver.h"
#include "DDImage/Thread.h"

//------------------------------------------------------------------------------

namespace DD
{
  namespace Image
  {
    class Op;
    class OpTree;
    
    //------------------------------------------------------------------------------

    //! This component of an op handles the manipulations of its trees: adding to, removing from, checking statuses, etc
    class DDImage_API OpTreeHandler
    {
    public:
      OpTreeHandler();
      ~OpTreeHandler();
      
      //! Returns true if the op was added to the tree, or false if only a ref was added.  See OpFns.cpp for further details.
      bool addTree(OpTree* lpTree);
      //! Returns true if the op was removed from the tree, or false if only a ref was removed.  See OpFns.cpp for further details.
      bool removeTree(OpTree* lpTree);
      
      bool isInTree(OpTree* lpTree) const;              //!< Returns whether this handler's op is in the given tree
      bool isInAnyTree() const;                         //!< Returns true iff this handler's op is in at least one op tree
      int getNumTrees() const;                          //!< Returns the number of trees the handler's op is in
      
      bool wasConnectedTreeAborted() const;             //!< Returns true iff any of the trees this handler's op is in was aborted
      bool wasConnectedTreeCancelled() const;           //!< Returns true iff any of the trees this handler's op is in was cancelled
      
      void abortConnectedTrees() const;
      void cancelConnectedTrees() const;
      
      void addOpToTrees(Op* lpOp) const;                //!< If valid, adds the op to each tree
      void removeOpFromTrees(Op* lpOp) const;           //!< If valid, removes the op from each tree
      
      //! Removes the op from the list of message ops on each tree; does not remove it from the tree itself
      void removeMessageOpFromTrees(Op* lpOp) const;
      
      //! Locks the vector of the processing trees that this op belongs to, and returns a reference to it.
      //! Make sure you call \a unlockTrees after calling this.
      //! Avoid calling this externally.  It's only public for access in message observer notifications.
      const std::vector<OpTree*>& lockTrees() const;
      
      //! Unlocks tree access.  Must be called after using \a lockTrees.
      void unlockTrees();
      
      //! Returns a string describing the trees that this op belongs to
      std::string getTreeDebugText() const;
      std::string getDetailedTreeDebugText() const;
      
      inline bool wasWarningGiven() const { return _warningGiven; }   //!< True when a warning for missing op trees has been given
      inline void setWarningGiven() const { _warningGiven = true; }   //!< Set that the warning has been given
      
      bool allowsNoTrees() const { return _allowNoTrees; }    //!< True iff having no trees should be handled as a normal case
      void disallowNoTrees()     { _allowNoTrees = false; }   //!< Set that the object must have trees for normal operation
      
    private:
      
      OpTreeHandler(const OpTreeHandler& lrNoCopying);
      OpTreeHandler& operator = (const OpTreeHandler& lrNoCopying);
      
      void cacheStatus() const;         //!< Store the up to date aborted and cancelled states; slow (uses locks)
      void updateCache() const;         //!< Calls \a cacheStatus if the status version doesn't match the tree manager's version
      
      
      typedef std::vector<OpTree*>  OpTreeVec;
      typedef std::vector<int>      IntVec;
      
      OpTreeVec       _trees;               //!< Stores pointers to the op trees this object's owner op is in
      IntVec          _refCounts;           //!< Ref counts for each tree (matching indices to \a _trees)
      mutable RecursiveLock _treeLock;      //!< Held while accessing or modifying \a _trees and \a _refCounts
      mutable int     _cachedStatusVersion; //!< The last version of global tree status we checked aborted/cancelled at
      mutable bool    _cachedAborted;       //!< The aborted status of this op from the above status version
      mutable bool    _cachedCancelled;     //!< The cancelled status of this op from the above status version
      mutable bool    _cachedValidTrees;    //!< True iff the object has at least one tree and so aborted and cancelled are valid
      mutable bool    _warningGiven;        //!< True when a warning for missing op trees has been given
      bool            _allowNoTrees;        //!< True iff having no trees should be handled as a normal case 
      
    };

    //------------------------------------------------------------------------------

  }   // Image
}     // DD

//------------------------------------------------------------------------------

#endif  // DDIMAGE_OPTREEHANDLER_H
