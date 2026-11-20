// Copyright (c) 2010 The Foundry Visionmongers Ltd. All Rights Reserved.
#ifndef DDIMAGE_RAYCAST_H
#define DDIMAGE_RAYCAST_H

#include "DDImage/DDImage_API.h"
#include "DDImage/Allocators.h"
#include "DDImage/Vector3.h"
#include "DDImage/Vector4.h"
#include "DDImage/Box3.h"
#include "DDImage/RefCountedObject.h"

#include <list>
#include <vector>
#include <limits>
#include <memory>

namespace DD {
  namespace Image {

    class Iop;
    class GeoInfo;
    class Primitive;
    class PrimitiveContext;
    class Scene;
    class GeometryList;
    struct DrawableGeo;
    class ViewerContext;
    class RegularGridImpl;

    //!
    //! Ray
    //!

    struct DDImage_API Ray
    {
      Vector3 src, dir;
      float minT, maxT;
      unsigned int uniqueID;
    };

    struct DDImage_API  CollisionResult
    {
      CollisionResult()
      : _collisionTime(std::numeric_limits<float>::max())
      , _collisionNormal(0.0f, 0.0f, 0.0f)
      , _collisionPosition(0.0f, 0.0f, 0.0f)
      , _collisionPrimitiveIdx(0)
      , _collisionFaceIdx(0)
      , _collisionGeo(nullptr)
      , _collided(false)
      {
      }

      //! time between 0.0 (start of ray) and length of ray
      float             _collisionTime;   
      Vector3           _collisionNormal;
      Vector3           _collisionPosition;
      unsigned int      _collisionPrimitiveIdx;   //!< Index into geo's primitive list
      unsigned int      _collisionFaceIdx;        //!< Index into primitive's face list
      GeoInfo*          _collisionGeo;
      bool              _collided;
    };


    DDImage_API void GetScreenToWorldRay(ViewerContext* ctx, int screenX, int screenY, Ray &ray);

    //! An object of this class needs to be created before calling a ray cast. It is used to mark information about which 
    //! polygon a given ray has already been collision tested with. In the case where multiple ray casts are being 
    //! performed, this class should only be created once before all of the ray casts, eg.
    //!
    //!  PrimitiveTestCache  testCache(drawables);
    //!  for ( i = 0; i < numRays; i++) {
    //!    // Create and initialise a ray, and then call:
    //!    sceneGrid.testRayIntersection( param1, param2, testCache);
    //!  }
    //!
    //! Note that no part of the PrimitiveTestCache needs to be reset before the next ray cast. The only requirement 
    //! is that one PrimitiveTestCache needs to be created per thread.
    class DDImage_API PrimitiveTestCache
    {
    public:
      PrimitiveTestCache();
      PrimitiveTestCache(const std::vector<DrawableGeo>& objects);
      PrimitiveTestCache(const GeometryList& objects);
      ~PrimitiveTestCache();

      std::vector<unsigned int>&  operator[](unsigned int i)  { return _primTested[i];  }

      // Accessors
      CollisionResult*  getCollisionResult()  const   { return _collisionResult;  }
      void  setCollisionResult(CollisionResult* res)  { _collisionResult = res;   }

      void setCurrGeoId( int id ){ _currGeoID = id; }
      int getCurrGeoId()const { return _currGeoID; }
      bool isInit() const { return _init; }
      void init(const std::vector<GeoInfo*>& objects);
      void init(const std::vector<DrawableGeo>& objects);
      void clear() { _init = false; }

    private:
      bool _init;
      int _currGeoID;
      std::vector< std::vector<unsigned int> >  _primTested;
      CollisionResult*      _collisionResult;
    };


    //!
    //! BasicGrid
    //!

    class DDImage_API BasicGrid
    {
    public:
      BasicGrid();
      virtual ~BasicGrid();

      bool isValid() const { return !(_width == 0 || _height == 0 || _depth == 0); }
      unsigned int getWidth()  const { return _width; }
      unsigned int getHeight() const { return _height;}
      unsigned int getDepth()  const { return _depth;}

      unsigned int cellIndex(unsigned int x, unsigned int y, unsigned int z) const 
      { 
        return z * _width * _height + y * _width + x; 
      }

      const Vector3& cellSize() const { return _cellSize; }
      const Box3& bounds()      const { return _bounds;   }

      virtual bool getCellsAlongRay(ViewerContext* ctx, const Ray& ray, std::list<unsigned int>& candidates) const;      
      virtual bool processCellsAlongRay(ViewerContext* ctx, 
                                        const Ray& ray, 
                                        std::vector<bool>& tested, 
                                        PrimitiveTestCache& primTested);

      virtual bool processCell(ViewerContext* ctx, 
                                unsigned int cellIndex, 
                                const Ray& ray, 
                                std::vector<bool>& tested, 
                                PrimitiveTestCache& primTested)
      {
        return false;
      }

      void calculateCellSizes(int averageItemsPerCell, int totalItems);

      
      //! Get the cell color for wireframe draw. Used for debugging.
      virtual Vector4 cellColor(unsigned int index) const { return Vector4(0.0f, 0.0f, 0.0f, 0.0f); };

      //! Draw the wireframe grid. Used for debugging.
      void draw_wireframe(ViewerContext* vtx) const;

    protected:
      void clampGridDimensions();

    protected:
      unsigned int _width, _height, _depth;
      Vector3 _cellSize;
      Box3 _bounds;
    };

    struct DDImage_API GridCell
    {
      unsigned int lastObject;

      /// This is a list of indexes arranged into contiguous blocks by objectID.
      /// Each block ends with the marker value 0xFFFFFFFF. If there are no indexes
      /// from a particular object, then we just have the marker value without any
      /// preceding indexes.
      ///
      /// We assume that indexes are added in order of increasing objectID.
      std::vector<unsigned int, STL3DAllocator<unsigned int> > indexes;

      /// List of GeoInfos that intersect this cell
      std::vector<unsigned int, STL3DAllocator<unsigned int> > geoID;

      /// Is the cell clipped by the current selection area or completely inside?
      bool  _clipped;

      GridCell()
        : lastObject(0)
        , _clipped(true)
      {}

      void addIndex(unsigned int object, unsigned int point)
      {
        while (lastObject < object) {
          indexes.push_back(kNewObjectMarker);
          ++lastObject;
        }
        indexes.push_back(point);
      }

      void addObject(unsigned int object)
      {
        geoID.push_back(object);
      }

      /// Marker value used in the grid cell point indexes list to separate the
      /// index lists for different objects.
      static const unsigned int kNewObjectMarker;
    };


    class DDImage_API GridCellPointsIterator
    {
    public:
      GridCellPointsIterator(const GridCell& cell)
        : _cell(cell)
        , _objectID(0), 
        _index(0)
      {
        while (_index < _cell.indexes.size() && _cell.indexes[_index] == GridCell::kNewObjectMarker) {
          ++_objectID;
          ++_index;
        }
      }


      bool hasNext() const
      {
        return _index < _cell.indexes.size();
      }

      void next()
      {
        if (!hasNext())
          return;

        ++_index;
        while (_index < _cell.indexes.size() && _cell.indexes[_index] == GridCell::kNewObjectMarker) {
          ++_objectID;
          ++_index;
        }
      }

      unsigned int getObjectID() const
      {
        return _objectID;
      }

      unsigned int getPointID() const
      {
        return _cell.indexes[_index];
      }

    private:
      const GridCell& _cell;
      unsigned int _objectID;
      unsigned int _index;
    };


    //! A Regular grid is used to perform ray intersection with an array of DrawableGeo.
    //! Multiple threads can query for ray intersection, all the ray test intersection methods are thread safe.
    //! Please note that the rebuild method is not thread safe and must be called before
    //! any ray intersection test.

    using ClipTestFunc = bool (*)(const Box& box);

    class DDImage_API RegularGrid : public BasicGrid
    {
    public:
      RegularGrid();
      ~RegularGrid() override;

      GridCell& cell(unsigned int x, unsigned int y, unsigned int z);
      GridCell& cell(unsigned int index);

      void rebuild(const std::vector<GeoInfo*>& objects, bool buildPoints);
      void rebuild(const std::vector<DrawableGeo>& objects, bool buildPoints);
      void rebuild(const GeometryList& objects, bool buildPoints);

      virtual std::vector<unsigned> getCandidateGridCells(ViewerContext* ctx, const Box& selection, ClipTestFunc clipTestFunc = nullptr);

      bool testRayIntersection(ViewerContext* ctx, 
                               Ray& ray, 
                               PrimitiveTestCache& primTested, 
                               std::vector<bool>& tested,
                               CollisionResult* collisionInfo = nullptr);

      bool testRayIntersection(ViewerContext* ctx, 
                               Ray& ray, 
                               PrimitiveTestCache& primTested,
                               CollisionResult* collisionInfo = nullptr );

      static void getScreenToWorldRay(ViewerContext* ctx,
                                unsigned int screenX,
                                unsigned int screenY,
                                Ray& ray);


      bool screenToWorldRayTest(ViewerContext* ctx,
                                unsigned int screenX,
                                unsigned int screenY,
                                PrimitiveTestCache& primTested, 
                                CollisionResult* collisionInfo);

      bool processCell(ViewerContext* ctx, 
                                unsigned int cellIndex, 
                                const Ray& ray, 
                                std::vector<bool>& tested, 
                                PrimitiveTestCache& primTested) override;

      Vector4 cellColor(unsigned int index) const override;
      void draw_wireframe(ViewerContext* vtx) const;

    private:

      bool rayIntersection(ViewerContext* ctx, 
                           Ray& ray, 
                           PrimitiveTestCache& primTested, 
                           std::vector<bool>& tested,
                           CollisionResult* collisionInfo);
      
      void  rebuildPoints();

      std::unique_ptr<RegularGridImpl> _pImpl;

      // Not implemented
      RegularGrid(const RegularGrid&);
      RegularGrid& operator=(const RegularGrid&);

    };

  } // namespace Image
} // namespace DD

#endif
