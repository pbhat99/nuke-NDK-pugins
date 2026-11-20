// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef SLR_PRIMINFO_H
#define SLR_PRIMINFO_H

#include "DDImage/DDImage_API.h"

#include "fdk/math/types.h" // for Mat4dList

#include "usg/geom/Path.h"

#include <vector>

namespace slr
{
  class SlrSurfaceMaterial;
  class SlrLightMaterial;
  class SlrLightShader;

  //------------------------------------------------------------------


  /*! This structure represents a single usg::Prim acting as a renderable surface.
  */
  class DDImage_API SlrPrimInfo
  {
  public:
    // Flags defined as uints vs. enums so they can be OR'd & AND'd together:
    using PrimFlag = uint32_t;
    static constexpr PrimFlag        animatingFlag() { return 0x00000001; } //!< Does the object animate its xform or shape?
    static constexpr PrimFlag  visibleToCameraFlag() { return 0x00000002; } //!< Should the object be visible to camera?
    static constexpr PrimFlag  reversedWindingFlag() { return 0x00000004; } //!< Reversed face winding order (CW - clockwise)
    static constexpr PrimFlag      doubleSidedFlag() { return 0x00000008; } //!< Surface should shade backside faces
    static constexpr PrimFlag      castsShadowFlag() { return 0x00000010; } //!< Does the object cast a shadow?
    static constexpr PrimFlag   receivesShadowFlag() { return 0x00000020; } //!< Does the object get shadowed by lights?

  public:
    uint32_t              renderableIndex;    //!< Index of object in renderable_prims in SlrEngineContext
    usg::Path             primPath;           //!< Prim path in Stage (prototype path if an instance)
    int32_t               primAdapter;        //!< To drive render primitive conversion
    int32_t               facesetBaseIndex;   //!< Faceset base prim in renderable_prims in SlrEngineContext
    int32_t               facesetIndex;       //!< Faceset index if this prim is one, otherwise -1
    int32_t               instancerIndex;     //!< Instancer index in instancer_prims in SlrEngineContext
    int32_t               instanceIndex;      //!< Instance index if this prim is one, otherwise -1.
    usg::Path             materialPath;       //!< Material binding path in Stage
    SlrSurfaceMaterial*   material;           //!< Render material (stored in SlrEngineContext)
    uint32_t              flags;              //!< Various surface quality flags


    SlrPrimInfo();

    bool         animates() const { return (flags &       animatingFlag()); } //!< Does the object animate its xform or shape?
    bool  visibleToCamera() const { return (flags & visibleToCameraFlag()); } //!< Should the object be visible to camera?
    bool  reversedWinding() const { return (flags & reversedWindingFlag()); } //!< Reversed face winding order (CW - clockwise)
    bool      doubleSided() const { return (flags &     doubleSidedFlag()); } //!< Surface should shade backside faces
    bool      castsShadow() const { return (flags &     castsShadowFlag()); } //!< Does the object cast a shadow?
    bool   receivesShadow() const { return (flags &  receivesShadowFlag()); } //!< Does the object get shadowed by lights?


    friend DDImage_API std::ostream& operator << (std::ostream&, const SlrPrimInfo&);
  };

  using SlrPrimInfoList = std::vector<SlrPrimInfo>;


  //------------------------------------------------------------------


  /*! This structure represents a single usg::Prim acting as a light source.
  */
  class DDImage_API SlrLightInfo
  {
  public:
    // Flags defined as uints vs. enums so they can be OR'd & AND'd together:
    using LightFlag = uint32_t;
    static constexpr LightFlag    animatingFlag() { return 0x00000001; } //!< Does the light animate its xform or shape?
    static constexpr LightFlag  castShadowsFlag() { return 0x00000002; } //!< Does the light cast a shadow?

  public:
    uint32_t          renderableLightIndex;   //!< Index of the light prim in the SlrLightInfoList
    usg::Path         primPath;               //!< Prim path in Stage
    usg::Path         materialPath;           //!< Material binding path in Stage (for projections)
    SlrLightMaterial* material;               //!< Light material (stored in SlrEngineContext)
    uint32_t          flags;                  //!< Various light quality flags


    SlrLightInfo();

    //! Convenience to get light shader from material. Warning, does no null testing!
    SlrLightShader*  lightShader() const;

    bool     animates() const { return (flags &   animatingFlag()); } //!< Does the light animate its xform or shape?
    bool  castShadows() const { return (flags & castShadowsFlag()); } //!< Does the light cast a shadow?


    friend DDImage_API std::ostream& operator << (std::ostream&, const SlrLightInfo&);
  };

  using SlrLightInfoList = std::vector<SlrLightInfo>;


} // namespace slr

#endif
