// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
#ifndef DDImage_WidgetContext_h
#define DDImage_WidgetContext_h
#pragma once

//----------------------------------------------------------------------------------

#include "DDImage/DDImage_API.h"
#include <memory>
#include <cassert>

//----------------------------------------------------------------------------------

namespace DD
{
  namespace Image
  {

//----------------------------------------------------------------------------------

class WidgetContextImpl;

//----------------------------------------------------------------------------------

class DDImage_API WidgetContext
{
private :
  WidgetContextImpl* _pImpl;

public :

  typedef unsigned char WidgetContextFlags;
  enum EFlags{
    eDefault           = 0,
    eVerticalToolbar   = 1,
    eHorizontalToolbar = 1<<1,
  };

  //! instantiates a default WidgetContext with the eDefault.
  WidgetContext();
  //! instantiate a with a specific set of flags, most likely a toolbar context setup.
  WidgetContext(const WidgetContextFlags flags);
  //! copy c'tor.
  WidgetContext(const WidgetContext& context);
  ~WidgetContext();

  //! gets the context flags for custom editing of widgets.
  WidgetContextFlags getFlags() const;
  //! set the minimum height for widgets created using this context.
  void setMinHeight(const int newHeight);
  //! as setMinHeight() but for width.
  void setMinWidth(const int newWidth);
  //! getter for the value set by setMinHeight().
  int getMinHeight() const;
  //! getter for the value set by setMinWidth().
  int getMinWidth() const;
  //! set the size to be used by icon-buttons.
  void setIconButtonSize(const int w, const int h);
  //! getter for the 'w' value set by setIconButtonSize().
  int getIconButtonWidth() const;
  //! getter for the 'h' value set by setIconButtonSize().
  int getIconButtonHeight() const;
};

//----------------------------------------------------------------------------------

  } // namespace Image
} // namespace DD

#endif // DDImage_WidgetContext_h

