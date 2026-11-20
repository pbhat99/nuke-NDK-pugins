// Copyright (c) 2024 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef USG_SHADE_MATERIALNETWORK_H
#define USG_SHADE_MATERIALNETWORK_H

#include "usg/api.h"
#include "usg/base/Token.h"
#include "usg/base/Value.h"
#include "usg/geom/Layer.h"
#include "usg/geom/MaterialPrim.h"

namespace usg {
namespace shade {

using TokenAndType = std::pair<Token, Value::Type>;

struct USG_API PropertyInterface {
  std::string name;
  Value::Type type;
  std::string displayGroup;
  std::string displayName;
  Value value;

  PropertyInterface(std::string_view name, const Value::Type type)
  : name(name)
  , type(type)
  {
  }

  PropertyInterface(std::string_view name, const Value::Type type, std::string_view displayGroup)
  : name(name)
  , type(type)
  , displayGroup(displayGroup)
  {
  }

  PropertyInterface(std::string_view name, const Value::Type type, std::string_view displayGroup,
                    std::string_view displayName)
  : name(name)
  , type(type)
  , displayGroup(displayGroup)
  , displayName(displayName)
  {
  }

  bool operator<(const PropertyInterface& rhs) const
  {
    return name < rhs.name;
  }

  bool operator==(const PropertyInterface& rhs) const
  {
    return name == rhs.name && type == rhs.type && displayGroup == rhs.displayGroup &&
           displayName == rhs.displayName;
  }

  friend std::ostream& operator<<(std::ostream& os, const PropertyInterface& v)
  {
    os << v.name << "('" << v.displayName << "'), " << v.displayGroup << ", " << usg::Value::getTypeName(v.type);
    return os;
  }
};

struct USG_API MaterialConnection {
  Path upstreamNode;
  TokenAndType upstreamOutput;
  bool interfaceConnection = false;

  bool operator==(const MaterialConnection& rhs) const {
    return upstreamNode == rhs.upstreamNode &&
           upstreamOutput == rhs.upstreamOutput;
  }
  friend std::ostream& operator<<(
      std::ostream& os, const MaterialConnection& materialConnection) {
    os << "Connection: " << materialConnection.upstreamNode << ":"
       << materialConnection.upstreamOutput.first;
    return os;
  }
};

struct USG_API MaterialNode {
  Token nodeTypeId;
  std::map<PropertyInterface, usg::Value> parameters;
  std::map<TokenAndType, std::vector<MaterialConnection>> inputConnections;
  Value position;
  Value color;
  Value viewState;

  bool operator==(const MaterialNode& rhs) const {
    return (nodeTypeId == rhs.nodeTypeId && parameters == rhs.parameters &&
            inputConnections == rhs.inputConnections);
  }
  friend std::ostream& operator<<(std::ostream& os,
                                  const MaterialNode& materialNetwork) {
    os << "Node ID:" << materialNetwork.nodeTypeId << "\n";
    os << "  Parameters {\n";
    for (const auto& [propertyInterface, value] : materialNetwork.parameters) {
      os << "    " << propertyInterface << ": " << value << "\n";
    }
    os << "  }\n";
    os << "  Inputs connections {\n";
    for (const auto& [input, connectionArray] :
         materialNetwork.inputConnections) {
      os << "    " << input.first << ", " << usg::Value::getTypeName(input.second)
         << ": \n";
      for (const auto& connection : connectionArray) {
        os << "      " << connection << "\n";
      }
    }
    os << "  }\n";
    return os;
  }
};

struct USG_API MaterialNetwork {
  std::map<Path, MaterialNode> nodes;
  std::map<Token, MaterialConnection> terminals;
  std::map<PropertyInterface, usg::Value> parametersInterface;

  bool operator==(const MaterialNetwork& rhs) const {
    return nodes == rhs.nodes && terminals == rhs.terminals;
  }

  friend std::ostream& operator<<(std::ostream& os,
                                  const MaterialNetwork& materialNetwork) {
    os << "Material Network\n";
    os << "Terminals {\n";
    for (const auto& [input, upstreamCon] : materialNetwork.terminals) {
      os << "  " << input << ": " << upstreamCon << "\n";
    }
    os << "}\n";
    os << "Nodes {\n";
    for (const auto& [path, node] : materialNetwork.nodes) {
      os << "  " << path << "\n";
      os << "  " << node << "\n";
    }
    os << "}\n";
    os << "Parameters interface {\n";
    for (const auto& [propertyInterface, value] : materialNetwork.parametersInterface) {
      os << "  " << propertyInterface << ": " << value << "\n";
    }
    os << "}\n";
    return os;
  }
};

struct USG_API ChildMaterial {
  Path parent;
  std::map<PropertyInterface, usg::Value> parametersInterface;

  friend std::ostream& operator<<(std::ostream& os,
                                  const ChildMaterial& childMaterial) {
    os << "Child Material\n";
    os << "Parent path: " << childMaterial.parent << "\n";
    os << "Parameters {\n";
    for (const auto& [propertyInterface, value] : childMaterial.parametersInterface) {
      os << "  " << propertyInterface << ": " << value << "\n";
    }
    os << "}\n";
    return os;
  }
};

void USG_API writeMaterialNetworkToLayer(
    const MaterialNetwork& materialNetwork, MaterialPrim& materialPrim,
    const LayerRef& editLayer,
    const fdk::TimeValue time = fdk::defaultTimeValue());

void USG_API writeChildMaterialToLayer(
    const ChildMaterial& childMaterial, MaterialPrim& materialPrim,
    const fdk::TimeValue time = fdk::defaultTimeValue(),
    bool isOverride = false);

void USG_API
writeMaterialNodeToPrim(const MaterialNode& node, Prim& prim,
                        const fdk::TimeValue time = fdk::defaultTimeValue());
}  // namespace shade
}  // namespace usg
#endif
