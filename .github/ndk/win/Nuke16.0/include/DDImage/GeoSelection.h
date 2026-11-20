// (c) 2010 The Foundry Visionmongers Ltd.

#ifndef GEOSELECTION_H_
#define GEOSELECTION_H_

#include "DDImage/Hash.h"
#include "DDImage/OrderedMap.h"

#include <vector>

#if !defined(FN_COMPILER_INTEL) && defined(FN_OS_WINDOWS) // cl compiler on window
#  pragma warning( push )
#  pragma warning( disable : 4251 ) // warning #4251: needs to have dll-interface to be used by clients of <X>
#endif // !defined(FN_COMPILER_INTEL) && defined(FN_OS_WINDOWS)

namespace DD {
  namespace Image {

#ifndef FN_NO_LEGACY_3D
    class GeoInfo;

    using ObjectSelections = std::vector<float>;
    using SelectionWeights = std::vector<float>;

    //both Selections and both Weights should point at the same corresponding type
    //these are obsolete and defined only to maintain compatibility
    using VertexSelection = ObjectSelections;
    using FaceSelection   = ObjectSelections;
    using VertexWeights   = SelectionWeights;
    using FaceWeights     = SelectionWeights;


    struct GeoObjectSelection {
      VertexSelection vertices;
      VertexWeights vertexWeights;
      FaceSelection faces;
      FaceWeights faceWeights;
      bool object;

      GeoObjectSelection() :
        vertices(),
        vertexWeights(),
        faces(),
        faceWeights(),
        object(false)
      {
      }

      bool isClear() const
      {
        return
            vertices.empty()      &&
            vertexWeights.empty() &&
            faces.empty()         &&
            faceWeights.empty()   &&
            !object;
      }


      void append(Hash& hash)
      {
        hash.append(vertices);
        hash.append(vertexWeights);
        hash.append(faces);
        hash.append(faceWeights);
        hash.append(object);
      }
    };


    class DDImage_API GeoSelection : private OrderedMap<Hash, GeoObjectSelection*>
    {
      typedef OrderedMap<Hash, GeoObjectSelection*> Base;
      typedef Hash Key;
      typedef GeoObjectSelection* Value;

      mutable bool _cachedHashValid;
      mutable Hash _cachedHash;

      void invalidateCachedHash();

      template<typename T>
      friend class ReadWriteHandle;
    public:
      GeoSelection();
      ~GeoSelection();

      GeoSelection(const GeoSelection& other);
      GeoSelection& operator=(const GeoSelection& other);


      // Const accessors to contained objects
      bool                            hasKey(const Hash& objId) const;
      const GeoObjectSelection*       get(const Hash& objId) const;

      // indexed access
      size_t                          indexOf(const Hash& objId) const;
      const Hash&                     getKeyAt(size_t index) const;
      const GeoObjectSelection*       getAt(size_t index) const;

      // query the number of contained object selections
      size_t                          size() const;
      bool                            empty() const;

      /*! Set selection for an object id - transfers ownership of selection to GeoSelection */
      void set(const Hash& objId, GeoObjectSelection* selection);

      const VertexSelection& vertices(const Hash& objID) const;
      const VertexWeights& vertexWeights(const Hash& objID) const;
      const FaceSelection& faces(const Hash& objID) const;
      const FaceWeights& faceWeights(const Hash& objID) const;
      bool objectSelected(const Hash& objID) const;

      /*! A handle for accessing a VertexSelection / FaceSelection
          Will ensure any cached selection hashes are invalidated properly.
      */
      template<typename T>
      class ReadWriteHandle
      {
        GeoSelection* _parent;
        T*  _selection;
      public:
        ReadWriteHandle() : _parent(nullptr), _selection(nullptr)  {}
        ReadWriteHandle(GeoSelection* parent, T& selection) : _parent(parent), _selection(&selection) {}
        ~ReadWriteHandle() {if(_parent)_parent->invalidateCachedHash();}

        T& operator*() {return *_selection;}
        T* operator->() {return _selection;}

        const T& operator*() const {return *_selection;}
        const T* operator->() const {return _selection;}
      };

      /*! Obtain read/write access to the vertex selection array*/
      ReadWriteHandle<VertexSelection> verticesReadWrite(const Hash& objID);
      ReadWriteHandle<VertexWeights> vertexWeightsReadWrite(const Hash& objID);

      /*! Obtain read/write access to the face selection array*/
      ReadWriteHandle<FaceSelection> facesReadWrite(const Hash& objID);
      ReadWriteHandle<FaceWeights> faceWeightsReadWrite(const Hash& objID);

      void setVertices(const Hash& objID, const VertexSelection& selection);
      void setVertexWeights(const Hash& objID, const VertexWeights& selection);
      void setFaces(const Hash& objID, const FaceSelection& selection);
      void setFaceWeights(const Hash& objID, const FaceWeights& selection);
      void setObjectSelected(const Hash& objID, bool selected);

      void clearVertices();
      void clearVertexWeights();
      void clearFaces();
      void clearFaceWeights();
      void clearObjects();

      void setAllVertices(float amount);
      void setAllVertexWeights(float amount);
      void setAllFaces(float amount);
      void setAllFaceWeights(float amount);
      void setAllObjects(bool selected);

      void append(Hash& hash) const;

      void addSelection(const GeoSelection& sel);
      void removeSelection(const GeoSelection& sel);

      void deleteAll();
      void deleteEmpty();

      static Hash geoID(const GeoInfo& geo);

    private:
      template<typename Functor>
      void setImpl(const Hash& objID, Functor fun);
      template<typename Functor>
      void clearImpl(Functor fun);
      template<typename Functor>
      void setAllImpl(float amount, Functor fun);
    };
#endif

  } // namespace Image
} // namespace DD

#if !defined(FN_COMPILER_INTEL) && defined(FN_OS_WINDOWS)
#  pragma warning( pop )
#endif // !defined(FN_COMPILER_INTEL) && defined(FN_OS_WINDOWS)

#endif /* GEOSELECTION_H_ */
