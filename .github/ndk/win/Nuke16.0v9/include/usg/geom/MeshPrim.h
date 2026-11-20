// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

// This is automatically-generated code.
// You may add custom code between the --(BEGIN CUSTOM CODE)-- and --(END CUSTOM CODE)-- markers.
// Code added anywhere else will be destroyed when the code is generated.

#ifndef USG_GEOM_MESHPRIM_H
#define USG_GEOM_MESHPRIM_H

#include "usg/api.h"

#include "usg/geom/GeomTokens.h"

#include "usg/geom/Prim.h"
#include "usg/base/ArrayTypes.h"
#include "usg/geom/PointBasedPrim.h"

namespace usg {

  // class MeshPrim
  //   inherits from PointBased, Gprim, Boundable, Xformable, Imageable
  //
  // Attributes:
  //     int[] cornerIndices
  //     float[] cornerSharpnesses
  //     int[] creaseIndices
  //     int[] creaseLengths
  //     float[] creaseSharpnesses
  //     token faceVaryingLinearInterpolation = "cornersPlus1" (allowed values: none, cornersOnly, cornersPlus1, cornersPlus2, boundaries, all)
  //     int[] faceVertexCounts
  //     int[] faceVertexIndices
  //     int[] holeIndices
  //     token interpolateBoundary = "edgeAndCorner" (allowed values: none, edgeOnly, edgeAndCorner)
  //     uniform token subdivisionScheme = "catmullClark" (allowed values: catmullClark, loop, bilinear, none)
  //     token triangleSubdivisionRule = "catmullClark" (allowed values: catmullClark, smooth)
  //
  // Inherited attributes:
  //   From PointBased:
  //     vector3f[] accelerations
  //     normal3f[] normals
  //     point3f[] points
  //     vector3f[] velocities
  //   From Gprim:
  //     uniform bool doubleSided
  //     uniform token orientation = "rightHanded" (allowed values: rightHanded, leftHanded)
  //     color3f[] primvars:displayColor
  //     float[] primvars:displayOpacity
  //   From Boundable:
  //     float3[] extent
  //   From Xformable:
  //     uniform token[] xformOpOrder
  //   From Imageable:
  //     rel proxyPrim
  //     uniform token purpose = "default" (allowed values: default, render, proxy, guide)
  //     token visibility = "inherited" (allowed values: inherited, invisible)

  class USG_API MeshPrim : public PointBasedPrim {
  public:
    MeshPrim();
    MeshPrim(const Prim& prim);
    ~MeshPrim();

    bool validate();
    static bool isValidPrim(const Prim& prim);
    static MeshPrim defineInLayer(const LayerRef& layer, const Path& path);
    static MeshPrim overrideInLayer(const LayerRef& layer, const Prim& prim);
    static MeshPrim getInStage(const StageRef& stage, const Path& path);

    Attribute createCornerIndicesAttr(const Value& defaultValue = Value());
    Attribute getCornerIndicesAttr() const;
    void setCornerIndices(const IntArray& value, fdk::TimeValue time = fdk::defaultTimeValue());
    IntArray getCornerIndices(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createCornerSharpnessesAttr(const Value& defaultValue = Value());
    Attribute getCornerSharpnessesAttr() const;
    void setCornerSharpnesses(const FloatArray& value, fdk::TimeValue time = fdk::defaultTimeValue());
    FloatArray getCornerSharpnesses(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createCreaseIndicesAttr(const Value& defaultValue = Value());
    Attribute getCreaseIndicesAttr() const;
    void setCreaseIndices(const IntArray& value, fdk::TimeValue time = fdk::defaultTimeValue());
    IntArray getCreaseIndices(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createCreaseLengthsAttr(const Value& defaultValue = Value());
    Attribute getCreaseLengthsAttr() const;
    void setCreaseLengths(const IntArray& value, fdk::TimeValue time = fdk::defaultTimeValue());
    IntArray getCreaseLengths(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createCreaseSharpnessesAttr(const Value& defaultValue = Value());
    Attribute getCreaseSharpnessesAttr() const;
    void setCreaseSharpnesses(const FloatArray& value, fdk::TimeValue time = fdk::defaultTimeValue());
    FloatArray getCreaseSharpnesses(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createFaceVaryingLinearInterpolationAttr(const Value& defaultValue = Value());
    Attribute getFaceVaryingLinearInterpolationAttr() const;
    void setFaceVaryingLinearInterpolation(const Token& value, fdk::TimeValue time = fdk::defaultTimeValue());
    Token getFaceVaryingLinearInterpolation(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createFaceVertexCountsAttr(const Value& defaultValue = Value());
    Attribute getFaceVertexCountsAttr() const;
    void setFaceVertexCounts(const IntArray& value, fdk::TimeValue time = fdk::defaultTimeValue());
    IntArray getFaceVertexCounts(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createFaceVertexIndicesAttr(const Value& defaultValue = Value());
    Attribute getFaceVertexIndicesAttr() const;
    void setFaceVertexIndices(const IntArray& value, fdk::TimeValue time = fdk::defaultTimeValue());
    IntArray getFaceVertexIndices(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createHoleIndicesAttr(const Value& defaultValue = Value());
    Attribute getHoleIndicesAttr() const;
    void setHoleIndices(const IntArray& value, fdk::TimeValue time = fdk::defaultTimeValue());
    IntArray getHoleIndices(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createInterpolateBoundaryAttr(const Value& defaultValue = Value());
    Attribute getInterpolateBoundaryAttr() const;
    void setInterpolateBoundary(const Token& value, fdk::TimeValue time = fdk::defaultTimeValue());
    Token getInterpolateBoundary(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createSubdivisionSchemeAttr(const Value& defaultValue = Value());
    Attribute getSubdivisionSchemeAttr() const;
    void setSubdivisionScheme(const Token& value, fdk::TimeValue time = fdk::defaultTimeValue());
    Token getSubdivisionScheme(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createTriangleSubdivisionRuleAttr(const Value& defaultValue = Value());
    Attribute getTriangleSubdivisionRuleAttr() const;
    void setTriangleSubdivisionRule(const Token& value, fdk::TimeValue time = fdk::defaultTimeValue());
    Token getTriangleSubdivisionRule(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    // --(BEGIN CUSTOM CODE)--
    //! Retrieves the facevert count from the faceVertexIndices attribute.
    size_t  getNumVerts() const;

    //! Retrieves the face count from the faceVertexCounts attribute.
    size_t  getNumFaces() const;

    /*! Build facevert counts and point indices lists, returning number of faces.
        For convenience - same as separately getting the facevert counts, indices,
        and orientation attributes.
        If \a winding_is_clockwise is provided the mesh's orientation attribute is
        used to set this value, and can then be used to reverse the winding of
        \a faceVertexIndices.
    */
    virtual size_t  getFaceVerts(const fdk::TimeValue& time,
                                 Int32Array&           faceVertexCounts,
                                 Int32Array&           faceVertexIndices,
                                 bool*                 winding_is_clockwise = nullptr) const;
    // --(END CUSTOM CODE)--
  };

}

#endif
