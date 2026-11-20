// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

// This is automatically-generated code.
// You may add custom code between the --(BEGIN CUSTOM CODE)-- and --(END CUSTOM CODE)-- markers.
// Code added anywhere else will be destroyed when the code is generated.

#ifndef USG_LUX_LUXTOKENS_H
#define USG_LUX_LUXTOKENS_H

#include "usg/api.h"
#include "usg/base/Token.h"

namespace usg {
namespace lux {
  struct USG_API LuxTokensDef {
    Token::Lazy angular{"angular"};
    Token::Lazy automatic{"automatic"};
    Token::Lazy consumeAndContinue{"consumeAndContinue"};
    Token::Lazy consumeAndHalt{"consumeAndHalt"};
    Token::Lazy cubeMapVerticalCross{"cubeMapVerticalCross"};
    Token::Lazy extent{"extent"};
    Token::Lazy filters{"filters"};
    Token::Lazy geometry{"geometry"};
    Token::Lazy guideRadius{"guideRadius"};
    Token::Lazy ignore{"ignore"};
    Token::Lazy independent{"independent"};
    Token::Lazy infoId{"info:id"};
    Token::Lazy infoImplementationSource{"info:implementationSource"};
    Token::Lazy inputsAngle{"inputs:angle"};
    Token::Lazy inputsColor{"inputs:color"};
    Token::Lazy inputsColorTemperature{"inputs:colorTemperature"};
    Token::Lazy inputsDiffuse{"inputs:diffuse"};
    Token::Lazy inputsEnableColorTemperature{"inputs:enableColorTemperature"};
    Token::Lazy inputsExposure{"inputs:exposure"};
    Token::Lazy inputsHeight{"inputs:height"};
    Token::Lazy inputsIntensity{"inputs:intensity"};
    Token::Lazy inputsLength{"inputs:length"};
    Token::Lazy inputsNormalize{"inputs:normalize"};
    Token::Lazy inputsRadius{"inputs:radius"};
    Token::Lazy inputsShadowColor{"inputs:shadow:color"};
    Token::Lazy inputsShadowDistance{"inputs:shadow:distance"};
    Token::Lazy inputsShadowEnable{"inputs:shadow:enable"};
    Token::Lazy inputsShadowFalloff{"inputs:shadow:falloff"};
    Token::Lazy inputsShadowFalloffGamma{"inputs:shadow:falloffGamma"};
    Token::Lazy inputsShapingConeAngle{"inputs:shaping:cone:angle"};
    Token::Lazy inputsShapingConeSoftness{"inputs:shaping:cone:softness"};
    Token::Lazy inputsShapingFocus{"inputs:shaping:focus"};
    Token::Lazy inputsShapingFocusTint{"inputs:shaping:focusTint"};
    Token::Lazy inputsShapingIesAngleScale{"inputs:shaping:ies:angleScale"};
    Token::Lazy inputsShapingIesFile{"inputs:shaping:ies:file"};
    Token::Lazy inputsShapingIesNormalize{"inputs:shaping:ies:normalize"};
    Token::Lazy inputsSpecular{"inputs:specular"};
    Token::Lazy inputsTextureFile{"inputs:texture:file"};
    Token::Lazy inputsTextureFormat{"inputs:texture:format"};
    Token::Lazy inputsWidth{"inputs:width"};
    Token::Lazy latlong{"latlong"};
    Token::Lazy lightFilters{"light:filters"};
    Token::Lazy lightMaterialSyncMode{"light:materialSyncMode"};
    Token::Lazy lightShaderId{"light:shaderId"};
    Token::Lazy lightFilterShaderId{"lightFilter:shaderId"};
    Token::Lazy lightList{"lightList"};
    Token::Lazy lightListCacheBehavior{"lightList:cacheBehavior"};
    Token::Lazy materialGlowTintsLight{"materialGlowTintsLight"};
    Token::Lazy mirroredBall{"mirroredBall"};
    Token::Lazy noMaterialResponse{"noMaterialResponse"};
    Token::Lazy portals{"portals"};
    Token::Lazy proxyPrim{"proxyPrim"};
    Token::Lazy purpose{"purpose"};
    Token::Lazy treatAsLine{"treatAsLine"};
    Token::Lazy treatAsPoint{"treatAsPoint"};
    Token::Lazy visibility{"visibility"};
    Token::Lazy xformOpOrder{"xformOpOrder"};
    struct {
      Token::Lazy BoundableLightBase{"BoundableLightBase"};
      Token::Lazy CylinderLight{"CylinderLight"};
      Token::Lazy DiskLight{"DiskLight"};
      Token::Lazy DistantLight{"DistantLight"};
      Token::Lazy DomeLight{"DomeLight"};
      Token::Lazy GeometryLight{"GeometryLight"};
      Token::Lazy LightAPI{"LightAPI"};
      Token::Lazy LightFilter{"LightFilter"};
      Token::Lazy LightListAPI{"LightListAPI"};
      Token::Lazy ListAPI{"ListAPI"};
      Token::Lazy MeshLightAPI{"MeshLightAPI"};
      Token::Lazy NonboundableLightBase{"NonboundableLightBase"};
      Token::Lazy PluginLight{"PluginLight"};
      Token::Lazy PluginLightFilter{"PluginLightFilter"};
      Token::Lazy PortalLight{"PortalLight"};
      Token::Lazy RectLight{"RectLight"};
      Token::Lazy ShadowAPI{"ShadowAPI"};
      Token::Lazy ShapingAPI{"ShapingAPI"};
      Token::Lazy SphereLight{"SphereLight"};
      Token::Lazy VolumeLightAPI{"VolumeLightAPI"};
      Token::Lazy MaterialXConfigAPI{"MaterialXConfigAPI"};
    } types;
    // --(BEGIN CUSTOM CODE)--
    struct SlrLightTokensDef {
      usg::Token::Lazy falloffType{"slr:falloff_type"};
      usg::Token::Lazy noFalloff{"noFalloff"};
      usg::Token::Lazy linearFalloff{"linearFalloff"};
      usg::Token::Lazy quadraticFalloff{"quadraticFalloff"};
      usg::Token::Lazy cubicFalloff{"cubicFalloff"};
    } slrLightTokens;

    // --(END CUSTOM CODE)--
  };

  extern USG_API LuxTokensDef LuxTokens;
}
}
#endif
