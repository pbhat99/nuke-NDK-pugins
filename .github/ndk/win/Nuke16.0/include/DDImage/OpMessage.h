//------------------------------------------------------------------------------
// OpMessage: Encapsulates errors and warnings produced by ops along with
//            various information about them.
//
// Copyright (c) 2010 The Foundry Visionmongers Ltd.  All Rights Reserved.
//------------------------------------------------------------------------------

#ifndef DDIMAGE_OPMESSAGE_H
#define DDIMAGE_OPMESSAGE_H

#include "DDImage/DDImage_API.h"
#include "Build/fnBuild.h"

#include <string>
#include <vector>
#include <utility>
#include <ctime>

#if !defined(FN_COMPILER_INTEL) && defined(FN_OS_WINDOWS)
#  pragma warning(push)
#  pragma warning( disable : 4251 ) // warning #4251: needs to have dll-interface to be used by clients of <X>
#endif // FN_OS_WINDOWS &&  FN_COMPILER_INTEL

//------------------------------------------------------------------------------

namespace DD
{
  namespace Image
  {
    class OpMessageObserver;
    
    //------------------------------------------------------------------------------
    
    //! This class encapsulates a text message to the user produced by an op
    class DDImage_API OpMessage
    {
    public:
      enum Type         { eWarning, eError, eDebug };
      enum Source       { eFromOp, eFromKnob };
      enum EnablePopUp  { ePU_Disable, ePU_Enable };
      enum Empty        { eEmpty };
      typedef unsigned ID;
      
      OpMessage(Type lType, Source lSource, const std::string& lrText, EnablePopUp lEnablePopUp = ePU_Disable);
      //! Use this to explicitly create an empty op message
      explicit OpMessage(Empty lEmpty) { clear(); }
      // Standard copy constructor and destructor.
      
      Type getType() const               { return _type; }
      Source getSource() const           { return _source; }
      bool isError() const               { return _type == eError; }
      const std::string& getText() const { return _text; }
      const time_t& getTime() const      { return _time; }
      ID getID() const                   { return _ID; }
      bool isPopUpEnabled() const        { return _enablePopUp; }
      void disablePopUp() const          { _enablePopUp = false; }
      bool isNew() const                 { return _isNew; }
      void setNew(bool lIsNew) const     { _isNew = lIsNew; }
      
      //! Avoid using this if possible.  It is only provided for use in the Op base class.
      void setPopUpsEnabled(bool lEnabled) const { _enablePopUp = lEnabled; }
      
      bool operator == (const OpMessage& lrOther) const { return _ID == lrOther._ID; }
      bool operator != (const OpMessage& lrOther) const { return _ID != lrOther._ID; }
      bool matchesID(ID lID) const                      { return _ID == lID; }
      bool isEmpty() const                              { return _ID == kEmptyID; }
      
      //! Messages are equivalent iff they will look equivalent to the user (other than message time)
      bool isEquivalentTo(const OpMessage* lpOther) const
        { return _type == lpOther->_type && _source == lpOther->_source && _text == lpOther->_text; }
      
      void clear() { _ID = kEmptyID; }
      
    private:
      
      static const ID kEmptyID = 0;
      
      Type          _type;
      Source        _source;
      std::string   _text;
      mutable bool  _enablePopUp;
      mutable bool  _isNew;       //!< Set and cleared to indicate when messages have been refreshed when revalidating
      time_t        _time;        //!< The time when the message came up
      unsigned      _ID;          //!< Unique identifier of the message - but note that copying the message will maintain the same ID
      
    };
    
    //------------------------------------------------------------------------------
    
  }   // Image
}     // DD

//------------------------------------------------------------------------------

#if !defined(FN_COMPILER_INTEL) && defined(FN_OS_WINDOWS)
#  pragma warning(pop)  // reenable 4251: needs to have dll-interface to be used by clients of <X>
#endif // FN_OS_WINDOWS &&  FN_COMPILER_INTEL

#endif  // DDIMAGE_OPMESSAGE_H
