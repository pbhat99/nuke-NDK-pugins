// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef USG_TOOLS_SUBDIVIDE_H
#define USG_TOOLS_SUBDIVIDE_H

#include "usg/api.h"
#include "usg/base/ArrayTypes.h"

#include "fdk/math/Vec2.h"
#include "fdk/math/Vec3.h"
#include "fdk/math/Vec4.h"

namespace usg {

  class ArgSet;

  //--------------------------------------------------------------------------------


  /*! For passing abstracted mesh data to tessellation methods.
  */
  struct USG_API MeshTessellateContext
  {
    usg::Int32Array*              faceVertexCounts;       //!< Face vert count, may not be required (all_quads, all_tris)
    usg::Int32Array*              vert_position_indices;  //!< Per-vertex position indices
    bool                          all_quads;              //!< Indicates that all faces are quads
    bool                          all_tris;               //!< Indicates that all faces are triangles

    // Per-point rate:
    std::vector<usg::Vec3fArray*> position_lists;         //!< Per-point positions data lists (motion samples)

    // Per-vertex rate attributes:
    std::vector<usg::FloatArray*> vert_float_attribs;     //!< Arbitrary per-vertex float data lists
    std::vector<usg::Vec2fArray*> vert_vec2_attribs;      //!< Arbitrary per-vertex Vec2 data lists
    std::vector<usg::Vec3fArray*> vert_vec3_attribs;      //!< Arbitrary per-vertex Vec3 data lists
    std::vector<usg::Vec4fArray*> vert_vec4_attribs;      //!< Arbitrary per-vertex Vec4 data lists

    MeshTessellateContext() :
      faceVertexCounts(nullptr),
      vert_position_indices(nullptr),
      all_quads(false),
      all_tris(false)
    {
    }
  };


  /*! Apply uniform subdivision to the attribute arrays in the MeshTessellateContext.
      The number of uniform refinement levels applied is (target_subd_level - current_subd_level).
      If the current level is unknown pass 0 for \a current_subd_level, which assumes
      the mesh is currently the subd cage.
  */
  USG_API
  void subdivideUniform(const ArgSet&          exec_args,
                        size_t                 nFaces,
                        size_t                 nVerts,
                        size_t                 nPoints,
                        int32_t                current_subd_level,
                        int32_t                target_subd_level,
                        MeshTessellateContext& tess_ctx);

}

#endif

