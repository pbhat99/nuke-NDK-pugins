// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef USG_TOOLS_PATHEXPRESSION_H
#define USG_TOOLS_PATHEXPRESSION_H

#include "usg/api.h"
#include "usg/geom/Path.h"
#include "usg/geom/Stage.h"

#include <memory>
#include <string>

namespace usg {
  class MatcherImpl;
  class PathExpressionImpl;


  //! A compiled path-matching expression.
  class USG_API PathExpression {
  public:
    //! Constructor: Compiles the expression
    PathExpression(const std::string s);
    ~PathExpression();

    //! Returns true if the expression compiled successfully
    bool isValid() const;

    //! Returns the error message if an error occurred during parsing.
    std::string errorMessage() const;

    //! Executes the expression for the given root prim and returns a result set
    PathArray match(const StageRef& stage, const Prim::FlagsPredicate& predicate = Prim::DefaultPredicate) const;

    //! A class for doing batch matching of individual paths
    class USG_API Matcher {
    public:
      Matcher(PathExpression& expression, const usg::StageRef& stage);
      ~Matcher();

      //! Executes the expression for a single prim and returns true if it matches
      bool match(const usg::Prim& prim) const;

    private:
      std::unique_ptr<MatcherImpl, void(*)(MatcherImpl*)> _pImpl;
    };

  private:
    std::unique_ptr<PathExpressionImpl, void(*)(PathExpressionImpl*)> _pImpl;
  };

}

#endif

