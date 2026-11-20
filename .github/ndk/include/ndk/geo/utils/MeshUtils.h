// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef NDK_GEO_UTILS_MESHUTILS_H
#define NDK_GEO_UTILS_MESHUTILS_H

#include "ndk/api.h"
#include "ndk/geo/utils/MeshTopology.h"

#include "fdk/math/Vec3.h"
#include "fdk/math/Vec4.h"
#include "fdk/math/Mat4.h"

#include "fdk/base/StringTypes.h"
#include "fdk/base/Time.h"

#include "usg/base/ArrayTypes.h"
#include "usg/geom/Primvar.h"

#include <functional>

namespace ndk {

  class MeshSample;

  //--------------------------------------------------------------------------------------------------

  /*!
  */
  NDK_API bool
  calcPointNormals(size_t            nPoints,
                   const fdk::Vec3f* points,
                   size_t            nVerts,
                   const int32_t*    faceVertPointIndices,
                   size_t            nFaces,
                   const int32_t*    nVertsPerFace,
                   bool              all_tris,
                   bool              all_quads,
                   usg::Vec3fArray&  point_normals,
                   bool              ccwWindingOrder);

  /*!
  */
  NDK_API bool
  calcVertexNormals(size_t            nPoints,
                    const fdk::Vec3f* points,
                    size_t            nVerts,
                    const int32_t*    faceVertPointIndices,
                    size_t            nFaces,
                    const int32_t*    nVertsPerFace,
                    float             threshold,
                    usg::Vec3fArray&  vertex_normals);

  //! Builds normals for the current VertexBuffers state.
  //NDK_API bool calcMeshSampleNormals(MeshSample& sample);

  //! Get position on a face in a mesh for the given UV coordinates
  NDK_API fdk::Vec3f
  getPosFromImplicitUV(const ndk::MeshTopology& mesh, int faceIndex, const fdk::Vec2f& uv);

  //--------------------------------------------------------------------------------------------------

  /*! Reverse the vertex winding of all the values in the \a faceVertexValues array.
  */
  template <typename T>
  void
  reverseFaceWinding(size_t          nFaces,
                     const int32_t*  faceVertexCounts,
                     size_t          nVerts,
                     std::vector<T>& faceVertexValues);

  //! Convert primvars of various interpolations to face-vertex interpolation
  template<typename T>
  void
  convertPrimvarToFaceVertRate(const usg::Vec3fArray& points,
                               const usg::Int32Array& faceVertexCounts,
                               const usg::Int32Array& faceVertexIndices,
                               bool                   isClockwiseWinding,
                               const fdk::TimeValue&  time,
                               usg::Primvar&          primvar,
                               usg::Array<T>&         flattenedValues);

  //--------------------------------------------------------------------------------------------------

#if 0
// TODO: do we need these? They are not used in any Nuke code that I can find

  //! Corner pinning: map 0,0,1,1 square to the four corners (anticlockwise from bottom left)
  NDK_API Mat4f
  mapUnitSquareToQuad(const fdk::Vec2f& v0,
                      const fdk::Vec2f& v1,
                      const fdk::Vec2f& v2,
                      const fdk::Vec2f& v3);

  //! Corner pinning: map the four corners (anticlockwise from bottom left) to 0,0,1,1 square.
  NDK_API Mat4f
  mapQuadToUnitSquare(const fdk::Vec2f& v0,
                      const fdk::Vec2f& v1,
                      const fdk::Vec2f& v2,
                      const fdk::Vec2f& v3);
#endif

  //--------------------------------------------------------------------------------------------------

  /* Convenience function for building common mesh types.
     Assumes MeshSample is partially or fully initialized so only the enabled
     target attribs are modified.
     Quads are build with CCW(counter-clockwise, right-handed) winding order.
  */
  NDK_API void
  buildRectQuadMesh(MeshSample&       sample,
                    const fdk::Vec3f& x0y0,
                    const fdk::Vec3f& x1y0,
                    const fdk::Vec3f& x1y1,
                    const fdk::Vec3f& x0y1,
                    uint32_t          rows,
                    uint32_t          columns,
                    bool              buildFaces,
                    bool              buildPoints = false,
                    bool              buildNormals = false,
                    bool              buildUvs = false);

  //! Build a cube mesh. If \a sharedVertices is true, edge vertices are shared between faces
  NDK_API void
  buildCube(MeshSample&       sample,
            const fdk::Box3d& corners,
            uint32_t          rows,
            uint32_t          columns,
            bool              buildFaces,
            bool              buildPoints = false,
            bool              buildNormals = false,
            bool              buildUvs = false,
            bool              sharedVertices = true);


  //! Build a revolve grid faces points locations.
  NDK_API void
  buildRevolveMeshPoints(MeshSample&               sample,
                         uint32_t                  rows,
                         uint32_t                  columns,
                         std::function<float(int)> rowRadius,
                         std::function<float(int)> rowHeight,
                         const fdk::Vec3f&         axisScales,
                         bool                      closeTop,
                         bool                      closeBottom,
                         bool                      sharedVertices = false);

  NDK_API void
  buildRevolveMeshUVs(MeshSample& sample,
                      uint32_t    rows,
                      uint32_t    columns,
                      float       uExtent,
                      float       vExtent,
                      bool        closeTop,
                      bool        closeBottom,
                      bool        uvTopCap,
                      bool        uvBottomCap);
  NDK_API void
  buildCylinderMeshNormals(MeshSample& sample,
                           uint32_t    rows,
                           uint32_t    columns,
                           const fdk::Vec3f& axisScales,
                           bool        closeTop = false,
                           bool        closeBottom = false);

  NDK_API void
  buildSphereMeshNormals(MeshSample& sample,
                         uint32_t    rows,
                         uint32_t    columns,
                         const fdk::Vec3f& axisScales,
                         bool        closeTop = false,
                         bool        closeBottom = false);



  //--------------------------------------------------------------------------------------------------


  /*! Mesh snapshot in time.
  */
  class NDK_API MeshSample : public MeshTopology
  {
  public:
    fdk::TimeValue   time;               //!< Sample time
    int32_t          id_index;           //!< ID index (arbitrary ID, object index, etc)
    fdk::Box3f       bbox;               //!< Derived local bbox, should surround point locations!
    fdk::Mat4d       xform;              //!< Local transform
    fdk::Mat4d       ixform;             //!< Pre-calc'd inverse local transform
    bool             xform_is_identity;  //!< For quick testing

    // Mesh info:
    bool        all_tris;           //!< Are verts part of an all-tri mesh? Don't need a face list if so.
    bool        all_quads;          //!< Are verts part of an all-quad mesh? Don't need a face list if so.
    bool        cw_winding;         //!< Are mesh face verts in CW(clockwise, left-handed) winding order?

    // Face-vert rate attributes:
    // These are usually stored in Nuke-natural CCW winding order (cw_winding=false):
    // TODO: make these a list of fvar arrays - need that channel/layer attrib class!
    usg::Vec2fArray  facevert_uvs;              //!< Per facevert texture coord (no perspective!)
    usg::FloatArray  facevert_uv_persp_w;       //!< Per facevert texture perspective coord (w)
    usg::Vec3fArray  facevert_normals;          //!< Per facevert normal
    usg::Vec3fArray  facevert_colors;           //!< Per facevert color
    usg::FloatArray  facevert_opacities;        //!< Per facevert opacity

    // Point-rate attributes:
    usg::Vec3fArray  point_velocities;          //!< Point velocity (TODO: how is this defined temporally (to or from point)?)
    usg::Vec3fArray  point_accelerations;       //!< Point acceleration (TODO: how is this defined temporally (to or from point)?)

    // Subd-specific data:
    usg::Token       subd_scheme;               //!< Name of subdivision scheme
    uint32_t         subd_level;                //!< Current subd level. 0=cage, 1=1st uniform refinement applied
    usg::Int32Array  face_hole_indices;         //!< List of face indices to interpret as mesh holes
    usg::Int32Array  corner_point_indices;      //!< List of point indices to interpret as mesh corners
    usg::FloatArray  corner_point_weights;      //!< Per corner-point sharpness weight
    usg::Int32Array  crease_edge_point_indices; //!< List of point indices to interpret as crease edges(2 point indices per edge)
    usg::FloatArray  crease_edge_weights;       //!< Per crease-edge sharpness weight
    usg::Int32Array  crease_edges_per_line;     //!< Per crease-line edge counts


  public:
    //!
    MeshSample();

    //! Constructs the sample from a MeshPrim at a specific time and updates
    //! just the face topology info - ie the faceVertexCounts and faceVertexIndices
    //! arrays, face winding order and the sample's time.
    //! Use defaultTimeValue() for the time to get the Mesh's default values.
    MeshSample(const usg::MeshPrim&  prim,
               const fdk::TimeValue& time = fdk::defaultTimeValue());

    //! Constructs the sample from a MeshPrim at a specific time.
    //! Use defaultTimeValue() for the time to get the Mesh's default values.
    MeshSample(const usg::MeshPrim&  prim,
               const fdk::TimeValue& time,
               int32_t               idIndex = 0,
               int32_t               targetSubdLevel = 0,
               bool                  getUvs       = false,
               bool                  getNormals   = false,
               bool                  getOpacities = false,
               bool                  getColors    = false,
               bool                  getVelocities    = false,
               bool                  getAccelerations = false,
               const                 usg::Token& uvAttrName = usg::Token());

    //! Samples the MeshPrim at a specific time.
    //! Use defaultTimeValue() for the time to get the Mesh's default values.
    //! Return true on success.
    bool sampleMeshPrim(const usg::MeshPrim&  prim,
                        const fdk::TimeValue& time,
                        int32_t               idIndex,
                        int32_t               targetSubdLevel,
                        bool                  getUvs,
                        bool                  getNormals,
                        bool                  getOpacities,
                        bool                  getColors,
                        bool                  getVelocities,
                        bool                  getAccelerations,
                        const                 usg::Token& uvAttrName = usg::Token());

    //! Samples the MeshPrim face topology at a specific time and updates
    //! the faceVertexCounts and faceVertexIndices arrays, winding order,
    //! and the sample's time.
    //! Note this does not affect any other facevert arrays which may not
    //! longer be correctly sized.
    //! Return true if the face/vert count is non-zero.
    bool sampleMeshPrimFaceTopology(const usg::MeshPrim&  prim,
                                    const fdk::TimeValue& time = fdk::defaultTimeValue());

    //! Reserve space in the faceVertexCounts and faceVertexIndices space.
    void reserveFaceVertSpace(size_t nFaces, size_t nVerts);

    //!
    size_t  nFaces()  const { return faceVertexCounts.size(); }
    size_t  nVerts()  const { return faceVertexIndices.size(); }
    size_t  nPoints() const { return points.size(); }

    //!
    const int32_t*     vertsPerFace()         const { return faceVertexCounts.data(); }
    const int32_t*     faceVertPointIndices() const { return faceVertexIndices.data(); }
    const fdk::Vec3f*  pointLocations()       const { return points.data(); }

    //! Convenience to check if this is a polygon mesh or subd mesh.
    bool isSubdMesh() const { return (!subd_scheme.empty() && subd_scheme != usg::GeomTokens.none); }

    //! Convenience functions to add a single triangle or quad face.
    void   addTri(int32_t v0, int32_t v1, int32_t v2);
    void  addQuad(int32_t v0, int32_t v1, int32_t v2, int32_t v3);


  public:
    //! Set MeshPrim face topology properties from the MeshSample values,
    //! at the time of the sample.
    bool setMeshPrimFaceTopology(usg::MeshPrim& prim);

    //! Set MeshPrim points property from the MeshSample values,
    //! at the time of the sample, optionally building the bounds box
    //! from the points values.
    bool setMeshPrimPointsAndBounds(usg::MeshPrim& prim,
                                    bool           buildBoundsFromPoints = true);

    /*! Set various MeshPrim properties from the MeshSample values, at the
        time of the sample.
        TODO: the number of optional args is a bit silly.  This should be
        separate methods or better yet MeshSample needs additional controls
        to help specify the desired properties with corresponding
        'is-animated' indicators so the per-attr animation state is known.
    */
    bool setMeshPrimProperties(usg::MeshPrim& prim,
                               bool           setUvs = false,
                               bool           setNormals = false,
                               bool           setColors = false,
                               bool           setOpacities = false,
                               bool           setVelocities = false,
                               bool           setAccelerations = false);

  };



/*---------------------------------------------------------------------*/
/*---------------------------------------------------------------------*/
/*                   Inline Function Implementations                   */
/*---------------------------------------------------------------------*/
/*---------------------------------------------------------------------*/

template <typename T>
inline void
reverseFaceWinding(size_t          nFaces,
                   const int32_t*  faceVertexCounts,
                   size_t          nVerts,
                   std::vector<T>& faceVertexValues)
{
  if (nFaces == 0 || nVerts < 2 || faceVertexValues.size() != nVerts) {
    return;
  }

  int32_t v = 0;
  for (size_t f=0; f < nFaces; ++f) {
    const int32_t nFaceVerts = faceVertexCounts[f];
    for (int32_t i=v,j=v+nFaceVerts-1; i < v+(nFaceVerts/2); ++i,--j) {
      // TODO: is std::swap cheaper for this?
      const T t = faceVertexValues[i];
      faceVertexValues[i] = faceVertexValues[j];
      faceVertexValues[j] = t;
    }
    v += nFaceVerts;
  }
}

template<typename T>
inline void
convertPrimvarToFaceVertRate(const usg::Vec3fArray& points,
                             const usg::Int32Array& faceVertexCounts,
                             const usg::Int32Array& faceVertexIndices,
                             bool                   isClockwiseWinding,
                             const fdk::TimeValue&  time,
                             usg::Primvar&          primvar,
                             usg::Array<T>&         flattenedValues)
{
  if (primvar) {
    const auto interpolation = primvar.getInterpolation();
    const int32_t size = primvar.attribute().getSize(time);
    const size_t nVerts = faceVertexIndices.size();
    const size_t nFaces = faceVertexCounts.size();

    primvar.computeFlattened(flattenedValues, time);
    if (flattenedValues.size() == 0) {
      return; // don't crash...
    }

    if ((interpolation == usg::GeomTokens.faceVarying ||
         interpolation == usg::GeomTokens.varying) && size == nVerts) {
      // Vertex-rate - copy but possibly unwind:
      if (isClockwiseWinding) {
        reverseFaceWinding(nFaces, faceVertexCounts.data(), nVerts, flattenedValues);
      }
    }
    else if (interpolation == usg::GeomTokens.vertex && size == points.size()) {
      // Point-rate - convert to facevert rate:
      const auto copy = flattenedValues;
      flattenedValues.resize(nVerts);
      for (size_t v = 0; v < nVerts; ++v) {
        flattenedValues[v] = copy[faceVertexIndices[v]];
      }
    }
    else if (interpolation == usg::GeomTokens.uniform && size == nFaces) {
      // Face-rate - convert to facevert rate:
      const auto copy = flattenedValues;
      flattenedValues.resize(nVerts);
      int faceVertexIndex = 0;
      for (size_t face = 0; face < nFaces; ++face) {
        const auto faceVertexCount = faceVertexCounts[face];
        for (int32_t v = 0; v < faceVertexCount; ++v) {
          flattenedValues[faceVertexIndex + v] = copy[face];
        }
        faceVertexIndex += faceVertexCount;
      }
    }
    else if (interpolation == usg::GeomTokens.constant && size == 1) {
      // Prim-rate - convert to facevert rate:
      flattenedValues.resize(nVerts, flattenedValues[0]);
    }
    else if (size >= 1) {
      // Error fallback to catch degenerate cases:
      flattenedValues.resize(nVerts, flattenedValues[0]);
    }
  }
}


} // namespace ndk

#endif
