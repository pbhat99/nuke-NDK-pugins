// Plane.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_Plane_h
#define DDImage_Plane_h

#include "DDImage_API.h"
#include "DDImage/DDMath.h"
#include "DDImage/Vector3.h"

/*! \class DD::Image::Plane

   A plane in 3D space ( nx*x + ny*y + nz*z + d = 0 )
 */

namespace DD
{
  namespace Image
  {

    class DDImage_API Plane
    {
    public:

      Vector3 normal;
      float d;

      /*! default constructor */
      Plane() : normal(Vector3(0.0f, 0.0f, 1.0f)), d(0.0f) {
      }

      /*! copy constructor */
      Plane(const Plane &plane) : normal(plane.normal), d(plane.d) {
      }

      /*! define a plane from a normal and a point on the plane */
      Plane(const Vector3 &n, const Vector3 &p) {
        normal = n;
        d = -normal.dot(p);
      }

      /*! define a plane by three points */
      Plane(const Vector3 &p0, const Vector3 &p1, const Vector3 &p2) {
        Vector3 v1 = p0 - p1;
        Vector3 v2 = p2 - p0;

        normal = v1.cross(v2);
        normal.normalize();
        d = -normal.dot(p0);
      }

      /*! distance between point and a plane */
      float distance( const Vector3& p) const
      {
        return normal.dot(p) + d;
      }

    };

  }
}  // end of namespace DD::Image

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
