// Copyright (c) 2021 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDIMAGE_STRINGKNOBI_H
#define DDIMAGE_STRINGKNOBI_H

#include "DDImage/DDImage_API.h"
#include "DDImage/ValueProvider.h"

namespace DD {
  namespace Image {
    class OutputContext;
    class Knob;

    //! \class DD::Image::StringKnobI
    //! \brief interface class for an array of characters or a string.
    class StringKnobI : public ValueProvidableI
    {
    public:
      static const int kNoSelection = -1; //< indicates no selection in callback

      //! callback for when the cursor position changes
      typedef void (CursorPositionChangedCallback)(Knob* knob, int position, int selectionStart, int selectionEnd);

      //! set the cursor to the absolute position and optionally move the anchor also
      virtual void setCursorPosition(int newPosition, bool moveAnchor) = 0;
      //! register a callback object to be called when the cursor position changes
      virtual void registerCursorChangedCallback(CursorPositionChangedCallback* cb) = 0;

    public:
      //! Return the Knob this interface is on. Implementations should just "return this;"
      virtual Knob* baseKnob() = 0;

      virtual ~StringKnobI() {}

      //! Sets the default value for the StringKnob.
      //! If \a store is true then the string content is copied locally into the StringKnob,
      //! otherwise the calling object retains ownership of the string buffer.
      virtual void default_value(const char* s, bool store = false) = 0;

      //! \a evaluate only valid for EvalStringKnob subclasses. Returns a non-evaluated (raw) string
      //! if \a evaluate is false.
      virtual const char* get_text(const OutputContext* context, bool evaluate = true) const = 0;

    };
  }
}

#endif
