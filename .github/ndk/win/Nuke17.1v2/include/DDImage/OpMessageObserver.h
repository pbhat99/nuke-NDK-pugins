//------------------------------------------------------------------------------
// OpMessageObserver: Interface for classes that can add their objects to the
//                    list to be notified when an op message is set or cleared.
//
// Copyright (c) 2010 The Foundry Visionmongers Ltd.  All Rights Reserved.
//------------------------------------------------------------------------------

#ifndef DDIMAGE_OPMESSAGEOBSERVER_H
#define DDIMAGE_OPMESSAGEOBSERVER_H

#include <vector>

#include "DDImage/DDImage_API.h"

//------------------------------------------------------------------------------

namespace DD
{
  namespace Image
  {
    class Op;
    class OpTree;
    class OpMessage;

    //------------------------------------------------------------------------------
    
    //! Classes can inherit this and add themselves to the op message observers list
    //! to be notified when an op message is set or cleared, or when an op's children
    //! have changed so the message's effects may do so too.
    class DDImage_API OpMessageObserver
    {
    public:
      enum MessageChange {
        kMC_Set,              //!< Sent after a message is set
        kMC_Cleared,          //!< Sent after a message is cleared
        kMC_ChildrenChanged   //!< Sent with a blank message after an op's vector of children has changed
      };
      
      //! The op passed into this function will never be NULL
      virtual void notify(const OpMessage& lrMessage, MessageChange lMsgChange, Op* lpSourceOp,
                          const std::vector<OpTree*>& lrAffectedTrees) = 0;
      
    protected:
      //! Protected constructor: only create from child classes
      OpMessageObserver();
      
      //! Protected destructor (not for polymorphic deletion)
      //! Automatically removes this object from the OpMessageHandler observer list.
      //! Doesn't need to be virtual, because it's protected, but making it so reduces warnings.
      virtual ~OpMessageObserver();
    };

    //------------------------------------------------------------------------------
  }
}

//------------------------------------------------------------------------------

#endif  // DDIMAGE_OPMESSAGEOBSERVER_H
