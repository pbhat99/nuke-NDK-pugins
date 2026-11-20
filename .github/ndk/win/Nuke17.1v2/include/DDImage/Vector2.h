// Vector2.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_Vector2_h
#define DDImage_Vector2_h

#include "DDImage_API.h"
#include "math.h"
#include <iostream>
#include "DDImage/FloatFunctions.h"

/*! \class DD::Image::Vector2

   2-component vector, used to represent a point/vector in the plane.

   This object is in general compatible with the SbVec2 template in the
   Inventor library. However it allows direct access to the x,y,z
   members, which imho makes math using this much easier to read.

   See DD::Image::Vector3 for more details on the member functions.
 */

namespace DD
{
  namespace Image
  {

    class Hash;

    class DDImage_API Vector2
    {
    public:
      float      x, y;
      float&     operator[] (int i) { return *((&x) + i); }
      const float& operator[] (int i) const { return *((&x) + i); }
      const float* array() const { return &x; } // for passing to OpenGL

      Vector2() : x( 0.0f ), y( 0.0f ) {}
      Vector2(const Vector2 &i) : x(i.x), y(i.y) {}
      Vector2(const float v[2]) : x(v[0]), y(v[1]) {}
      Vector2(const double v[2]) : x((float)v[0]), y((float)v[1]) {}
      Vector2(float a, float b) : x(a), y(b) {}
      // This constructor causes problems with order-of-operation in
      // equations - use Vector2(float a, float b) instead:
      //   Vector2(float v) : x(v), y(v) {}

      Vector2& operator =  (const Vector2& v) { x = v.x;
                                                y = v.y;
                                                return *this; }

      void set(float i) { x = i;
                          y = i; }
      void set(float a, float b) { x = a;
                                   y = b; }
      void clear() { x = y = 0.0f; }

      Vector2  operator *  (float d) const { return Vector2(x * d, y * d); }
      Vector2& operator *= (float d) { x *= d;
                                       y *= d;
                                       return *this; }
      /*! This is component multiplication! Use dot() or cross() for vector multiply */
      Vector2  operator *  (const Vector2& v) const { return Vector2(x * v.x, y * v.y); }
      Vector2& operator *= (const Vector2& v) { x *= v.x;
                                                y *= v.y;
                                                return *this; }
      //  Vector2  operator *  (const Matrix4 &v) const;
      //  Vector2& operator *= (const Matrix4 &v);
      Vector2  operator /  (float d) const { return Vector2(x / d, y / d); }
      Vector2& operator /= (float d) { x /= d;
                                       y /= d;
                                       return *this; }
      /*! Component-wise division. */
      Vector2  operator /  (const Vector2& v) const { return Vector2(x / v.x, y / v.y); }
      Vector2& operator /= (const Vector2& v) { x /= v.x;
                                                y /= v.y;
                                                return *this; }
      Vector2  operator +  (const Vector2& v) const { return Vector2(x + v.x, y + v.y); }
      Vector2& operator += (const Vector2& v) { x += v.x;
                                                y += v.y;
                                                return *this; }
      Vector2  operator -  () const { return Vector2(-x, -y); }
      Vector2  operator -  (const Vector2& v) const { return Vector2(x - v.x, y - v.y); }
      Vector2& operator -= (const Vector2& v) { x -= v.x;
                                                y -= v.y;
                                                return *this; }
      bool operator == (const Vector2& v) const { return IsEqual(x, v.x) && IsEqual(y, v.y); }
      bool operator != (const Vector2& v) const { return !IsEqual(x, v.x) || !IsEqual(y, v.y); }
      bool operator == (float d) const { return IsEqual(x, d) && IsEqual(y, d); }
      bool operator != (float d) const { return !IsEqual(x, d) || !IsEqual(y, d); }

      //! Fairly arbitrary operator so you can store these in ordered arrays
      bool operator< ( const Vector2& v ) const
      {
        if (x < v.x)
          return true;
        if (x > v.x)
          return false;
        return y < v.y;
      }

      float length() const { return sqrtf(x * x + y * y); }

      float lengthSquared() const { return x * x + y * y; }

      /*! Same as (this-v).length() */
      float distanceBetween(const Vector2& v) const
      {
        return sqrtf((x - v.x) * (x - v.x) + (y - v.y) * (y - v.y));
      }

      /*! Same as (this-v).lengthSquared() */
      float distanceSquared(const Vector2& v) const
      {
        return (x - v.x) * (x - v.x) + (y - v.y) * (y - v.y);
      }

      float dot(const Vector2& v) const { return x * v.x + y * v.y; }

      //! Returns the Z component of the cross product, Ux*Vy-Uy*Vx
      float cross(const Vector2& v) const { return x * v.y - y * v.x; }

      //! Change the vector to be unit length. Returns the original length.
      float normalize()
      {
        float d = length();
        if (d)
          *this *= (1.0f / d);
        return d;
      }

      void negate() { x = -x;
                      y = -y; }

      // Vector2 reflect(const Vector2& nml) const {
      //   Vector2 r = nml * (nml.dot(*this) * 2.0f); return *this-r;}

      /*! Add this to the Hash object. */
      void append(Hash&) const;
      friend std::ostream& operator << (std::ostream& o, const Vector2&);
    };

  }
}  // end of namespace DD::Image

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
