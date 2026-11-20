// ModifyGeo.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_ModifyGeo_h
#define DDImage_ModifyGeo_h

#include "DDImage/GeoOp.h"

#ifdef _WIN32
  #pragma warning(disable:4251)
#endif

namespace DD
{
  namespace Image
  {

    /*!
        It's primary task is to make the geometry_engine() call more easy
        to implement by managing the modified point list prior to and after the
        engine call.  In addition to automatically calling \b evaluate_transform()
        to see if the points must be transformed by the object matrix prior to
        manipulation, ModifyGeo copies the point list from the input so a plugin
        writer must only need to write a single point loop.
        The plugin author must at least implement the \b modify_geometry() method.
     */
    class DDImage_API ModifyGeo : public GeoOp
    {
    protected:

      ModifyGeo(Node * node) : GeoOp(node) {}


      /*! Sub-class dependent portion of get_geometry().
          This should be implemented on plugins to modify geometry.  The default
          implementation gets the input's points then calls the single cache
          modify_geometry() on each GeoInfo in the output cache list.
       */
      void geometry_engine(Scene& scene, GeometryList& out) override;

      /*! Single object GeoInfo modify_geometry call so that plugins which
         apply the same function to all GeoInfos in the output list can just
         implement this one function.
         Pure virtual - must be implemented.
       */
      virtual void modify_geometry(int obj, Scene&, GeometryList& out) = 0;

    };

  }
}

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
