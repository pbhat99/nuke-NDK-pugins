// Copyright (c) 2024 The Foundry Visionmongers Ltd. All Rights Reserved.

// This is automatically-generated code.
// You may add custom code between the --(BEGIN CUSTOM CODE)-- and --(END CUSTOM CODE)-- markers.
// Code added anywhere else will be destroyed when the code is generated.

#ifndef USG_UI_NODEGRAPHNODEAPI_H
#define USG_UI_NODEGRAPHNODEAPI_H

#include "usg/api.h"
#include "usg/geom/Prim.h"
#include "usg/ui/UITokens.h"

#include "fdk/math/Vec3.h"
#include "fdk/math/Vec2.h"

namespace usg {
namespace ui {

  // class NodeGraphNodeAPI
  //
  // Attributes:
  //     uniform color3f ui:nodegraph:node:displayColor
  //     uniform token ui:nodegraph:node:expansionState (allowed values: open, closed, minimized)
  //     uniform asset ui:nodegraph:node:icon
  //     uniform float2 ui:nodegraph:node:pos
  //     uniform float2 ui:nodegraph:node:size
  //     uniform int ui:nodegraph:node:stackingOrder

  class USG_API NodeGraphNodeAPI {
  public:
    NodeGraphNodeAPI();
    NodeGraphNodeAPI(const Prim& prim);
    ~NodeGraphNodeAPI();

    bool validate();
    static bool isValidPrim(const Prim& prim);
    explicit operator bool() const { return isValidPrim(_prim); }

    void apply();

    Attribute createNodegraphNodeDisplayColorAttr(const Value& defaultValue = Value());
    Attribute getNodegraphNodeDisplayColorAttr() const;
    void setNodegraphNodeDisplayColor(const fdk::Vec3f& value, fdk::TimeValue time = fdk::defaultTimeValue());
    fdk::Vec3f getNodegraphNodeDisplayColor(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createNodegraphNodeExpansionStateAttr(const Value& defaultValue = Value());
    Attribute getNodegraphNodeExpansionStateAttr() const;
    void setNodegraphNodeExpansionState(const Token& value, fdk::TimeValue time = fdk::defaultTimeValue());
    Token getNodegraphNodeExpansionState(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createNodegraphNodeIconAttr(const Value& defaultValue = Value());
    Attribute getNodegraphNodeIconAttr() const;
    void setNodegraphNodeIcon(const AssetPath& value, fdk::TimeValue time = fdk::defaultTimeValue());
    AssetPath getNodegraphNodeIcon(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createNodegraphNodePosAttr(const Value& defaultValue = Value());
    Attribute getNodegraphNodePosAttr() const;
    void setNodegraphNodePos(const fdk::Vec2f& value, fdk::TimeValue time = fdk::defaultTimeValue());
    fdk::Vec2f getNodegraphNodePos(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createNodegraphNodeSizeAttr(const Value& defaultValue = Value());
    Attribute getNodegraphNodeSizeAttr() const;
    void setNodegraphNodeSize(const fdk::Vec2f& value, fdk::TimeValue time = fdk::defaultTimeValue());
    fdk::Vec2f getNodegraphNodeSize(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createNodegraphNodeStackingOrderAttr(const Value& defaultValue = Value());
    Attribute getNodegraphNodeStackingOrderAttr() const;
    void setNodegraphNodeStackingOrder(int value, fdk::TimeValue time = fdk::defaultTimeValue());
    int getNodegraphNodeStackingOrder(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    // --(BEGIN CUSTOM CODE)--
    // --(END CUSTOM CODE)--
  protected:
    Prim _prim;
  };

}
}

#endif
