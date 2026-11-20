// Copyright (c) 2024 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef NDK_GEO_RENDER_SURFACETRAITS_H
#define NDK_GEO_RENDER_SURFACETRAITS_H

#include "ndk/api.h"
#include "fdk/math/Math.h"

namespace ndk {

  //! Traits flags are defined as uints vs. enums so they can be OR'd & AND'd together.
  using OpacityTraitsMask = uint16_t;

  // Predefined trait masks:
  static constexpr OpacityTraitsMask                opaqueSolid() { return 0x0000; } //!< 100% opaque, 100% presence, 0% transmission

  // Presence/opacity:
  static constexpr OpacityTraitsMask             presenceVaries() { return 0x0001; } //!< Presence may vary across surface
  static constexpr OpacityTraitsMask              opacityVaries() { return 0x0002; } //!< Opacity may vary across surface
  static constexpr OpacityTraitsMask     opacityCreatesPresence() { return 0x0004; } //!< Varying opacity is used for presence threshold
  static constexpr OpacityTraitsMask         transparencyTraits() { return 0x000f; } //!< All flags related to surface transparency

  // Surface interface handling:
  static constexpr OpacityTraitsMask  thinInterfaceTransmission() { return 0x0010; } //!< Thin surface, no refraction
  static constexpr OpacityTraitsMask       interiorTransmission() { return 0x0020; } //!< Light propagates into surface interior, possibly refracting
  static constexpr OpacityTraitsMask         transmissionTraits() { return 0x00f0; } //!< All flags related to surface transmission

  // Interior type - if no interior traits enabled an 'empty' interior is assumed:
  static constexpr OpacityTraitsMask      subsurfacePropagation() { return 0x0100; } //!< Solid interior, light propagates through possibly scattering
  static constexpr OpacityTraitsMask           subsurfaceVolume() { return 0x0200; } //!< Volume interior, light propagates through volume
  static constexpr OpacityTraitsMask             interiorTraits() { return 0x0f00; } //!< All flags related to surface interior

  // Blending - if no blending traits enabled a standard over/under is assumed:
  static constexpr OpacityTraitsMask     blendDoesNotUseOpacity() { return 0x1000; } //!< Blending does not require opacity (blend plus, max)
  static constexpr OpacityTraitsMask        cannotBlendToOpaque() { return 0x2000; } //!< Cannot produce 100% opaque after blending (blend min, max, etc)
  static constexpr OpacityTraitsMask             blendingTraits() { return 0xf000; } //!< All flags related to surface blending


  // Common convenience tests:
  inline bool  isTriviallyOpaque(OpacityTraitsMask mask)         { return ((mask & transparencyTraits()) == 0); }

  inline bool  doesPresenceVary(OpacityTraitsMask mask)          { return (mask & presenceVaries()); }
  inline bool  doesOpacityVary(OpacityTraitsMask mask)           { return (mask & opacityVaries()); }
  inline bool  doesOpacityCreatePresence(OpacityTraitsMask mask) { return ((mask & opacityCreatesPresence()) && doesOpacityVary(mask)); }

  inline bool  doesThinInterfaceTransmission(OpacityTraitsMask mask) { return (mask & thinInterfaceTransmission()); }
  inline bool  doesInteriorTransmission(OpacityTraitsMask mask)      { return (mask & interiorTransmission()); }

  inline bool  doesSubsurfacePropagation(OpacityTraitsMask mask)     { return (mask & subsurfacePropagation()); }
  inline bool  doesSubsurfaceVolume(OpacityTraitsMask mask)          { return (mask & subsurfaceVolume()); }

  inline bool  doesSurfaceBlend(OpacityTraitsMask mask)              { return ((mask & transparencyTraits()) && ((mask & blendingTraits()) == 0)); }
  inline bool  doesSurfaceBlendToFullyOpaque(OpacityTraitsMask mask) { return ((mask & cannotBlendToOpaque()) == 0); }
  inline bool  doesSurfaceBlendUseOpacity(OpacityTraitsMask mask)    { return ((mask & blendDoesNotUseOpacity()) == 0); }



} // namespace ndk

#endif
