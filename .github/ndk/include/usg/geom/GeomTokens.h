// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

// This is automatically-generated code.
// You may add custom code between the --(BEGIN CUSTOM CODE)-- and --(END CUSTOM CODE)-- markers.
// Code added anywhere else will be destroyed when the code is generated.

#ifndef USG_GEOM_GEOMTOKENS_H
#define USG_GEOM_GEOMTOKENS_H

#include "usg/api.h"
#include "usg/base/Token.h"

namespace usg {
  struct USG_API GeomTokensDef {
    Token::Lazy X{"X"};
    Token::Lazy Y{"Y"};
    Token::Lazy Z{"Z"};
    Token::Lazy accelerations{"accelerations"};
    Token::Lazy all{"all"};
    Token::Lazy angularVelocities{"angularVelocities"};
    Token::Lazy axis{"axis"};
    Token::Lazy basis{"basis"};
    Token::Lazy bezier{"bezier"};
    Token::Lazy bilinear{"bilinear"};
    Token::Lazy boundaries{"boundaries"};
    Token::Lazy bounds{"bounds"};
    Token::Lazy box{"box"};
    Token::Lazy bspline{"bspline"};
    Token::Lazy cards{"cards"};
    Token::Lazy catmullClark{"catmullClark"};
    Token::Lazy catmullRom{"catmullRom"};
    Token::Lazy clippingPlanes{"clippingPlanes"};
    Token::Lazy clippingRange{"clippingRange"};
    Token::Lazy closed{"closed"};
    Token::Lazy cornerIndices{"cornerIndices"};
    Token::Lazy cornerSharpnesses{"cornerSharpnesses"};
    Token::Lazy cornersOnly{"cornersOnly"};
    Token::Lazy cornersPlus1{"cornersPlus1"};
    Token::Lazy cornersPlus2{"cornersPlus2"};
    Token::Lazy creaseIndices{"creaseIndices"};
    Token::Lazy creaseLengths{"creaseLengths"};
    Token::Lazy creaseSharpnesses{"creaseSharpnesses"};
    Token::Lazy cross{"cross"};
    Token::Lazy cubic{"cubic"};
    Token::Lazy curveVertexCounts{"curveVertexCounts"};
    Token::Lazy curveIndices{"curveIndices"};
    Token::Lazy default_{"default"};
    Token::Lazy doubleSided{"doubleSided"};
    Token::Lazy edgeAndCorner{"edgeAndCorner"};
    Token::Lazy edgeOnly{"edgeOnly"};
    Token::Lazy elementType{"elementType"};
    Token::Lazy exposure{"exposure"};
    Token::Lazy extent{"extent"};
    Token::Lazy fStop{"fStop"};
    Token::Lazy face{"face"};
    Token::Lazy faceVaryingLinearInterpolation{"faceVaryingLinearInterpolation"};
    Token::Lazy faceVertexCounts{"faceVertexCounts"};
    Token::Lazy faceVertexIndices{"faceVertexIndices"};
    Token::Lazy familyName{"familyName"};
    Token::Lazy focalLength{"focalLength"};
    Token::Lazy focusDistance{"focusDistance"};
    Token::Lazy fromTexture{"fromTexture"};
    Token::Lazy guide{"guide"};
    Token::Lazy guideVisibility{"guideVisibility"};
    Token::Lazy height{"height"};
    Token::Lazy holeIndices{"holeIndices"};
    Token::Lazy horizontalAperture{"horizontalAperture"};
    Token::Lazy horizontalApertureOffset{"horizontalApertureOffset"};
    Token::Lazy ids{"ids"};
    Token::Lazy indices{"indices"};
    Token::Lazy inherited{"inherited"};
    Token::Lazy interpolateBoundary{"interpolateBoundary"};
    Token::Lazy invisible{"invisible"};
    Token::Lazy invisibleIds{"invisibleIds"};
    Token::Lazy knots{"knots"};
    Token::Lazy left{"left"};
    Token::Lazy leftHanded{"leftHanded"};
    Token::Lazy linear{"linear"};
    Token::Lazy loop{"loop"};
    Token::Lazy modelApplyDrawMode{"model:applyDrawMode"};
    Token::Lazy modelCardGeometry{"model:cardGeometry"};
    Token::Lazy modelCardTextureXNeg{"model:cardTextureXNeg"};
    Token::Lazy modelCardTextureXPos{"model:cardTextureXPos"};
    Token::Lazy modelCardTextureYNeg{"model:cardTextureYNeg"};
    Token::Lazy modelCardTextureYPos{"model:cardTextureYPos"};
    Token::Lazy modelCardTextureZNeg{"model:cardTextureZNeg"};
    Token::Lazy modelCardTextureZPos{"model:cardTextureZPos"};
    Token::Lazy modelDrawMode{"model:drawMode"};
    Token::Lazy modelDrawModeColor{"model:drawModeColor"};
    Token::Lazy mono{"mono"};
    Token::Lazy motionBlurScale{"motion:blurScale"};
    Token::Lazy motionNonlinearSampleCount{"motion:nonlinearSampleCount"};
    Token::Lazy motionVelocityScale{"motion:velocityScale"};
    Token::Lazy none{"none"};
    Token::Lazy nonperiodic{"nonperiodic"};
    Token::Lazy normals{"normals"};
    Token::Lazy open{"open"};
    Token::Lazy order{"order"};
    Token::Lazy orientation{"orientation"};
    Token::Lazy orientations{"orientations"};
    Token::Lazy origin{"origin"};
    Token::Lazy orthographic{"orthographic"};
    Token::Lazy overlay{"overlay"};
    Token::Lazy periodic{"periodic"};
    Token::Lazy perspective{"perspective"};
    Token::Lazy pinned{"pinned"};
    Token::Lazy pointWeights{"pointWeights"};
    Token::Lazy points{"points"};
    Token::Lazy positions{"positions"};
    Token::Lazy primvarsDisplayColor{"primvars:displayColor"};
    Token::Lazy primvarsDisplayOpacity{"primvars:displayOpacity"};
    Token::Lazy primvarsNormals{"primvars:normals"};
    Token::Lazy primvarsSt{"primvars:st"};
    Token::Lazy primvarsSt_0{"primvars:st_0"};
    Token::Lazy primvarsSt_1{"primvars:st_1"};
    Token::Lazy primvarsStw{"primvars:stw"};
    Token::Lazy primvarsStw_0{"primvars:stw_0"};
    Token::Lazy primvarsStw_1{"primvars:stw_1"};
    Token::Lazy primvarsUv{"primvars:uv"};
    Token::Lazy primvarsWidths{"primvars:widths"};
    Token::Lazy projection{"projection"};
    Token::Lazy protoIndices{"protoIndices"};
    Token::Lazy prototypes{"prototypes"};
    Token::Lazy proxy{"proxy"};
    Token::Lazy proxyPrim{"proxyPrim"};
    Token::Lazy proxyVisibility{"proxyVisibility"};
    Token::Lazy purpose{"purpose"};
    Token::Lazy radius{"radius"};
    Token::Lazy ranges{"ranges"};
    Token::Lazy render{"render"};
    Token::Lazy renderVisibility{"renderVisibility"};
    Token::Lazy right{"right"};
    Token::Lazy rightHanded{"rightHanded"};
    Token::Lazy scales{"scales"};
    Token::Lazy shutterClose{"shutter:close"};
    Token::Lazy shutterOpen{"shutter:open"};
    Token::Lazy size{"size"};
    Token::Lazy smooth{"smooth"};
    Token::Lazy stereoRole{"stereoRole"};
    Token::Lazy subdivisionScheme{"subdivisionScheme"};
    Token::Lazy subdivisionTags{"subdivisionTags"};
    Token::Lazy topology{"topology"};
    Token::Lazy tangents{"tangents"};
    Token::Lazy triangleSubdivisionRule{"triangleSubdivisionRule"};
    Token::Lazy trimCurveCounts{"trimCurve:counts"};
    Token::Lazy trimCurveKnots{"trimCurve:knots"};
    Token::Lazy trimCurveOrders{"trimCurve:orders"};
    Token::Lazy trimCurvePoints{"trimCurve:points"};
    Token::Lazy trimCurveRanges{"trimCurve:ranges"};
    Token::Lazy trimCurveVertexCounts{"trimCurve:vertexCounts"};
    Token::Lazy type{"type"};
    Token::Lazy uForm{"uForm"};
    Token::Lazy uKnots{"uKnots"};
    Token::Lazy uOrder{"uOrder"};
    Token::Lazy uRange{"uRange"};
    Token::Lazy uVertexCount{"uVertexCount"};
    Token::Lazy vForm{"vForm"};
    Token::Lazy vKnots{"vKnots"};
    Token::Lazy vOrder{"vOrder"};
    Token::Lazy vRange{"vRange"};
    Token::Lazy vVertexCount{"vVertexCount"};
    Token::Lazy velocities{"velocities"};
    Token::Lazy verticalAperture{"verticalAperture"};
    Token::Lazy verticalApertureOffset{"verticalApertureOffset"};
    Token::Lazy visibility{"visibility"};
    Token::Lazy visible{"visible"};
    Token::Lazy widths{"widths"};
    Token::Lazy wrap{"wrap"};
    Token::Lazy xformOpOrder{"xformOpOrder"};
    struct {
      Token::Lazy BasisCurves{"BasisCurves"};
      Token::Lazy Boundable{"Boundable"};
      Token::Lazy Camera{"Camera"};
      Token::Lazy Capsule{"Capsule"};
      Token::Lazy Cone{"Cone"};
      Token::Lazy Cube{"Cube"};
      Token::Lazy Curves{"Curves"};
      Token::Lazy Cylinder{"Cylinder"};
      Token::Lazy GeomModelAPI{"GeomModelAPI"};
      Token::Lazy GeomSubset{"GeomSubset"};
      Token::Lazy Gprim{"Gprim"};
      Token::Lazy HermiteCurves{"HermiteCurves"};
      Token::Lazy Imageable{"Imageable"};
      Token::Lazy Mesh{"Mesh"};
      Token::Lazy MotionAPI{"MotionAPI"};
      Token::Lazy NurbsCurves{"NurbsCurves"};
      Token::Lazy NurbsPatch{"NurbsPatch"};
      Token::Lazy PointBased{"PointBased"};
      Token::Lazy PointInstancer{"PointInstancer"};
      Token::Lazy Points{"Points"};
      Token::Lazy PrimvarsAPI{"PrimvarsAPI"};
      Token::Lazy Scope{"Scope"};
      Token::Lazy Sphere{"Sphere"};
      Token::Lazy VisibilityAPI{"VisibilityAPI"};
      Token::Lazy Xform{"Xform"};
      Token::Lazy XformCommonAPI{"XformCommonAPI"};
      Token::Lazy Xformable{"Xformable"};
      Token::Lazy RenderPass{"RenderPass"};
      Token::Lazy RenderProduct{"RenderProduct"};
      Token::Lazy RenderSettings{"RenderSettings"};
      Token::Lazy RenderSettingsBase{"RenderSettingsBase"};
      Token::Lazy RenderVar{"RenderVar"};
    } types;
    // --(BEGIN CUSTOM CODE)--
    Token::Lazy colorAov{"color"};
    Token::Lazy depthAov{"depth"};
    Token::Lazy cameraDepthAov{"cameraDepth"};
    //
    Token::Lazy primIdAov{"primId"};
    Token::Lazy instanceIdAov{"instanceId"};
    Token::Lazy elementIdAov{"elementId"};
    Token::Lazy edgeIdAov{"edgeId"};
    Token::Lazy pointIdAov{"pointId"};
    //
    Token::Lazy PeyeAov{"Peye"};
    Token::Lazy NeyeAov{"Neye"};
    Token::Lazy patchCoordAov{"patchCoord"};
    Token::Lazy primitiveParamAov{"primitiveParam"};
    Token::Lazy normalAov{"normal"};

    // Render/shading
    Token::Lazy usgSchema{"UsgSchema"};
    Token::Lazy usgShader{"UsgShader"};

    Token::Lazy universalRenderContext{""};
    Token::Lazy targetRenderers{"targetRenderers"};
    Token::Lazy slrRenderContext{"slr"};

    Token::Lazy universalSourceType{""};
    Token::Lazy glslfxSourceType{"glslfx"};
    Token::Lazy customGlslfxSourceType{"customGlslfx"};
    Token::Lazy slrSourceType{"slr"};

    // Material families
    Token::Lazy mtlxMaterialFamily{"mtlx"};

    // Property connectivity
    Token::Lazy connectability{"connectability"};
    struct {
      Token::Lazy full{"full"};
      Token::Lazy interfaceOnly{"interfaceOnly"};
    } connectivity;

    // Attribute interpolation
    Token::Lazy interpolation{"interpolation"};

    Token::Lazy constant{"constant"};
    Token::Lazy uniform{"uniform"};
    Token::Lazy varying{"varying"};
    Token::Lazy vertex{"vertex"};
    Token::Lazy faceVarying{"faceVarying"};

    Token::Lazy st{"st"};
    Token::Lazy stw{"stw"};
    Token::Lazy color{"color"};
    Token::Lazy opacity{"opacity"};

    struct {
      Token::Lazy binding{"material:binding"};
      Token::Lazy bindingPreview{"material:binding:preview"};
      Token::Lazy bindingFull{"material:binding:full"};
    } material;

    struct {
      Token::Lazy weakerThanDescendants{"weakerThanDescendants"};
      Token::Lazy strongerThanDescendants{"strongerThanDescendants"};
    } bindingStrength;

    // USG-specific tokens
    struct {
      Token::Lazy owner{"usg:owner"};
      Token::Lazy modified{"usg:modified"};
      Token::Lazy displayMaterials{"usg:displayMaterials"};
      Token::Lazy localProp{"usg:localProp"};
      Token::Lazy created{"usg:created"};
      Token::Lazy parent{"usg:parent"};
      Token::Lazy selectable{"usg:selectable"};
    } usg;

    // Render settings attribute tokens
    Token::Lazy aspectRatioConformPolicy{"aspectRatioConformPolicy"};
    Token::Lazy camera{"camera"};
    Token::Lazy dataWindowNDC{"dataWindowNDC"};
    Token::Lazy disableDepthOfField{"disableDepthOfField"};
    Token::Lazy disableMotionBlur{"disableMotionBlur"};
    Token::Lazy instantaneousShutter{"instantaneousShutter"};
    Token::Lazy pixelAspectRatio{"pixelAspectRatio"};
    Token::Lazy resolution{"resolution"};
    Token::Lazy includedPurposes{"includedPurposes"};
    Token::Lazy materialBindingPurposes{"materialBindingPurposes"};
    Token::Lazy products{"products"};
    Token::Lazy renderingColorSpace{"renderingColorSpace"};
    Token::Lazy dataType{"dataType"};
    Token::Lazy sourceName{"sourceName"};
    Token::Lazy sourceType{"sourceType"};
    Token::Lazy orderedVars{"orderedVars"};
    Token::Lazy productName{"productName"};
    Token::Lazy productType{"productType"};
    Token::Lazy command{"command"};
    Token::Lazy fileName{"fileName"};
    Token::Lazy inputPasses{"inputPasses"};
    Token::Lazy passType{"passType"};
    Token::Lazy renderSource{"renderSource"};

    // Render collection tokens
    struct {
      Token::Lazy cameraVisibility{"collection:cameraVisibility"};
      Token::Lazy cameraVisibilityExcludes{"collection:cameraVisibility:excludes"};
      Token::Lazy cameraVisibilityExpansionRule{"collection:cameraVisibility:expansionRule"};
      Token::Lazy cameraVisibilityIncludeRoot{"collection:cameraVisibility:includeRoot"};
      Token::Lazy cameraVisibilityIncludes{"collection:cameraVisibility:includes"};
      Token::Lazy cameraVisibilityMembershipExpression{"collection:cameraVisibility:membershipExpression"};
      Token::Lazy cameraVisibilityMode{"collection:cameraVisibility:mode"};
      Token::Lazy matte{"collection:matte"};
      Token::Lazy matteExcludes{"collection:matte:excludes"};
      Token::Lazy matteExpansionRule{"collection:matte:expansionRule"};
      Token::Lazy matteIncludeRoot{"collection:matte:includeRoot"};
      Token::Lazy matteIncludes{"collection:matte:includes"};
      Token::Lazy matteMembershipExpression{"collection:matte:membershipExpression"};
      Token::Lazy matteMode{"collection:matte:mode"};
      Token::Lazy prune{"collection:prune"};
      Token::Lazy pruneExcludes{"collection:prune:excludes"};
      Token::Lazy pruneExpansionRule{"collection:prune:expansionRule"};
      Token::Lazy pruneIncludeRoot{"collection:prune:includeRoot"};
      Token::Lazy pruneIncludes{"collection:prune:includes"};
      Token::Lazy pruneMembershipExpression{"collection:prune:membershipExpression"};
      Token::Lazy pruneMode{"collection:prune:mode"};
      Token::Lazy renderVisibility{"collection:renderVisibility"};
      Token::Lazy renderVisibilityExcludes{"collection:renderVisibility:excludes"};
      Token::Lazy renderVisibilityExpansionRule{"collection:renderVisibility:expansionRule"};
      Token::Lazy renderVisibilityIncludeRoot{"collection:renderVisibility:includeRoot"};
      Token::Lazy renderVisibilityIncludes{"collection:renderVisibility:includes"};
      Token::Lazy renderVisibilityMembershipExpression{"collection:renderVisibility:membershipExpression"};
      Token::Lazy renderVisibilityMode{"collection:renderVisibility:mode"};
    } renderCollection;
    // --(END CUSTOM CODE)--
  };

  extern USG_API GeomTokensDef GeomTokens;
}
#endif
