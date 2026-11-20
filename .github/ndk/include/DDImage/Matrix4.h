// Matrix4.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_Matrix4_h
#define DDImage_Matrix4_h

#include "DDImage/Vector4.h"
#include "DDImage/Vector3.h"
#include "DDImage/Vector2.h"

#include <string.h>
#include <assert.h>

#include <vector>
#include <cmath>

#ifdef FN_OS_LINUX
#include <limits>
#endif

#define DD_IMAGE_MATRIX4_SIZE 16
#define DD_IMAGE_MATRIX4_BYTE_SIZE (DD_IMAGE_MATRIX4_SIZE*sizeof(float))
// Define the mistyped ones for backwards-compatibility:
#define DD_IMAGE_MATRXI4_SIZE      DD_IMAGE_MATRIX4_SIZE
#define DD_IMAGE_MATRXI4_BYTE_SIZE DD_IMAGE_MATRIX4_BYTE_SIZE

namespace DD
{
  namespace Image
  {

    /*! \class DD::Image::Matrix4

       \brief A 4x4 transformation matrix.

       Used to represent perspective transforms. Also because this is the
       only form OpenGL accepts, this is often used for affine transforms
       that could in fact be stored in a Matrix3.

       The data is stored packed together in OpenGL order, which is transposed
       from the way used in most modern graphics literature. This affects
       how the array() and [] operator work, these are all equal:
       - matrix.a12
       - matrix[2][1]
       - matrix.array()[1+4*2]
     */
    class DDImage_API Matrix4
    {

      inline int isNaN(float lValue) const { 
        return std::isnan(lValue);
      }
    public:
      enum TransformOrder { 
        eSRT = 0, 
        eSTR, 
        eRST, 
        eRTS, 
        eTSR, 
        eTRS 
      };

      enum RotationOrder { 
        eXYZ = 0, 
        eXZY, 
        eYXZ, 
        eYZX, 
        eZXY, 
        eZYX 
      };

      // warning: this relies on C++ packing these together in order.

      // Matrix is stored in transposed order to allow it to be passed
      // to OpenGL unchanged:
      float
      a00, a10, a20, a30,
      a01, a11, a21, a31,
      a02, a12, a22, a32,
      a03, a13, a23, a33;

      // warning: for back compatibility the [][] operator is transposed
      // to be [col][row] order!

      /*! \fn float* Matrix4::operator[](int i)
          Return a pointer to the \e column number \a i. */
      float* operator[](int i) { return &a00 + i * 4; }

      /*! \fn const float* Matrix4::operator[](int i) const
          Return a pointer to the \e column number \a i. */
      const float* operator[](int i) const { return &a00 + i * 4; }

      /*! \fn const float* Matrix4::array() const
          Return a pointer to a00. This array is in the correct order to
          send it to OpenGL (column order).
       */
      const float* array() const { return &a00; } // for passing to OpenGL

      Matrix4() {}
      Matrix4(const float array[DD_IMAGE_MATRXI4_SIZE]) { memcpy(&a00, array, DD_IMAGE_MATRXI4_BYTE_SIZE); }


      /*! \fn Matrix4::Matrix4(float a, float b, float c, float d,
                              float e, float f, float g, float h,
                              float i, float j, float k, float l,
                              float m, float n, float o, float p)
          Initialize with a00=a, a01=b, a02=c, etc, ie the arguments are
          given as rows.
       */
      Matrix4(float a, float b, float c, float d,
              float e, float f, float g, float h,
              float i, float j, float k, float l,
              float m, float n, float o, float p) {
        a00 = a;
        a01 = b;
        a02 = c;
        a03 = d;
        a10 = e;
        a11 = f;
        a12 = g;
        a13 = h;
        a20 = i;
        a21 = j;
        a22 = k;
        a23 = l;
        a30 = m;
        a31 = n;
        a32 = o;
        a33 = p;
      }

      void set(float a, float b, float c, float d,
               float e, float f, float g, float h,
               float i, float j, float k, float l,
               float m, float n, float o, float p)
      {
        a00 = a;
        a01 = b;
        a02 = c;
        a03 = d;
        a10 = e;
        a11 = f;
        a12 = g;
        a13 = h;
        a20 = i;
        a21 = j;
        a22 = k;
        a23 = l;
        a30 = m;
        a31 = n;
        a32 = o;
        a33 = p;
      }


      /*! Does matrix multiplication. */
      Matrix4  operator *  ( const Matrix4& ) const;

      /*! Does matrix multiplication. */
      Matrix4& operator *= ( const Matrix4& );

      /*! Add each matching locations. */
      Matrix4  operator +  ( const Matrix4& ) const;

      /*! Add each matching locations. */
      Matrix4& operator += ( const Matrix4& );

      /*! Subtract each matching locations. */
      Matrix4  operator -  ( const Matrix4& ) const;

      /*! Subtract each matching locations. */
      Matrix4& operator -= ( const Matrix4& );

      /*! Multiplies every location by f. For transforms this is not a scale,
          because it scales the W parameter as well, producing no change. */
      Matrix4  operator *  (float) const;

      /*! Multiplies every location by f. For transforms this is not a scale,
          because it scales the W parameter as well, producing no change. */
      Matrix4& operator *= (float);
      Matrix4  operator /  (float d) const { return (*this) * (1 / d); }
      Matrix4& operator /= (float d) { return (*this) *= (1 / d); }

      Vector4  operator *  (const Vector4& v) const
      {
        return Vector4(
                 a00 * v.x + a01 * v.y + a02 * v.z + a03 * v.w,
                 a10 * v.x + a11 * v.y + a12 * v.z + a13 * v.w,
                 a20 * v.x + a21 * v.y + a22 * v.z + a23 * v.w,
                 a30 * v.x + a31 * v.y + a32 * v.z + a33 * v.w
                 );
      }

      /*! Same as this*v. */
      Vector4 transform(const Vector4& v) const { return (*this) * v; }

      /*!
         Same as the xyz of transform(v,1). This will transform a point
         in space but \e only if this is not a perspective matrix, meaning
         the last row is 0,0,0,1.
       */
      Vector3 transform(const Vector3& v) const
      {
        return Vector3(
                 a00 * v.x + a01 * v.y + a02 * v.z + a03,
                 a10 * v.x + a11 * v.y + a12 * v.z + a13,
                 a20 * v.x + a21 * v.y + a22 * v.z + a23
                 );
      }
      
      /*!
         Will transform a Vector2 assuming z = 0.0
       */
      
      Vector2 transform(const Vector2& v) const
      {
        return Vector2(
              a00 * v.x + a01 * v.y + a03,
              a10 * v.x + a11 * v.y + a13
              );
      }
      

      /*!
         Same as the xyz of transform(v,0). This will transform a vector
         in space but \e only if this is not a perspective matrix, meaning
         the last row is 0,0,0,1.
       */
      Vector3 vtransform(const Vector3& v) const
      {
        return Vector3(
                 a00 * v.x + a01 * v.y + a02 * v.z,
                 a10 * v.x + a11 * v.y + a12 * v.z,
                 a20 * v.x + a21 * v.y + a22 * v.z
                 );
      }

      /*!
         Same as transpose().transform(v,0). If this is the inverse of
         a transform matrix, this will transform normals.
       */
      Vector3 ntransform(const Vector3& v) const
      {
        return Vector3(
                 a00 * v.x + a10 * v.y + a20 * v.z,
                 a01 * v.x + a11 * v.y + a21 * v.z,
                 a02 * v.x + a12 * v.y + a22 * v.z
                 );
      }

      /*!
         Same as this*Vector4(v.x,v.y,v.z,w). Useful for doing transforms
         when w is stored in a different location than the xyz.
       */
      Vector4 transform(const Vector3& v, float w) const
      {
        return Vector4(
                 a00 * v.x + a01 * v.y + a02 * v.z + a03 * w,
                 a10 * v.x + a11 * v.y + a12 * v.z + a13 * w,
                 a20 * v.x + a21 * v.y + a22 * v.z + a23 * w,
                 a30 * v.x + a31 * v.y + a32 * v.z + a33 * w
                 );
      }

      /*! \fn bool Matrix4::operator!=(const Matrix4& b) const
          Returns true if any of all 16 locations are different. */
      bool operator != ( const Matrix4& b) const { return memcmp(&a00, &b.a00, DD_IMAGE_MATRXI4_BYTE_SIZE) != 0; }

      /*! \fn bool Matrix4::operator==(const Matrix4& b) const
          Returns true if all 16 locations are equal. */
      bool operator == ( const Matrix4& b) const { return !memcmp(&a00, &b.a00, DD_IMAGE_MATRXI4_BYTE_SIZE); }

      /*! \fn float Matrix4::determinant(void) const
         Return the determinant. Non-zero means the transformation can be
         inverted.
       */
      float determinant(void) const
      {
        return
          a01 * a23 * a32 * a10 - a01 * a22 * a33 * a10 - a23 * a31 * a02 * a10 + a22 * a31 * a03 * a10
          - a00 * a23 * a32 * a11 + a00 * a22 * a33 * a11 + a23 * a30 * a02 * a11 - a22 * a30 * a03 * a11
          - a01 * a23 * a30 * a12 + a00 * a23 * a31 * a12 + a01 * a22 * a30 * a13 - a00 * a22 * a31 * a13
          - a33 * a02 * a11 * a20 + a32 * a03 * a11 * a20 + a01 * a33 * a12 * a20 - a31 * a03 * a12 * a20
          - a01 * a32 * a13 * a20 + a31 * a02 * a13 * a20 + a33 * a02 * a10 * a21 - a32 * a03 * a10 * a21
          - a00 * a33 * a12 * a21 + a30 * a03 * a12 * a21 + a00 * a32 * a13 * a21 - a30 * a02 * a13 * a21;
      }

      /*!
          Returns the inverse of this matrix where \a det is the precomputed
          determinant. This is useful if you already computed the determinant
          in order to see if you can invert the matrix.
       */
      Matrix4 inverse(float det) const;

      /*! \fn Matrix4 Matrix4::inverse() const
          Returns the inverse of this matrix. If determinant() is zero
          this will replace all the items with +/- infinity or zero.
       */
      Matrix4 inverse() const { return inverse(determinant()); }

      static const Matrix4& identity() 
      {
        static const Matrix4 _identity(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
        return _identity;
      }

      bool isIdentity() const
      {
        return *this == identity();
      }

      /*! Replace the contents with the identity. */
      void makeIdentity() { memcpy(&a00, &identity().a00, DD_IMAGE_MATRXI4_BYTE_SIZE); }

      /*! Replace the contents with a uniform scale by \a x. */
      void scaling(float);

      /*! Replace the contents with a scale by \a x,y,z. */
      void scaling(float, float, float);

      /*! \fn void Matrix4::scaling(const Vector3& v)
          Replace the contents with a scale by the x,y,z of the vector
       */
      void scaling(const Vector3& v) { scaling(v.x, v.y, v.z); }

      /*! Replace the contents with a translation by \a x,y,z. */
      void translation(float, float, float = 0.0f);
 
      /*! Replace the contents with a translation by the x,y,z of the vector */
      void translation(const Vector3& v) { translation(v.x, v.y, v.z); }

      /*! Replace the contents with a rotation by angle (in radians) around
          the X axis. */
      void rotationX(float);

      /*! Replace the contents with a rotation by angle (in radians) around
          the Y axis. */
      void rotationY(float);

      /*! Replace the contents with a rotation by angle (in radians) around
          the Z axis. */
      void rotationZ(float);

      /*! \fn Matrix4& Matrix4::rotation(float angle)
          Same as rotationZ(angle). */
      void rotation(float a) { rotationZ(a); }

      /*! Replace the contents with a rotation by angle (in radians) around
          the vector x,y,z. */
      void rotation(float a, float x, float y, float z);

      /*! \fn void Matrix4::rotation(float angle, const Vector3& v)
         Replace the contents with a rotation by angle (in radians) around
         the vector.
       */
      void rotation(float a, const Vector3& v) { rotation(a, v.x, v.y, v.z); }

      /*! Replace the contents with a camera projection. The camera is
          sitting at 0,0,0 and pointing along the Z axis, and the ratio of
          it's focal length to the width of the film is \a lens.

          The area
          that will appear on the film is transformed to be in a square with
          X and Y ranging from -1 to 1. The plane Z==minz is transformed to
          be at Z==-1 and the plane Z==maxz is transformed to Z==1.
       */
      void projection(float lens, float minz, float maxz, bool persp = true);

      // destructive modifications:

      /*! Replace the contents with the transposition (reflect through diagonal) */
      void transpose();

      /*! Scale the transformation by \a s. */
      void scale(float);

      /*! Scale the transformation by \a x,y,z. */
      void scale(float, float, float = 1);

      /*! \fn void Matrix4::scale(const Vector3& v)
          Scale the transformation by the x,y,z of the vector
       */
      void scale(const Vector3& v) { scale(v.x, v.y, v.z); }

      /*! Translate the transformation by \a x,y,z. */
      void translate(float, float, float = 0.0f);

      /*! \fn Matrix4 & Matrix4::translation(const Vector3 & v)
          Translate the transformation by the x,y,z of the vector. */
      void translate(const Vector3& v) { translate(v.x, v.y, v.z); }

      /*! Rotate the transformation by \a a radians about the X axis. */
      void rotateX(float);
  
      /*! Rotate the transformation by \a a radians about the Y axis. */
      void rotateY(float);

      /*! Rotate the transformation by \a a radians about the Z axis. */
      void rotateZ(float);

      /*! \fn void Matrix4::rotate(float a)
          Same as rotateZ(a). */
      void rotate(float a) { rotateZ(a); }

      /*! Rotate the transformation by \a a radians about the vector x,y,z. */
      void rotate(float a, float x, float y, float z);

      /*! \fn void Matrix4::rotate(float a, const Vector4& v)
          Rotate the transformation by \a a radians about the vector. */
      void rotate(float a, const Vector4& v) { rotate(a, v.x, v.y, v.z); }
      void rotate(RotationOrder order, const Vector3& rot);

      /*! Skew the transformation by \a a (X positions have a*Y added to them). */
      void skew(float a);
      void skewXY(float x, float y);
      void skewYX(float x, float y);
      void skewVec(const Vector3 &skew);

      /*! Return the transformation of a 1 unit vector in x, if this is not a persxpective matrix (ie bottom row must be 0,0,0,1). */
      Vector3 x_axis() const { return Vector3(a00, a10, a20); }
      /*! Return the transformation of a 1 unit vector in y, if this is not a persxpective matrix (ie bottom row must be 0,0,0,1). */
      Vector3 y_axis() const { return Vector3(a01, a11, a21); }
      /*! Return the transformation of a 1 unit vector in z, if this is not a persxpective matrix (ie bottom row must be 0,0,0,1). */
      Vector3 z_axis() const { return Vector3(a02, a12, a22); }
      /*! Return the transformation of the point 0,0,0, if this is not a perspective matrix (ie bottom row is 0,0,0,1). */
      Vector3 translation() const { return Vector3(a03, a13, a23); }

      void setXAxis(const Vector3& axis) 
      {
        a00 = axis.x;
        a10 = axis.y;
        a20 = axis.z;
      }

      void setYAxis(const Vector3& axis) 
      {
        a01 = axis.x;
        a11 = axis.y;
        a21 = axis.z;
      }

      void setZAxis(const Vector3& axis) 
      {
        a02 = axis.x;
        a12 = axis.y;
        a22 = axis.z;
      }

      /*! Return the scale of a transformation matrix.
       */
      Vector3 scale() const;

      /*! Modify the transformation matrix to represent the scale component only.
       */
      void scaleOnly();

      /*! Modify the transformation matrix to represent the rotation component only.
       */
      void rotationOnly();

      /*! Modify the transformation matrix to represent the translation component only.
       */
      void translationOnly();

      /*! Modify the transformation matrix to represent the scale and rotation component only.
       */
      void scaleAndRotationOnly();

      /*! Given a matrix which is assumed to be the multiplication of any
          number of rotation matricies (no translations, please) calculate
          the equivalent rotations around the X, Y and Z axis.  You can then
          recreate this transformation by doing rotatez(x), rotatex(y), rotatey(z).
       */
      void rotationsXYZ(float& rx, float& ry, float& rz) const;

      /*! Given a matrix which is assumed to be the multiplication of any
          number of rotation matricies (no translations, please) calculate
          the equivalent rotations around the X, Y and Z axis.  You can then
          recreate this transformation by doing rotatez(x), rotatex(z), rotatey(y).
       */
      void rotationsXZY(float& rx, float& ry, float& rz) const;

      /*! Given a matrix which is assumed to be the multiplication of any
          number of rotation matricies (no translations, please) calculate
          the equivalent rotations around the X, Y and Z axis.  You can then
          recreate this transformation by doing rotatez(y), rotatex(x), rotatey(z).
       */
      void rotationsYXZ(float& rx, float& ry, float& rz) const;

      /*! Given a matrix which is assumed to be the multiplication of any
          number of rotation matricies (no translations, please) calculate
          the equivalent rotations around the X, Y and Z axis.  You can then
          recreate this transformation by doing rotatez(y), rotatex(z), rotatey(x).
       */
      void rotationsYZX(float& rx, float& ry, float& rz) const;

      /*! Given a matrix which is assumed to be the multiplication of any
          number of rotation matricies (no translations, please) calculate
          the equivalent rotations around the X, Y and Z axis.  You can then
          recreate this transformation by doing rotatez(z), rotatex(x), rotatey(y).
       */
      void rotationsZXY(float& rx, float& ry, float& rz) const;

      /*! Given a matrix which is assumed to be the multiplication of any
          number of rotation matricies (no translations, please) calculate
          the equivalent rotations around the X, Y and Z axis.  You can then
          recreate this transformation by doing rotatez(z), rotatex(y), rotatey(x).
       */
      void rotationsZYX(float& rx, float& ry, float& rz) const;

      /*! Given a matrix which is assumed to be the multiplication of any
          number of rotation matricies (no translations, please) calculate
          the equivalent rotations around the X, Y and Z axis.  You can then
          recreate this transformation by concatenating individual rotations
          in the order specified by the RotationOrder parameter
       */
      void getRotations(RotationOrder order, float& rx, float& ry, float& rz) const;

      /*! Get and remove scale and rotation from the matrix. 
       */
      bool extractAndRemoveScalingAndShear(DD::Image::Vector3& scaleOut, DD::Image::Vector3& shearOut);

      /*! Decompose the matrix into its individual transform components. 
          Returned rotation values are based on the rotation order provided.
       */
      bool decompose( DD::Image::Vector3& rotationOut,
                      DD::Image::Vector3& translationOut,
                      DD::Image::Vector3& scaleOut,
                      DD::Image::Vector3& shearOut,
                      const DD::Image::Matrix4::RotationOrder rOrder) const;

      //! Corner pinning: map 0,0,1,1 square to the four corners (anticlockwise from bottom left)
      /*! map 0,0,1,1 square to the four corners (anticlockwise from bottom left)
       */
      void mapUnitSquareToQuad( float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3 );
      
      /*! Corner pinning: map the four corners (anticlockwise from bottom left) to 0,0,1,1 square */
      void mapQuadToUnitSquare( float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3 );

      /*! Add this to the Hash object. */
      void append(Hash&) const;


      /*! Return whether all of the components are valid numbers. */
      bool isValid() const 
      {
        if (isNaN(a00) || isNaN(a10) || isNaN(a20) || isNaN(a30) ||
            isNaN(a01) || isNaN(a11) || isNaN(a21) || isNaN(a31) ||
            isNaN(a02) || isNaN(a12) || isNaN(a22) || isNaN(a32) ||
            isNaN(a03) || isNaN(a13) || isNaN(a23) || isNaN(a33))
          return false;
        return true;
      }

      void set( TransformOrder tOrder, 
                RotationOrder rOrder,
                const Vector3& pivot,
                const Vector3& translation,
                const Vector3& rotation,
                const Vector3& vScale,
                const Vector3& vSkew)
      {
        makeIdentity();

        // translate pivot to origin
        translate(pivot.x, pivot.y, pivot.z);

        // do the transform:
        switch (tOrder) {
          case eSRT:
            translate(translation.x, translation.y, translation.z);
            rotate(rOrder, rotation);
            skewVec(vSkew);
            scale(vScale.x, vScale.y, vScale.z);
            break;
          case eSTR:
            rotate(rOrder, rotation);
            skewVec(vSkew);
            translate(translation.x, translation.y, translation.z);
            scale(vScale.x, vScale.y, vScale.z);
            break;
          case eRST:
            translate(translation.x, translation.y, translation.z);
            scale(vScale.x, vScale.y, vScale.z);
            rotate(rOrder, rotation);
            skewVec(vSkew);
            break;
          case eRTS:
            scale(vScale.x, vScale.y, vScale.z);
            translate(translation.x, translation.y, translation.z);
            rotate(rOrder, rotation);
            skewVec(vSkew);
            break;
          case eTSR:
            rotate(rOrder, rotation);
            skewVec(vSkew);
            scale(vScale.x, vScale.y, vScale.z);
            translate(translation.x, translation.y, translation.z);
            break;
          case eTRS:
            scale(vScale.x, vScale.y, vScale.z);
            rotate(rOrder, rotation);
            skewVec(vSkew);
            translate(translation.x, translation.y, translation.z);
            break;
        }

        // translate origin back to pivot
        translate(-pivot.x, -pivot.y, -pivot.z);
      }
    };

    /*! Writes it in nuke/tcl notation, with nested curly braces */
    DDImage_API std::ostream& operator << (std::ostream& o, const Matrix4& matrix );

    //! convert a DD::Image::Matrix4 to a std::vector<double>
    //! transposes from column-major to row-major
    inline std::vector<double> Matrix4ToVector(const Matrix4& matrix)
    {
      std::vector<double> ret(DD_IMAGE_MATRXI4_SIZE);
      for (int i = 0; i < DD_IMAGE_MATRXI4_SIZE; i++) {
        ret[i] = matrix[i % 4][i / 4];
      }
      return ret;
    }
    
    //! convert a std::vector<double> to a DD::Image::Matrix4
    //! transposes from row-major to column-major
    inline Matrix4 VectorToMatrix4(const std::vector<double>& matrix)
    {
      assert(matrix.size() == DD_IMAGE_MATRXI4_SIZE);
      Matrix4 ret;
      for (int i = 0; i < DD_IMAGE_MATRXI4_SIZE ; i++) {
        ret[i % 4][i / 4] = float(matrix[i]);
      }
      return ret;
    }

    //! convert a DD::Image::Matrix4 to an array of doubles
    //! transposes from column-major to row-major
    inline void Matrix4ToArray(Matrix4& matrix, double* arr)
    {
      matrix.transpose();
      std::copy(matrix.array(), matrix.array() + DD_IMAGE_MATRXI4_SIZE, arr);
      matrix.transpose();
    }

    //! convert a DD::Image::Matrix4 to an array of doubles
    //! ignores 3rd row
    //! transposes from column-major to row-major
    inline void Matrix3x4ToArray(Matrix4& matrix, double* arr)
    {
      matrix.transpose();
      std::copy(matrix.array(), matrix.array() + 3 * 4, arr);
      matrix.transpose();
      arr[8] = matrix.a30;
      arr[9] = matrix.a31;
      arr[10] = matrix.a32;
      arr[11] = matrix.a33;
    }
  }
}

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
// end of Matrix4.h
