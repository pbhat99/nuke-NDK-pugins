// CurveKnobI.h
// Copyright (c) 2012 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_CurveKnobI_h
#define DDImage_CurveKnobI_h

#include "DDImage/DDImage_API.h"
#include "DDImage/ElementI.h"

namespace DD
{
  namespace Image
  {
    class OutputContext;

    class DDImage_API CurveKnobI
    {
      public:
        typedef ElementI::ElementPtr ElementPtr;
      protected:
        CurveKnobI();
      private:
        CurveKnobI(const CurveKnobI&);
      public:
        virtual ~CurveKnobI();
        virtual ElementPtr rootElement() = 0;
        virtual ElementPtr toElement(const char* curvePath) = 0; 
        virtual void evaluate(const OutputContext &) = 0;

        //! The tree is treated as if it has changed, it is updated and re-evaluated.
        //! If incrementTreeVersion is true, all cached data is marked data and will be refreshed.
        virtual void updateCurveTree(bool incrementTreeVersion) = 0;

        //! Add a new element whose parent will be \var parentElement and \return it.
        virtual ElementPtr addElement(CurveI::CurveType curveType, const std::string& name, const ElementPtr& parentElement) = 0;
        //! Remove the given element. If it has children and removeChildren is false, 
        //! then the children are parented to element's parent. Otherwise, they are removed as well.
        virtual bool removeElement(ElementPtr element, bool removeChildren) = 0;

        //! Begin an undo operation. If the change won't affect children, and isn't destructive \var
        //! noChildren should be set to true.
        virtual void beginUndo(const std::string& name, const bool noChildren, const ElementPtr& node) = 0;
        virtual void endUndo(const std::string& name, const bool noChildren, const ElementPtr& node) = 0;
    };
  }
}

#endif

// Copyright (c) 2012 The Foundry Visionmongers Ltd.  All Rights Reserved.
