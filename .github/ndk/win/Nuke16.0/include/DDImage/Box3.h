// Box3.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_Box3_H
#define DDImage_Box3_H

#include "Box.h"
#include "Vector3.h"
#include "DDImage/DDMath.h"

namespace DD
{
  namespace Image
  {

    class Matrix4;

    //! 3D Box
    class DDImage_API Box3
    {
      Vector3 min_; // "Lower-left"
      Vector3 max_; // "Upper-right"
    public:
      Box3() : min_(0, 0, 0), max_(-1, 0, 0) {}
      Box3(float x, float y, float z) {
        min_.x = max_.x = x;
        min_.y = max_.y = y;
        min_.z = max_.z = z;
      }
      Box3(const Vector3 p) { min_ = p;
                              max_ = p; }
      Box3(const Vector3 p1, const Vector3 p2) { min_ = p1;
                                                 max_ = p2; }

      const Vector3& min() const { return min_; }
      const Vector3& max() const { return max_; }
      float x() const { return min_.x; }
      float y() const { return min_.y; }
      float n() const { return min_.z; } //!< near, min z
      float r() const { return max_.x; } //!< right, max x
      float t() const { return max_.y; } //!< top, max y
      float f() const { return max_.z; } //!< far, max z
      float w() const { return max_.x - min_.x; } //!< width
      float h() const { return max_.y - min_.y; } //!< height
      float d() const { return max_.z - min_.z; } //!< depth
      //   float center_x() const {return (min_.x+max_.x)/2;}
      //   float center_y() const {return (min_.y+max_.y)/2;}
      //   float center_z() const {return (min_.z+max_.z)/2;}
      Vector3 center() const { return (min_ + max_) / 2.0f; }
      float radius() const { return (max_ - min_).length() / 2.0f; }

      void set(float x, float y, float z, float r, float t, float f)
      {
        min_.set(x, y, z);
        max_.set(r, t, f);
      }
      void set(Vector3 p1, Vector3 p2) { min_ = p1;
                                         max_ = p2; }
      void set(const Box3& v) { *this = v; }
      void set_min(float x, float y, float z) { min_.set(x, y, z); }
      void set_max(float x, float y, float z) { max_.set(x, y, z); }

      /*! Return true if the box is empty. */
      bool empty() const { return max_.x < min_.x; }
      void clear() { min_.set(0, 0, 0);
                     max_.set(-1, 0, 0); }

      /*! Return true if the Vector3 point is inside this box. */
      bool inside(const Vector3& p) const
      {
        if (p.x >= min_.x && p.x <= max_.x &&
            p.y >= min_.y && p.y <= max_.y &&
            p.z >= min_.z && p.z <= max_.z)
          return true;
        return false;
      }


      /*! Union the box with box \a v. If this is empty it is set to
         the other box, if the other one is empty this is unchanged.
       */
      void expand(const Box3& v);

      /*! Expand the bbox to contain \a p. If it is empty it is set to a
         zero-size box at p. */
      void expand(const Vector3& p);
      void intersect(const Box3& v);


      /*! Transform each corner of the box by the matrix, merging them
          into a new axis-aligned box.
       */
      void transform(const Matrix4& m);

      /*! Transform each corner of the box by a projection matrix, merging them
          into 2D bbox.  This is typically used to project into screen space.
          Pass false to 'wnormalize' if the perspective division is not wanted.
       */
      void project(const Matrix4& m, Box& bbox) const;

      void append(Hash& h) const { min_.append(h);
                                   max_.append(h); }

      void print_info(const char* title = nullptr) const;
    };

  }
}

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
