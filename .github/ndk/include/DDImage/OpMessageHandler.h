//------------------------------------------------------------------------------
// OpMessageHandler: Handles lifetimes and transmissions of op messages such as
//                   errors and warnings.
//
// Copyright (c) 2010 The Foundry Visionmongers Ltd.  All Rights Reserved.
//------------------------------------------------------------------------------

#ifndef DDIMAGE_OPMESSAGEHANDLER_H
#define DDIMAGE_OPMESSAGEHANDLER_H

#include <string>
#include <vector>
#include <list>

#include "DDImage/OpMessage.h"
#include "DDImage/OpMessageObserver.h"

//------------------------------------------------------------------------------

namespace DD
{
  namespace Image
  {
    class RecursiveLock;
    class Op;
    class OpTree;
    class OpTreeHandler;
    
    //------------------------------------------------------------------------------

    //! This component of an op handles its messages to the user
    class DDImage_API OpMessageHandler
    {
    public:
      OpMessageHandler();
      ~OpMessageHandler();
      
      // When adding or clearing messages, it's necessary to pass in the op that caused it and its trees.
      
      //! addMessage only actually adds a new message if it is unique - i. e. if its data (other than the ID) doesn't
      //! match any message currently present.
      void addMessage(const OpMessage& lrMessage, Op* lpSourceOp, const std::vector<OpTree*>& lrAffectedTrees);
      void removeMessage(const OpMessage& lrMessage, Op* lpSourceOp, const std::vector<OpTree*>& lrAffectedTrees);
      void removeMessage(OpMessage::ID lID, Op* lpSourceOp, const std::vector<OpTree*>& lrAffectedTrees);
      void clearAllMessages(Op* lpSourceOp, const std::vector<OpTree*>& lrAffectedTrees);
      void clearMessagesFromSource(Op* lpSourceOp, OpMessage::Source lSource, const std::vector<OpTree*>& lrAffectedTrees);
      
      //! Marks all current messages as old, for potential clearing later
      void markMessagesAsOld();
      //! Clears any messages that are marked as old
      void clearOldMessages(Op* lpSourceOp, const std::vector<OpTree*>& lrAffectedTrees);
      
      //! Notify all observers that this op's vector of children has changed (which affects what messages apply to)
      void notifyChildrenChanged(Op* lpSourceOp, const std::vector<OpTree*>& lrAffectedTrees) const;
      
      inline bool hasMessage() const  { return _hasMessage; }
      inline bool hasError() const    { return _hasError; }
      
      int getNumMessages() const;
      int getNumErrors() const;
      
      const OpMessage* getFirstMessage() const;
      const OpMessage* getLatestMessage() const;
      const OpMessage* getFirstError() const;
      const OpMessage* getLatestError() const;
      
      //! Debugging info: returns information on current messages
      std::string getDebugInfo() const;
      std::string getDetailedDebugInfo() const;

      static void AddObserver(OpMessageObserver* lpObserver);
      static void RemoveObserver(OpMessageObserver* lpObserver);

      static void IgnoreErrors();
      static void ObserveErrors();
      
    private:
      
      OpMessageHandler(const OpMessageHandler& lrNoCopying);
      OpMessageHandler& operator = (const OpMessageHandler& lrNoCopying);
      
      //! Clears messages if they satisfy a given member predicate, which must match the signature: void Fn(OpMessage&)
      template <class OpMessageFn>
      void clearMessagesIf(OpMessageFn lpFnShouldRemove, Op* lpSourceOp, const std::vector<OpTree*>& lrAffectedTrees);
      
      void notifyObservers(const OpMessage& lrMessage, OpMessageObserver::MessageChange lMessageChange,
                           Op* lpSourceOp, const std::vector<OpTree*>& lrOpTrees) const;
      
      
      //! Messages are stored in a simple vector, as they're usually accessed sequentially
      std::vector<OpMessage>  _messages;
      RecursiveLock*          _pLock;
      volatile bool           _hasMessage;    //!< Cached result to save locking
      volatile bool           _hasError;      //!< Cached result to save locking
      
      //! All objects that are interested in hearing when a message is set or cleared
      static std::list<OpMessageObserver*> _sObservers;

      static bool _sIgnoreErrors;
    };

    //------------------------------------------------------------------------------

  }   // Image
}     // DD

//------------------------------------------------------------------------------

#endif  // DDIMAGE_OPMESSAGEHANDLER_H
