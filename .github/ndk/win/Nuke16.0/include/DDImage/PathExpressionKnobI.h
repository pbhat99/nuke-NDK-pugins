// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef DDIMAGE_PATHEXPRESSIONKNOBI_H
#define DDIMAGE_PATHEXPRESSIONKNOBI_H

#include "DDImage/DDImage_API.h"

#include "usg/geom/Stage.h"
#include "usg/engine/GeomEngine.h"

#include <string>

namespace DD
{
  namespace Image
  {
    //! Abstract base class for PathExpression knobs.
    class DDImage_API PathExpression_KnobI
    {
    public:
      virtual ~PathExpression_KnobI() = default;

      //! Returns all paths in \a stage which match \a pattern. This will display any errors that occur.
      //! The \a engine parameter is used for matching tokens such as {lastmodified}. If \a engine is null, the input engine at the UI context, if any, will be used.
      virtual usg::PathArray match(const usg::StageRef& stage, const std::string& pattern, const usg::GeomEngine* engine = nullptr) = 0;
    };
  }
}

#endif
