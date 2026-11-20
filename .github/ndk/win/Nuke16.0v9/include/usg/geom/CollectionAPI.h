// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

// This is automatically-generated code.
// You may add custom code between the --(BEGIN CUSTOM CODE)-- and --(END CUSTOM CODE)-- markers.
// Code added anywhere else will be destroyed when the code is generated.

#ifndef USG_GEOM_COLLECTIONAPI_H
#define USG_GEOM_COLLECTIONAPI_H

#include "usg/api.h"

#include "usg/base/ArrayTypes.h"
#include "usg/geom/PathExpr.h"
#include "usg/geom/Prim.h"
#include "usg/geom/Tokens.h"

namespace usg {

  // class CollectionAPI
  //
  // Attributes:
  //     rel excludes
  //     uniform token expansionRule = "expandPrims" (allowed values: explicitOnly, expandPrims, expandPrimsAndProperties)
  //     uniform bool includeRoot
  //     rel includes

  class USG_API CollectionAPI
  {
    public:
    CollectionAPI();
    CollectionAPI(const Prim& prim, const Token& name);
    ~CollectionAPI();

    bool validate();
    static bool isValidPrim(const Prim& prim, const Token& instanceName = Token());
    explicit operator bool() const { return isValidPrim(_prim); }

    void apply();

    Relationship createExcludesRel();
    Relationship getExcludesRel() const;
    void setExcludes(const PathArray& value);
    PathArray getExcludes() const;

    Attribute createExpansionRuleAttr(const Value& defaultValue = Value());
    Attribute getExpansionRuleAttr() const;
    void setExpansionRule(const Token& value, fdk::TimeValue time = fdk::defaultTimeValue());
    Token getExpansionRule(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createIncludeRootAttr(const Value& defaultValue = Value());
    Attribute getIncludeRootAttr() const;
    void setIncludeRoot(bool value, fdk::TimeValue time = fdk::defaultTimeValue());
    bool getIncludeRoot(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Relationship createIncludesRel();
    Relationship getIncludesRel() const;
    void setIncludes(const PathArray& value);
    PathArray getIncludes() const;

    // --(BEGIN CUSTOM CODE)--

    Token getName() const;
    static std::vector<CollectionAPI> getAll(const usg::Prim& prim);
    static std::vector<Token> getAllNames(const usg::Prim& prim);

    Attribute createMembershipExpressionAttr(const Value& defaultValue = Value());
    Attribute getMembershipExpressionAttr() const;
    void setMembershipExpression(const PathExpr& value,
                                 fdk::TimeValue time = fdk::defaultTimeValue());
    PathExpr getMembershipExpression(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    bool isInRelationshipsMode() const;
    bool isInExpressionMode() const;

    usg::PathArray computeIncludedPaths() const;

    // --(END CUSTOM CODE)--
    protected:
    Prim _prim;
    Token _name;
  };

}

#endif
