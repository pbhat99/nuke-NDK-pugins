// ElementI.h
// Copyright (c) 2012 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_ElementI_h
#define DDImage_ElementI_h

#include "DDImage/CurveI.h"
#include "DDImage/DDImage_API.h"
#include "DDImage/RefCountedObject.h"

namespace DD
{
  namespace Image
  {
    class OutputContext;

    class DDImage_API ElementI : public RefCountedObject 
    {
      public:
        typedef RefCountedPtr<ElementI> ElementPtr;
        typedef CurveI::CurvePtr CurvePtr;
      protected:
        ElementI();
        ElementI(const ElementI &);
      public:
        ~ElementI() override;

        virtual bool hasChildren() const = 0;
        virtual size_t numChildren() const = 0;
        virtual ElementPtr getChild(size_t childIndex) const = 0;

        virtual bool hasCurves() const = 0;
        virtual size_t numCurves() const = 0;
        virtual CurvePtr getCurve(size_t curveIndex, const std::string& view = "") const = 0;

        virtual float getAttribute(const OutputContext &, const char *attrName) const = 0;
        virtual void setAttribute(const char *attrName, const float value) = 0;
        virtual void setAttribute(const OutputContext &, const char *attrName, const float value) = 0;
    }; 
  }
}

#endif

// Copyright (c) 2012 The Foundry Visionmongers Ltd.  All Rights Reserved.

