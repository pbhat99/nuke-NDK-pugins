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

      //! This can be used to turn on/off error reporting during a call to match.
      //! By default, errors are reported on calls to match.
      virtual void reportErrorsDuringMatch(bool report) = 0;

      struct ExpressionType
      {
        std::string label;
        std::string expression;
      };

      //! Sets selectable expression types that can be picked from a drop down for this knob.
      //! Note that a expression type with the label "Custom" is always present and does not
      //! need to be specified here. It will be ignored if given in the types vector passed in.
      virtual void setExpressionTypes(const std::vector<ExpressionType>& types) = 0;

      //! Gets the selectable expression types that can be picked from a drop down for this knob.
      //! Note that a expression type with the label "Custom" is always the last element here.
      virtual const std::vector<ExpressionType>& getExpressionTypes() = 0;

      //! Sets the path mask to replace '{default}' with when no paths have
      //! been injected from an upstream node. The description will be used for UI purposes.
      //! By default the mask is  "/*" and the description is "All Roots".
      virtual void setDefaultFallbackMask(const std::string& mask, const std::string& description) = 0;

      //! Sets a predicate that is used during the stage traversal for matching.
      virtual void setPredicate(const usg::Prim::FlagsPredicate& predicate) = 0;
    };

    //! Default expression types for PathExpression_KnobI.
    const std::vector<PathExpression_KnobI::ExpressionType> defaultExpressionTypes = {
      {"{default}", "{default}"},
      {"Last modified", "{lastmodified}" },
      {"All roots", "/*" },
      {"All Cameras", "{isa:Camera}"},
      {"All Lights", "{hasAPI:LightAPI}"},
      {"All Meshes", "{isa:Mesh}"},
      {"Custom", ""}
    };

    //! Default expression types for PathExpression_KnobI with materials.
    const std::vector<PathExpression_KnobI::ExpressionType> expressionTypesMaterials = {
      {"{default}", "{default}"},
      {"Last modified", "{lastmodified}" },
      {"All roots", "/*" },
      {"All Cameras", "{isa:Camera}"},
      {"All Lights", "{hasAPI:LightAPI}"},
      {"All Meshes", "{isa:Mesh}"},
      {"All Materials", "{isa:Material}"},
      {"Custom", ""}
    };

    //! Default expression types for PathExpression_KnobI with custom only.
    const std::vector<PathExpression_KnobI::ExpressionType> expressionTypesDefaultAndCustomOnly = {
      {"{default}", "{default}"},
      {"Custom", ""}
    };

    const std::vector<PathExpression_KnobI::ExpressionType> expressionTypesRootsAndMeshesOnly = {
      {"All roots", "/*" },
      {"All Meshes", "{isa:Mesh}"},
      {"Custom", ""}
    };

    //! Tooltip for the PathExpression knob.
    constexpr const char* kPathExpressionTooltip =
    "Patterns to match prim names to using standard glob-style wildcards '*', '?'. "
    "There can be multiple mask patterns separated by spaces."
    "<ul>"
    "<li>Default - selects the default prim(s) for the node. "
         "The default will be the injected mask prim paths if there is an upstream node with this. "
         "Otherwise the default mask for this node type is evaluated.</li>"
    "<li>All Roots - selects the top level Xform(s) in your scene graph</li>"
    "<li>Last modified - selects whatever the last node modified</li>"
    "<li>All Cameras - selects all Cameras in your scene graph</li>"
    "<li>All lights - selects all Lights in your scene graph</li>"
    "<li>All Meshes - selects all meshes in your scene graph</li>"
    "<li>Custom - sets a custom selection of prims that exist in your scene graph</li>"
    "</ul>";

    //! Tooltip for the PathExpression knob with materials.
    constexpr const char* kPathExpressionTooltipMaterials =
    "Patterns to match prim names to using standard glob-style wildcards '*', '?'. "
    "There can be multiple mask patterns separated by spaces."
    "<ul>"
    "<li>Default - selects the default prim(s) for the node. "
         "The default will be the injected mask prim paths if there is an upstream node with this. "
         "Otherwise the default mask for this node type is evaluated.</li>"
    "<li>All Roots - selects the top level Xform(s) in your scene graph</li>"
    "<li>Last modified - selects whatever the last node modified</li>"
    "<li>All Cameras - selects all Cameras in your scene graph</li>"
    "<li>All lights - selects all Lights in your scene graph</li>"
    "<li>All Meshes - selects all meshes in your scene graph</li>"
    "<li>All Materials - selects all materials in your scene graph</li>"
    "<li>Custom - sets a custom selection of prims that exist in your scene graph</li>"
    "</ul>";

    //! Tooltip for the PathExpression knob with materials.
    constexpr const char* kPathExpressionTooltipRootsAndMeshesOnly = 
    "Patterns to match prim names to using standard glob-style wildcards '*', '?'. "
    "There can be multiple mask patterns separated by spaces."
    "<ul>"
    "<li>All Roots - selects the top level Xform(s) in your scene graph</li>"
    "<li>All Meshes - selects all meshes in your scene graph</li>"
    "<li>Custom - sets a custom selection of prims that exist in your scene graph</li>"
    "</ul>";
  }
}

#endif
