// Matrix3.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_Matrix3_h
#define DDImage_Matrix3_h

#include "DDImage/Vector3.h"
#include <string.h>

namespace DD
{
  namespace Image
  {

    /*! \class DD::Image::Matrix3

       \brief A 3x3 transformation matrix.

       You multiply a Vector3 by one of these to go from one 3D space to another.
       This cannot represent 3D translations or perspective, use a Matrix4 for
       that. This is also often used to transform color spaces.

       The data is stored packed together in OpenGL order, which is transposed
       from the way used in most modern graphics literature. This affects
       how the array() and [] operator work, these are all equal:
       - matrix.a12
       - matrix[2][1]
       - matrix.array()[1+3*2]
     */
    class DDImage_API Matrix3
    {
    public:
      // warning: this relies on C++ packing these together in order.
      // Matrix is stored in transposed order:
      float
      a00, a10, a20,
      a01, a11, a21,
      a02, a12, a22;

      // warning: for back compatibility the [][] operator is transposed
      // to be [col][row] order!
 
      /*! \fn float* Matrix3::operator[](int i)
         Return a pointer to the \e column number \a i. */
     float* operator[](int i) { return &a00 + i * 3; }

      /*! \fn const float* Matrix3::operator[](int i) const
         Return a pointer to the \e column number \a i. */
      const float* operator[](int i) const { return &a00 + i * 3; }

      /*! \fn const float* Matrix3::array() const
         Return a pointer to a00. This array is in the correct order to
         send it to OpenGL.
       */
      const float* array() const { return &a00; } // for passing to OpenGL

      Matrix3() {}

      /*! \fn Matrix3::Matrix3(float a, float b, float c,
                   float d, float e, float f,
                   float g, float h, float i)
         Initialize with a00=a, a01=b, a02=c, etc, ie the arguments are
         given as rows.
       */
      Matrix3(float a, float b, float c,
              float d, float e, float f,
              float g, float h, float i) {
        a00 = a;
        a01 = b;
        a02 = c;
        a10 = d;
        a11 = e;
        a12 = f;
        a20 = g;
        a21 = h;
        a22 = i;
      }

      void set(float a, float b, float c,
               float d, float e, float f,
               float g, float h, float i)
      {
        a00 = a;
        a01 = b;
        a02 = c;
        a10 = d;
        a11 = e;
        a12 = f;
        a20 = g;
        a21 = h;
        a22 = i;
      }

      /*! Does matrix multiplication. */
      Matrix3  operator *  ( const Matrix3& ) const;

      /*! Does matrix multiplication. */
      Matrix3& operator *= ( const Matrix3& );

      /*! Add each matching locations. */
      Matrix3  operator +  ( const Matrix3& ) const;

      /*! Add each matching locations. */
      Matrix3& operator += ( const Matrix3& );

      /*! Subtract each matching locations. */
      Matrix3  operator -  ( const Matrix3& ) const;

      /*! Subtract each matching locations. */
      Matrix3& operator -= ( const Matrix3& );

      /*! Multiplies every location by f. */
      Matrix3  operator *  (float) const;

      /*! Multiplies every location by f. */
      Matrix3& operator *= (float);
      Matrix3  operator /  (float d) const { return (*this) * (1 / d); }
      Matrix3& operator /= (float d) { return (*this) *= (1 / d); }

      /*! Returns the transformation of v by this matrix. */
      Vector3  operator *  ( const Vector3& v) const
      {
        return Vector3(
                 a00 * v.x + a01 * v.y + a02 * v.z,
                 a10 * v.x + a11 * v.y + a12 * v.z,
                 a20 * v.x + a21 * v.y + a22 * v.z);
      }
      /*! Same as this*v */
      Vector3 transform(const Vector3& v) const { return (*this) * v; }

      /*! \fn bool Matrix3::operator!=(const Matrix3& b) const
         Returns true if any of all 9 locations are different. */
      bool  operator != ( const Matrix3& b) const { return memcmp(&a00, &b.a00, 9 * sizeof(float)) != 0; }

      /*! \fn bool Matrix3::operator==(const Matrix3& b) const
         Returns true if all 9 locations are equal. */
      bool  operator == ( const Matrix3& b) const { return !memcmp(&a00, &b.a00, 9 * sizeof(float)); }

      /*! \fn float Matrix3::determinant(void) const
         Return the determinant. Non-zero means the transformation can be
         inverted.
       */
      float determinant() const
      {
        return
          a20 * (a01 * a12 - a02 * a11) +
          a21 * (a02 * a10 - a00 * a12) +
          a22 * (a00 * a11 - a01 * a10);
      }

      /*!
         Returns the inverse of this matrix where \a det is the precomputed
         determinant. This is useful if you already computed the determinant
         in order to see if you can invert the matrix.
       */
      Matrix3 inverse(float det) const;

      /*! \fn Matrix3 Matrix3::inverse() const
         Returns the inverse of this matrix. If determinant() is zero
         this will replace all the items with +/- infinity or zero.
       */
      Matrix3 inverse() const { return inverse(determinant()); }

      static const Matrix3 _identity;
      static const Matrix3& identity() { return _identity; }

      /*! Replace the contents with the identity. */
      void makeIdentity() { *this = _identity; }

      /*! Replace the contents with a uniform scale by \a x. */
      void scaling(float);

      /*! Replace the contents with a scale by \a x,y,z. */
      void scaling(float, float, float);

      /*! \fn void Matrix3::scaling(const Vector3& v)
         Replace the contents with a scale by the x,y,z of the vector.
       */
      void scaling(const Vector3& v) { scaling(v.x, v.y, v.z); }

      /*! Replace the contents with a rotation by angle (in radians) around
          the X axis. */
      void rotationX(float);

      /*! Replace the contents with a rotation by angle (in radians) around
          the Y axis. */
      void rotationY(float);

      /*! Replace the contents with a rotation by angle (in radians) around
          the Z axis. */
      void rotationZ(float);

      /*! \fn void Matrix3::rotation(float angle)
         Same as rotationZ(angle).
       */
      void rotation(float a) { rotationZ(a); }

      /*! Replace the contents with a rotation by angle (in radians) around
          the vector x,y,z. */
      void rotation(float a, float x, float y, float z);

      /*! \fn void Matrix3::rotation(float angle, const Vector3& v)
         Replace the contents with a rotation by angle (in radians) around
         the vector.
       */
      void rotation(float a, const Vector3& v) { rotation(a, v.x, v.y, v.z); }

      // destructive modifications:
      //void transpose();

      /*! Scale the transformation by \a s. */
      void scale(float);

      /*! Scale the transformation by \a x,y,z. */
      void scale(float, float, float = 1);
 
      /*! \fn void Matrix3::scale(const Vector3& v)
         Scale the transformation by the x,y,z of the vector.
       */
      void scale(const Vector3& v) { scale(v.x, v.y, v.z); }

      /*! Rotate the transformation by \a a radians about the X axis. */
      void rotateX(float);

      /*! Rotate the transformation by \a a radians about the Y axis. */
      void rotateY(float);

      /*! Rotate the transformation by \a a radians about the Z axis. */
      void rotateZ(float);

      /*! \fn void Matrix3::rotate(float a)
         Same as rotateZ(a). */
      void rotate(float a) { rotateZ(a); }

      /*! Rotate the transformation by \a a radians about the vector x,y,z. */
      void rotate(float a, float x, float y, float z);

      /*! \fn void Matrix3::rotate(float a, const Vector3& v)
         Rotate the transformation by \a a radians about the vector. */
      void rotate(float a, const Vector3& v) { rotate(a, v.x, v.y, v.z); }

      /*! Skew the transformation by \a a (X positions have a*Y added to them). */
      void skew(float a);

      /*! Add this to the Hash object. */
      void append(Hash&) const;

      /*! Writes it in nuke/tcl notation, with nested curly braces */
      friend std::ostream& operator << (std::ostream& o, const Matrix3& matrix );
    };

  }
}

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
