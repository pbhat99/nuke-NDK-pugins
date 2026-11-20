// Vector3.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_Vector3_h
#define DDImage_Vector3_h

#include "DDImage_API.h"
#include "DDImage/DDMath.h"
#include "DDImage/FloatFunctions.h"
#include <iostream>

namespace DD
{
  namespace Image
  {

    class Hash;

    /*! \class DD::Image::Vector3

       3-component vector, used to represent a point or distance in 3D space.

       This object is in general compatible with the SbVec3 template in the
       Inventor library. However it allows direct access to the x,y,z
       members, which imho makes math using this much easier to read.
     */
    class DDImage_API Vector3
    {
    public:
      float      x, y, z;

      /*! vector[0] is x, vector[1] is y, vector[2] is z. */
      float&     operator[] (int i) { return *((&x) + i); }

      /*! vector[0] is x, vector[1] is y, vector[2] is z. */
      const float& operator[] (int i) const { return *((&x) + i); }

      /*! Returns a ponter that can be passed to OpenGL */
      const float* array() const { return &x; } // for passing to OpenGL

      Vector3() : x( 0.0f ), y( 0.0f ), z( 0.0f ) {}

      /*! Constructor from an array of numbers. */
      Vector3(const float v[3]) : x(v[0]), y(v[1]), z(v[2]) {}

      /*! Constructor from an array of numbers. */
      Vector3(const double v[3]) : x((float)v[0]), y((float)v[1]), z((float)v[2]) {}

      /*! Constructor that sets all 4 numbers */
      Vector3(float a, float b, float c) : x(a), y(b), z(c) {}

      /*! Change all of xyz. */
      void set(float a, float b, float c) { x = a;
                                            y = b;
                                            z = c; }

      /*! Multiplies all 3 numbers. This will scale the length(). */
      Vector3  operator *  (float d) const { return Vector3(x * d, y * d, z * d); }
      Vector3& operator *= (float d) { x *= d;
                                       y *= d;
                                       z *= d;
                                       return *this; }
      Vector3  operator *  (double d) const { return operator*(static_cast<float>(d)); }
      Vector3& operator *= (double d) { return(*this *= static_cast<float>(d)); }

      /*! Divides all 3 numbers. This will scale the length(). */
      // TODO : investigate multiplying through by the reciprocal i.e. doing the divide just once.
      Vector3  operator /  (float d) const { return Vector3(x / d, y / d, z / d); }
      Vector3& operator /= (float d) { x /= d;
                                       y /= d;
                                       z /= d;
                                       return *this; }

      /*! Component-wise multiplication, useful for colors. */
      Vector3  operator *  (const Vector3& v) const { return Vector3(x * v.x, y * v.y, z * v.z); }
      Vector3& operator *= (const Vector3& v) { x *= v.x;
                                                y *= v.y;
                                                z *= v.z;
                                                return *this; }
      /*! Component-wise division, useful for colors. */
      Vector3  operator /  (const Vector3& v) const { return Vector3(x / v.x, y / v.y, z / v.z); }
      Vector3& operator /= (const Vector3& v) { x /= v.x;
                                                y /= v.y;
                                                z /= v.z;
                                                return *this; }

      /*! Addition */
      Vector3  operator +  (const Vector3& v) const { return Vector3(x + v.x, y + v.y, z + v.z); }
      Vector3& operator += (const Vector3& v) { x += v.x;
                                                y += v.y;
                                                z += v.z;
                                                return *this; }

      /*! Subtraction */
      Vector3  operator -  (const Vector3& v) const { return Vector3(x - v.x, y - v.y, z - v.z); }
      Vector3& operator -= (const Vector3& v) { x -= v.x;
                                                y -= v.y;
                                                z -= v.z;
                                                return *this; }
      Vector3  operator -  () const { return Vector3(-x, -y, -z); }

      bool operator == (const Vector3& v) const { return IsEqual(x, v.x) && IsEqual(y, v.y) && IsEqual(z, v.z); }
      bool operator != (const Vector3& v) const { return !IsEqual(x, v.x) || !IsEqual(y, v.y) || !IsEqual(z, v.z); }
      bool operator == (float d) const { return IsEqual(x, d) && IsEqual(y, d) && IsEqual(z, d); }
      bool operator != (float d) const { return !IsEqual(x, d) || !IsEqual(y, d) || !IsEqual(z, d); }

      //! Fairly arbitrary operator so you can store these in ordered arrays
      bool operator< ( const Vector3& v ) const
      {
        if (x < v.x)
          return true;
        if (x > v.x)
          return false;
        if (y < v.y)
          return true;
        if (y > v.y)
          return false;
        return z < v.z;
      }

      /*! Also known as the absolute value or magnitude of the vector. */
      float length() const { return sqrtf(x * x + y * y + z * z); }

      //! Same as this dot this, length() squared
      float lengthSquared() const { return x * x + y * y + z * z; }

      /*! Same as (this-v).length() */
      float distanceBetween(const Vector3& v) const
      {
        return sqrtf((x - v.x) * (x - v.x) + (y - v.y) * (y - v.y) + (z - v.z) * (z - v.z));
      }

      /*! Same as (this-v).lengthSquared() */
      float distanceSquared(const Vector3& v) const
      {
        return (x - v.x) * (x - v.x) + (y - v.y) * (y - v.y) + (z - v.z) * (z - v.z);
      }

      /*! Dot product. Twice the area of the triangle between the vectors. */
      float dot(const Vector3& v) const { return x * v.x + y * v.y + z * v.z; }

      /*! Cross product. This is a vector at right angles to the vectors
         with a length equal to the product of the lengths. */
      Vector3 cross(const Vector3& v) const
      {
        return Vector3(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
      }

      /*! Change the vector to be unit length. Returns the original length. */
      float normalize()
      {
        float d = length();
        if (d)
          *this *= (1.0f / d);
        return d;
      }

      /*! Uwe Behrens' fast-normalize method.
         Approximate normalization, returns approximate length. */
      float fast_normalize()
      {
        const float xx = fabsf(x), yy = fabsf(y), zz = fabsf(z);
        const float max = xx > yy ? (xx > zz ? xx : zz) : (yy > zz ? yy : zz);
        const float fx = (xx * xx + yy * yy + zz * zz) / (max * max);
        const float d = max * ((-0.0555f * fx + 0.5849f) * fx + 0.4706f);
        *this *= 1 / d;
        return d;
      }

      /*! Modifies it in-place, does not return the new value. */
      void negate() { x = -x;
                      y = -y;
                      z = -z; }

      float distanceFromPlane(float A, float B, float C, float D) const
      {
        return A * x + B * y + C * z + D;
      }

      Vector3 reflect(const Vector3& N)
      {
        Vector3 r = N * (dot(N) * 2.0f) - *this;
        return r;
      }

      Vector3 minimum(const Vector3& a) const
      {
        return Vector3(MIN(a.x, x), MIN(a.y, y), MIN(a.z, z));
      }
      Vector3 maximum(const Vector3& a) const
      {
        return Vector3(MAX(a.x, x), MAX(a.y, y), MAX(a.z, z));
      }

      /*! Add this to the Hash object. */
      void append(Hash&) const;

      /*! Writes it in nuke/tcl notation, with nested curly braces */
      friend std::ostream& operator << (std::ostream& o, const Vector3&);
    };

  }
}  // end of namespace DD::Image

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
