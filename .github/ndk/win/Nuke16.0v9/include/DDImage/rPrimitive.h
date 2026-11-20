// rPrimitive.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_rPrimitive_H
#define DDImage_rPrimitive_H

#include "DDImage/DDImage_API.h"
#include "DDImage/Box3.h"
#include <vector>

#ifdef _WIN32
  #pragma warning(push)
  #pragma warning(disable:4251)
#endif

#ifdef __INTEL_COMPILER
#pragma warning(disable:444)  
#endif

namespace DD
{
  namespace Image
  {

    //----------------------------------------------------------------------

    class Iop;
    class Scene;
    class VArray;
    class GeoInfo;
    class MatrixArray;
    class Primitive;
    class PrimitiveContext;
    class SubdivisionContext;

    /*! \brief This is the base class of anything renderable!
        We use this class to handle renderable objects that can have simple
        geometry such as triangles, spheres, etc.
     */
    /*!
       Rendering primitive parent class.  These objects are usually created
       by a 'Primitive' subclass when rendering is done.
       The primary difference between two classes is that the 'Primitive'
       always remains in scene space, or world space, and a rPrimitive is
       allocated per render sample and transformed into clip and screen space.
       These objects are destroyed after rendering is complete.
     */
    //This object derives from a Box* type, which doesn't have a virtual destructor. This
    //is why warning 444 has been disabled for the intel compiler at the top of this file - the important
    //message is to not delete via a Box* pointer, as the destructor will not
    //get called.
    class DDImage_API rPrimitive : public Box3
    {
    protected:
      const GeoInfo* geoinfo_; //!< Parent object
      const Primitive* parent_; //!< What primitive generated us

    public:

      enum  rPrimitiveType
      {
        eUnknownRPrimitive = -1,
        eRTriangle,
        eRPoint,
      };

      virtual const char* Class() { return "rPrimitive"; }
      rPrimitive(const GeoInfo * info, const Primitive * p);

      /*! Copy constructor. */
      rPrimitive(const rPrimitive &);

      /*! Base destructor does nothing. */
      virtual ~rPrimitive();

      virtual rPrimitive* duplicate() const { return nullptr;}

      const GeoInfo* geoinfo() const { return geoinfo_; }
      const Primitive* parent() const { return parent_; }

      bool cast_shadows() const;
      bool receive_shadows() const;

      /*! \fn bool rPrimitive::add_to_render(Scene*);

         Clip this primitive, transform it to screen space, and add it to the
         list of primitives to render in the Scene.

         The return value is true if \e this was added to the list. Originally
         this was used to indicate clipping. However implementations are now
         allowed to create other rPrimitive instances and add them to the list
         (for clipping into multiple pieces). If false is returned, this is
         immediately deleted. If true is returned it is assumed that destroying
         the Scene list will delete the primitive.
       */
      virtual bool add_to_render(Scene*, PrimitiveContext*) = 0;
      virtual bool add_to_displacement_render(Scene*, PrimitiveContext*){return false;}

      virtual bool intersect(float x, float y) const = 0;
      virtual bool intersect(float x, float y, float* dist) const = 0;
      virtual bool intersect_dist_squared(float x, float y, float* dist_squared) const = 0;
      virtual bool intersect_scanline(float y, int& X, int& R,
                                      VArray& v0, VArray& du, VArray& dv) const = 0;
      virtual float trace(Scene*) = 0;
      virtual float distance(float x, float y) const = 0;
      virtual float distance_squared(float x, float y) const = 0;

      virtual void subdivide(std::vector<rPrimitive*>& primitives, SubdivisionContext* stx) {}

      /*! Only for internal Nuke use */
      virtual rPrimitiveType getPrimitiveType() const { return eUnknownRPrimitive; }
      
      /* Required new/delete operator overload */
      void* operator new(size_t size);
      void  operator delete(void* mem);
      void* operator new[](size_t size);
      void  operator delete[](void* mem);
    };

  }
}

#ifdef __INTEL_COMPILER
#pragma warning(default:444)  
#endif

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
