// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef NDK_GEO_UTILS_MESHTOPOLOGY_H
#define NDK_GEO_UTILS_MESHTOPOLOGY_H

#include "ndk/api.h"

#include "usg/base/ArrayTypes.h"
#include "usg/geom/MeshPrim.h"
#include "usg/geom/GeomTokens.h"

namespace ndk {
  
  using GLuint = unsigned int;

  //! Mesh topology
  struct NDK_API MeshTopology
  {
  private:
    //! An iterator over the faces of a mesh
    class FaceIterator {
    public:
      FaceIterator(const MeshTopology& sample, int startFace)
        : _sample(sample)
        , _face(startFace)
      {
      }

      FaceIterator& operator++()
      {
        _vertex += faceVertexCount();
        _face++;
        return *this;
      }

      const FaceIterator& operator*() const
      {
        return *this;
      }

      bool operator==(const FaceIterator& rhs) const
      {
        return &_sample == &rhs._sample && _face == rhs._face;
      }

      bool operator!=(const FaceIterator& rhs) const
      {
        return !(*this == rhs);
      }

      //! Return the index of the current face
      int faceIndex() const { return _face; }

      //! Return the number of vertices in the current face
      int faceVertexCount() const { return _sample.faceVertexCounts[_face]; }

      //! Return the vertex index for vertex \a n of the current face
      int faceVertexIndex(int n) const { return _sample.faceVertexIndices[_vertex + n]; }

      //! Return a pointer to the vertex indices for the current face
      const int* faceVertexIndices() const { return &_sample.faceVertexIndices[_vertex]; }

      //! Return the point location for vertex \a n of the current face
      const fdk::Vec3f& facePoint(int n) const { return _sample.points[_sample.faceVertexIndices[_vertex + n]]; }

    private:
      FaceIterator() = delete;

      const MeshTopology& _sample;
      int _face = 0;
      int _vertex = 0;
    };

    class Faces {
    public:
      Faces(const MeshTopology& sample)
        : _sample(sample)
      {
      }

      FaceIterator begin() const { return FaceIterator(_sample, 0); }
      FaceIterator end() const { return FaceIterator(_sample, _sample.faceVertexCounts.size()); }

    private:
      const MeshTopology& _sample;
    };


    //! An iterator over the edges of a mesh. For shared edges which join two faces, the edge will be returned
    //! twice, once in each direction.
    class EdgeIterator {
    public:
      EdgeIterator(const MeshTopology& sample, int startFace)
        : _sample(sample)
        , _face(startFace)
      {
      }

      EdgeIterator& operator++()
      {
        const auto n = faceVertexCount();
        if (++_faceVertex == n) {
          _vertex += n;
          _face++;
          _faceVertex = 0;
        }
        return *this;
      }

      const EdgeIterator& operator*() const
      {
        return *this;
      }

      bool operator==(const EdgeIterator& rhs) const
      {
        return &_sample == &rhs._sample && _face == rhs._face && _faceVertex == rhs._faceVertex;
      }

      bool operator!=(const EdgeIterator& rhs) const
      {
        return !(*this == rhs);
      }

      //! Return the index of the current face
      int faceIndex() const { return _face; }

      //! Return the number of vertices in the current face
      int faceVertexCount() const { return _sample.faceVertexCounts[_face]; }

      //! Return the vertex index for vertex \a n of the current face
      int faceVertexIndex(int n) const { return _sample.faceVertexIndices[_vertex + n]; }

      //! Return a pointer to the vertex indices for the current face
      const int* faceVertexIndices() const { return &_sample.faceVertexIndices[_vertex]; }

      //! Return the point location for point \a n of the current face
      const fdk::Vec3f& facePoint(int n) const { return _sample.points[_sample.faceVertexIndices[_vertex + n]]; }

      //! Return the starting vertex index for the current edge
      int startVertex() const { return _sample.faceVertexIndices[_vertex+_faceVertex]; }

      //! Return the ending vertex index for the current edge
      int endVertex() const { return _sample.faceVertexIndices[_vertex + ((_faceVertex+1) % faceVertexCount())]; }

    private:
      EdgeIterator() = delete;

      const MeshTopology& _sample;
      int _face = 0;
      int _vertex = 0;
      int _faceVertex = 0;
    };

    class Edges {
    public:
      Edges(const MeshTopology& sample)
        : _sample(sample)
      {
      }

      EdgeIterator begin() const { return EdgeIterator(_sample, 0); }
      EdgeIterator end() const { return EdgeIterator(_sample, _sample.faceVertexCounts.size()); }

    private:
      const MeshTopology& _sample;
    };

  public:

    //! The type of topology of the mesh (e.g. all tris, all quads, non-uniform)
    enum class TopologyType {
      NotSet,
      AllTris,
      AllQuads,
      NonUniform
    };

    //! Constructs the topology of the mesh at \a time.
    MeshTopology(const usg::MeshPrim& prim, const fdk::TimeValue& time);

    //! Destructor
    ~MeshTopology();

    //! Default constructor
    MeshTopology() = default;

    //! Move constructor
    MeshTopology(MeshTopology&& other) noexcept;

    //! Return an object which can be iterated to retrieve all faces of the mesh
    //! e.g. for (auto&& it : sample.faces()) { ... }
    const Faces faces() const { return Faces(*this); }

    //! Return an object which can be iterated to retrieve all edges of the mesh. Where two faces share an edge, the edge
    //! will be returned twice, once for each face in each direction.
    //! e.g. for (auto&& it : sample.edges()) { ... }
    const Edges edges() const { return Edges(*this); }

    //! Determine and store the topology type based on the face vertex counts.
    void storeTopologyType();

    //! Create OpenGL buffers for rendering the mesh if the topology is all tris or all quads.
    void createGLBuffers();

    //! Clear any OpenGL buffers that were created for rendering.
    void clearGLBuffers();

    //! Triangulate the mesh topology if it is non-uniform, producing new face vertex indices and counts.
    //! The new arrays are returned via the output parameters.
    bool triangulate(
      usg::Int32Array& newFaceVertexIndices,
      usg::Int32Array& newFaceVertexCounts);

    //! Get the VBO of the topology for rendering
    GLuint getVBO() const { return _vbo; }

    //! Get the EBO of the topology for rendering
    GLuint getEBO() const { return _ebo; }

    //! Get the topology type
    TopologyType getTopologyType() const { return _topologyType; }

    //! Get the topology type on the GPU.
    TopologyType getTopologyTypeGPU() const { return _topologyTypeGPU; }

    //! Get the total face count on the GPU.
    unsigned getTotalFaceCountGPU() const { return _totalFaceCountGPU; }

    usg::Vec3fArray  points;              //!< Local-space point locations
    usg::Int32Array  faceVertexCounts;    //!< Per-face vert count (not all_tris or all_quads)
    usg::Int32Array  faceVertexIndices;   //!< Per facevert point location indices

  private:
    TopologyType     _topologyType = TopologyType::NotSet;    //!< The type of topology
    TopologyType     _topologyTypeGPU = TopologyType::NotSet; //!< The type of topology on the GPU (after triangulation if needed)
    unsigned         _totalFaceCountGPU = 0;                  //!< Total number of faces on the GPU (after triangulation if needed)
    GLuint           _vbo = -1, _ebo = -1;                     //!< OpenGL buffers for rendering (if needed)
  };

}

#endif
