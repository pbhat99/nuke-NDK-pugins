// Copyright (c) 2023 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef USG_GEOM_UTILS_H
#define USG_GEOM_UTILS_H

#include "fdk/base/StringTypes.h"
#include "usg/api.h"
#include "usg/base/Value.h"
#include "usg/base/Token.h"
#include "usg/geom/Attribute.h"
#include "usg/geom/Layer.h"
#include "usg/geom/Relationship.h"
#include "usg/geom/ShaderRegistry.h"

#include <set>
#include <string_view>

namespace usg {

  struct LightTypeInfo {
    LightTypeInfo() = default;
    LightTypeInfo(std::string_view shaderIdentifier, std::string_view primType,
                  std::string_view apiSchema)
    : shaderIdentifier(shaderIdentifier)
    , primType(primType)
    , apiSchema(apiSchema)
    {
    }
    static std::string asString(const LightTypeInfo& in)
    {
      return "{shaderIdentifier: " + in.shaderIdentifier + ", primType: " + in.primType +
             ", apiSchema: " + in.apiSchema + "}";
    }

    bool operator==(const LightTypeInfo& rhs) const
    {
      return shaderIdentifier == rhs.shaderIdentifier && primType == rhs.primType &&
             apiSchema == rhs.apiSchema;
    }
    bool operator!=(const LightTypeInfo& rhs) const
    {
      return shaderIdentifier != rhs.shaderIdentifier || primType != rhs.primType ||
             apiSchema != rhs.apiSchema;
    }
    std::string shaderIdentifier;
    std::string primType;
    std::string apiSchema;
  };

  USG_API const fdk::StringList& KindNames();

  USG_API const fdk::StringList& PurposeNames();

  // Returns a set of the all the registered Concrete Schema
  // Will load any associated plugin to discover the total set available
  // in the current USD session.
  USG_API const fdk::StringList& GetConcreteSchemaTypeNames();
  USG_API const fdk::StringList& GetAPISchemaTypeNames();
  USG_API const fdk::StringList& GetSchemaTfTypeNames();
  USG_API const fdk::StringList& GetLightShaderIdentifiers();
  USG_API std::vector<ShaderProperty> GetShaderInputProperties(const Token& identifier,
                                                               const Token& sourceType = Token());

  USG_API std::vector<usg::Attribute> GetDefaultAttributesForType(const usg::Token& typeName);
  USG_API std::vector<usg::Relationship> GetDefaultRelationshipsForType(const usg::Token& typeName);

  USG_API std::vector<usg::Attribute> GetDefaultAttributesForAPISchema(
    const usg::Token& schemaTypeName);
  USG_API std::vector<usg::Relationship> GetDefaultRelationshipsForAPISchema(
    const usg::Token& schemaTypeName);
  USG_API fdk::KeyValueMap GetShaderMetadata(const usg::Token& identifier,
                                             const Token& sourceType = Token());

  USG_API void StitchLayers(const usg::LayerRef &strongLayer, const usg::LayerRef &weakLayer);

  USG_API usg::PathArray MatchPattern(const std::string& pattern, const usg::StageRef& stage,
                                      std::string& error);
  
  // Deduplicates all time samples on all attributes on the layer.
  USG_API void RemoveDuplicateTimeSamples(const usg::Layer& layer,
                                          bool makeSingleSamplesStatic = true);

  USG_API ShaderNode GetLightShaderFromName(const usg::Token& lightShaderName,
                                            std::string_view target);

  USG_API const std::map<std::string, LightTypeInfo>& GetLightTypeInfoMap();
  USG_API const std::vector<std::pair<std::string, std::string>>& GetShaderTypeInfoList();

  // Returns the LightTypeInfo matching the prim and the details we can infer, otherwise a nullopt.
  USG_API std::optional<LightTypeInfo> GetLightTypeInfoFromPrim(const usg::Prim& prim);

  USG_API bool IsValidIdentifier(const std::string& in);
  USG_API std::string MakeValidIdentifier(const std::string& in);
}

#endif
