//------------------------------------------------------------------------------
// OpManager: Encapsulates management operations that apply to all ops, whether
//            in trees or not.
//
// Copyright (c) 2010 The Foundry Visionmongers Ltd.  All Rights Reserved.
//------------------------------------------------------------------------------

#ifndef DDIMAGE_OPMANAGER_H
#define DDIMAGE_OPMANAGER_H

#include "DDImage/DDImage_API.h"

#include <set>
#include <vector>

//------------------------------------------------------------------------------

namespace DD
{
  namespace Image
  {
    class RecursiveLock;
    class Op;
    
    //------------------------------------------------------------------------------
    
    class DDImage_API OpManager
    {
    public:
      OpManager();
      void shutDown();      //!< Performs object shut down that needs to come before static destructors
      ~OpManager();
      
      void addOp(Op* lpOp);
      void removeOp(Op* lpOp);
      
      void addMessageOp(Op* lpOp);
      void removeMessageOp(Op* lpOp);
      
      void abortAll();                            //!< Aborts all ops, including all executing trees
      void cancelAll();                           //!< Cancels all ops, including all executing trees
      void setAborted()   { _aborted = true; }    //!< Sets the general flag without aborting any specific trees
      void setCancelled() { _cancelled = true; }  //!< Sets the general flag without cancelling any specific trees
      
      bool wasAnythingAborted() const   { return _aborted; }    //!< True iff any op in any tree was aborted
      bool wasAnythingCancelled() const { return _cancelled; }  //!< True iff any op in any tree was cancelled
      
      void clearAllAbortedFlags();
      void clearAllCancelledFlags();
      
      Op* findFirstOpWithError() const;           //!< Returns the first op with an error that isn't in any tree, or null if none
      std::vector<Op*> getErrorOps() const;       //!< Returns a vector copy of the ops with errors
      
    private:
      
      std::set<Op*>   _ops;           //!< All ops
      std::set<Op*>   _messageOps;    //!< All ops with active messages
      RecursiveLock*  _pOpLock;       //!< Held while accessing the ops set
      RecursiveLock*  _pMsgOpLock;    //!< Held while accessing the message ops set
      volatile bool   _aborted;       //!< A single aborted state for all ops
      volatile bool   _cancelled;     //!< A single cancelled state for all ops
      
      static bool     _sInitialised;  //!< Indicates construction state of the singleton below
      
    };
    
    //! Global singleton-style manager object - not a true singleton due to compiler and module issues
    extern DDImage_API OpManager gOpManager;
    
    //------------------------------------------------------------------------------
  }
}

//------------------------------------------------------------------------------

#endif  // DDIMAGE_OPMANAGER_H
