//------------------------------------------------------------------------------
// OpTree: Handles a tree of operations for processing and associated states.
//
// Copyright (c) 2010 The Foundry Visionmongers Ltd.  All Rights Reserved.
//------------------------------------------------------------------------------

#ifndef DDIMAGE_OPTREE_H
#define DDIMAGE_OPTREE_H

#include <set>
#include <vector>
#include <string>

#include "DDImage/DDImage_API.h"

//------------------------------------------------------------------------------

namespace DD
{
  namespace Image
  {

    class RecursiveLock;
    class Op;

    //------------------------------------------------------------------------------

    //! This class encapsulates a tree of ops to process, and associated states for that processing.
    //! It also manages a list of observers to notify when ops' error states change.
    class DDImage_API OpTree
    {
    public:

      explicit OpTree(Op* lpRoot = nullptr);
      ~OpTree();

      void abort();
      void cancel();

      void clearAbortedFlag();
      void clearCancelledFlag();

      inline bool wasAborted() const      { return _aborted; }
      inline bool wasCancelled() const    { return _cancelled; }
      
      inline void setExecuting(bool lExecuting) { _executing = lExecuting; }
      inline bool isExecuting() const           { return _executing; }
      
      bool addMessageOp(Op* lpOp);        //!< Called when an op in the tree enters an error state; returns true iff not in the set
      bool removeMessageOp(Op* lpOp);     //!< Called when an op in the tree has its error cleared; returns true iff in the set
      
      bool hasMessage() const;                                                  //!< Returns true if any op in this tree has a message set
      inline bool hasError() const   { return getFirstOpWithError() != nullptr; }  //!< Returns true if any op in this tree has an error set
      
      Op* getFirstOpWithMessage() const;  //!< Returns the first op in the tree with a message set, or NULL if none do
      Op* getFirstOpWithError() const;    //!< Returns the first op in the tree with an error set, or NULL if none do
      
      //! Changes the tree root to a different op, updating references appropriately; not thread safe.
      //! Also clears aborted and cancelled flags.
      void setNewRoot(Op* lpRoot);
      
      inline Op* getRoot() const { return _pRoot; }
      
      //! Information about an individual tree for debugging purposes
      //\{
      int getDebugIndex() const                       { return _debugIndex; }
      void setDebugLabel(const std::string& lrLabel)  { _debugLabel = lrLabel; }
      const std::string& getDebugLabel() const        { return _debugLabel; }
      int getNumMessageOps() const;
      //\}

    private:

      OpTree(const OpTree& lrNoCopying);
      OpTree& operator = (const OpTree& lrNoCopying);
      
      //! Debugging info: traces the details of ops with messages
      void traceMessageOps() const;


      Op*                     _pRoot;           //!< All ops in this tree are connected to the inputs of this op
      std::set<Op*>           _messageOps;      //!< Holds all ops in this tree that have an active message set
      RecursiveLock*          _pMessageLock;    //!< Held while modifying or accessing the message op set
      volatile bool           _aborted;         //!< Set to indicate that processing on this tree has been aborted due to an error
      volatile bool           _cancelled;       //!< Set to indicate that the user has requested to cancel processing on this tree
      volatile bool           _executing;       //!< Set to indicate that the tree is being processed - e. g. for a render or viewer image
      bool                    _hasError;        //!< True when this tree contains an error message op
      
      int                     _debugIndex;      //!< A sequential index of the tree, for debugging purposes
      std::string             _debugLabel;      //!< A label for debugging purposes

    };

    //------------------------------------------------------------------------------

  }   // Image
}     // DD

//------------------------------------------------------------------------------

#endif  // DDIMAGE_OPTREE_H
