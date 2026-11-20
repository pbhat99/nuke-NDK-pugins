// Vector4.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_Vector4_h
#define DDImage_Vector4_h

#include "Vector3.h"

namespace DD
{
  namespace Image
  {

    /*! \class DD::Image::Vector4

       4-component vector. This is used to represent a point in 3D
       homogenous (perspective) space, and is also useful to represent
       an rgb color with alpha.

       This object is NOT compatible with the SbVec4 template in the
       Inventor library. <i>Several of the operators ignore the w and
       treat this as a Vector3</i>. This was done as it made use of this
       in the 3D system much easier.
     */
    class DDImage_API Vector4
    {
    public:
      float      x, y, z, w;

      /*! vector[0] is x, vector[1] is y, vector[2] is z, vector[3] is w. */
      float&     operator[] (int i) { return *((&x) + i); }

      /*! vector[0] is x, vector[1] is y, vector[2] is z, vector[3] is w. */
      const float& operator[] (int i) const { return *((&x) + i); }

      /*! Returns a ponter that can be passed to OpenGL */
      const float* array() const { return &x; }

      /*! Default Constructor. */
      Vector4() : x(0.0f), y(0.0f), z(0.0f), w(1.0f) {}

      /*! Constructor from an array of numbers. */
      Vector4(const float v[4]) : x(v[0]), y(v[1]), z(v[2]), w(v[3]) {}

      /*! Constructor from an array of numbers. */
      Vector4(const double v[4]) : x((float)v[0]), y((float)v[1]), z((float)v[2]), w((float)v[3]) {}

      /*! Constructor that sets all 4 numbers */
      Vector4(float a, float b, float c = 0, float d = 1) : x(a), y(b), z(c), w(d) {}

      /*! Change all of xyzw. */
      void set(float a, float b, float c = 0, float d = 1) { x = a;
                                                             y = b;
                                                             z = c;
                                                             w = d; }

      /*! Conversion from a Vector3 */
      Vector4(const Vector3 &v, float d = 1) : x(v.x), y(v.y), z(v.z), w(d) {}
      /*! Conversion from a Vector3 */
      void set(const Vector3& v, float d = 1) { x = v.x;
                                                y = v.y;
                                                z = v.z;
                                                w = d; }
      /*! Divides xyz by w and returns that */
      Vector3 divide_w() const { return Vector3(x / w, y / w, z / w); }
      /*! Just ignores the w. This is correct for a distance where w==0. */
      Vector3 truncate_w() const { return Vector3(x, y, z); }

      /*! Multiplies all 4 numbers. This will multiply a distance. However if
         w is non-zero, the resulting 3D point will not move, because x/w
         will not change. */
      Vector4  operator *  (float d) const { return Vector4(x * d, y * d, z * d, w * d); }
      Vector4& operator *= (float d) { x *= d;
                                       y *= d;
                                       z *= d;
                                       w *= d;
                                       return *this; }
      /*! Divides all 4 numbers. */
      Vector4  operator /  (float d) const { return Vector4(x / d, y / d, z / d, w / d); }
      Vector4& operator /= (float d) { x /= d;
                                       y /= d;
                                       z /= d;
                                       w /= d;
                                       return *this; }
      /*! Component-wise multiplication, useful for colors. */
      Vector4  operator *  (const Vector4& v) const { return Vector4(x * v.x, y * v.y, z * v.z, w * v.w); }
      Vector4& operator *= (const Vector4& v) { x *= v.x;
                                                y *= v.y;
                                                z *= v.z;
                                                w *= v.w;
                                                return *this; }
      /*! Component-wise division, useful for colors. */
      Vector4  operator /  (const Vector4& v) const { return Vector4(x / v.x, y / v.y, z / v.z, w / v.w); }
      Vector4& operator /= (const Vector4& v) { x /= v.x;
                                                y /= v.y;
                                                z /= v.z;
                                                w /= v.w;
                                                return *this; }

      /*! Component-wise addition. */
      Vector4  operator +  (const Vector4& v) const { return Vector4(x + v.x, y + v.y, z + v.z, w + v.w); }
      Vector4& operator += (const Vector4& v) { x += v.x;
                                                y += v.y;
                                                z += v.z;
                                                w += v.w;
                                                return *this; }

      /*! Component-wise subtraction. */
      Vector4  operator -  () const { return Vector4(-x, -y, -z, -w); }
      Vector4  operator -  (const Vector4& v) const { return Vector4(x - v.x, y - v.y, z - v.z, w - v.w); }
      Vector4& operator -= (const Vector4& v) { x -= v.x;
                                                y -= v.y;
                                                z -= v.z;
                                                w -= v.w;
                                                return *this; }

      bool operator == (const Vector4& v) const { return IsEqual(x, v.x) && IsEqual(y, v.y) && IsEqual(z, v.z) && IsEqual(w, v.w); }
      bool operator != (const Vector4& v) const { return !IsEqual(x, v.x) || !IsEqual(y, v.y) || !IsEqual(z, v.z) || !IsEqual(w, v.w); }
      bool operator == (float d) const { return IsEqual(x, d) && IsEqual(y, d) && IsEqual(z, d) && IsEqual(w, d); }
      bool operator != (float d) const { return !IsEqual(x, d) || !IsEqual(y, d) || !IsEqual(z, d) || !IsEqual(w, d); }

      //! Fairly arbitrary operator so you can store these in ordered arrays
      bool operator< ( const Vector4& v ) const
      {
        if (x < v.x)
          return true;
        if (x > v.x)
          return false;
        if (y < v.y)
          return true;
        if (y > v.y)
          return false;
        if (z < v.z)
          return true;
        if (z > v.z)
          return false;
        return w < v.w;
      }

#if 0
      //! Returns length of 3D vector (x,y,z), <i>w is IGNORED</i>.
      float length() const { return sqrt(x * x + y * y + z * z); }

      //! Same as length() squared, or this->dot(*this). <i>w is IGNORED</i>.
      float lengthSquared() const { return x * x + y * y + z * z; }

      //! 3D dot product, <i>w is IGNORED</i>.
      float dot(const Vector4& v) const { return x * v.x + y * v.y + z * v.z; }

      //! 3D cross product, <i>w is IGNORED</i>, output w is 1.
      Vector4 cross(const Vector4& v) const
      {
        return Vector4(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
      }

      //! Normalizes based on length of 3D vector (x,y,z), <i>w is IGNORED</i> and UNCHANGED!
      float normalize()
      {
        float d = length();
        if (d) { x /= d;
                 y /= d;
                 z /= d; }
        return d;
      }
#endif

      /*! Add this to the Hash object. */
      void append(Hash&) const;

      /*! Writes it in nuke/tcl notation, with nested curly braces */
      friend std::ostream& operator << (std::ostream& o, const Vector4&);
    };

  }
}  // end of namespace DD::Image

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
// end of Vector4.h
