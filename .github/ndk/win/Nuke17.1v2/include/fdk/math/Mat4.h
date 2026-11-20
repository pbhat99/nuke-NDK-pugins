// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FDK_MATH_MAT4_H
#define FDK_MATH_MAT4_H

#include "fdk/math/Vec2.h"
#include "fdk/math/Vec3.h"
#include "fdk/math/Vec4.h"

#include <limits>

namespace fdk {

  template <typename T> class Box2; // include Box2.h *after* Mat4 class definition
  template <typename T> class Box3; // include Box3.h *after* Mat4 class definition


  //------------------------------------------------------------------------------


  // These match the transform order enums in DD::Image (Axis_KnobI, Matrix4)
  enum class XformOrder : int32_t
  {
    SRT,
    STR,
    RST,
    RTS,
    TSR,
    TRS
  };

  // See Vec3.h for RotateOrder enums
  //enum class RotateOrder

  //------------------------------------------------------------------------------


  //! \class fdk::Mat4
  //! \brief A 4x4 transformation matrix. You multiply a Vec4 by one of these
  //! to go from a transformed space to normal space.
  //!
  //! The data is stored packed together in OpenGL order, which is transposed
  //! from the way used in most modern graphics literature. This affects
  //! how the array() and [] operator work. You can directly access the
  //! entries with the aRC members, where R is the row and C is the column.
  //!
  //! For instance matrix.a03 is the top-right corner of the matrix in
  //! most literature. It is multiplied by the W of a vector to produce
  //! part of the X of the output vector, and can be considered the X
  //! translation of the matrix.
  //!
  //! However matrix.a03 is matrix[3][0], and is matrix.array()[12].
  //!
  //! a00, a10, a20, a30,
  //! a01, a11, a21, a31,
  //! a02, a12, a22, a32,
  //! a03, a13, a23, a33
  template<typename T>
  class Mat4
  {
  public:
    static constexpr uint8_t kNumElements = 16; //< The number of elements in the matrix

    // Matrix is stored in transposed order where a03 is a<row 0><col 3>
    // and a32 is a<row 3><col 2>:
    T
    /*        row0 row1 row2 row3 */
    /*col 0*/ a00, a10, a20, a30,
    /*col 1*/ a01, a11, a21, a31,
    /*col 2*/ a02, a12, a22, a32,
    /*col 3*/ a03, a13, a23, a33;


    /*---------------------------------------------------------*/
    /*                      Constructors                       */
    /*---------------------------------------------------------*/
    //! Default constructor leaves garbage in contents.
    Mat4() = default;

    //! Copy constructor supports conversion from another type with optional
    //! transpose-on-copy.
    template<typename S>
    explicit Mat4(const Mat4<S>& b,
                  bool           transpose = false) { setToArray(b.array(), transpose); }

    //! Copy from array constructor supports conversion from another type with
    //! optional transpose-on-copy.
    template<typename S>
    explicit Mat4(const S* src,
                  bool     transpose = false) { setToArray(src, transpose); }

    //! Initialize to identity matrix with a constant in the diagonal.
    explicit Mat4(T d);

    //! Initialize with a00=a, a01=b, a02=c, etc. ie the arguments are given as rows.
    Mat4(T a, T b, T c, T d,
         T e, T f, T g, T h,
         T i, T j, T k, T l,
         T m, T n, T o, T p, bool transpose=false) {
            setTo(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, transpose); 
        }

    //! Transmogrify as a specific type:
    operator Mat4<float>()  const;
    operator Mat4<double>() const;

    Mat4<float>  asMat4f() const;
    Mat4<double> asMat4d() const;


    /*---------------------------------------------------------*/
    /*                     Component Access                    */
    /*---------------------------------------------------------*/
    //! Return a pointer to the start of column 'c'.
    T*       operator [] (int col)       { return &a00 + col * 4; }
    const T* operator [] (int col) const { return &a00 + col * 4; }

    //! Return a pointer to the first element a00.
    T*           array()       { return &a00; }
    const T*     array() const { return &a00; }

    //! Return the value of matrix element a00 + 'i'.
    T&           element(int i)       { return *(&a00 + i); }
    const T&     element(int i) const { return *(&a00 + i); }

    //! Return row 0 as a Vec4.
    Vec4<T>      row0() const { return Vec4<T>(a00, a01, a02, a03); }

    //! Return row 1 as a Vec4.
    Vec4<T>      row1() const { return Vec4<T>(a10, a11, a12, a13); }

    //! Return row 2 as a Vec4.
    Vec4<T>      row2() const { return Vec4<T>(a20, a21, a22, a23); }

    //! Return row 3 as a Vec4.
    Vec4<T>      row3() const { return Vec4<T>(a30, a31, a32, a33); }

    //! Return column 0 as a Vec4.
    Vec4<T>      col0() const { return Vec4<T>(a00, a10, a20, a30); }

    //! Return column 1 as a Vec4.
    Vec4<T>      col1() const { return Vec4<T>(a01, a11, a21, a31); }

    //! Return column 2 as a Vec4.
    Vec4<T>      col2() const { return Vec4<T>(a02, a12, a22, a32); }

    //! Return column 3 as a Vec4.
    Vec4<T>      col3() const { return Vec4<T>(a03, a13, a23, a33); }

    //! Assign row 0, 1, or 2 from a Vec3.
    template<typename S>
    void         setRow0(const Vec3<S>& v);

    template<typename S>
    void         setRow1(const Vec3<S>& v);

    template<typename S>
    void         setRow2(const Vec3<S>& v);

    //! Return col 0, 1 or 2 as a Vec3.
    template<typename S>
    void         xAxis(Vec3<S>& v) const;
    Vec3<T>      xAxis()           const { return Vec3<T>(a00, a10, a20); }

    template<typename S>
    void         yAxis(Vec3<S>& v) const;
    Vec3<T>      yAxis()           const { return Vec3<T>(a01, a11, a21); }

    template<typename S>
    void         zAxis(Vec3<S>& v) const;
    Vec3<T>      zAxis()           const { return Vec3<T>(a02, a12, a22); }

    //! Assign col 0, 1, or 2 from a Vec3.
    template<typename S>
    void         setXAxis(const Vec3<S>& v);

    template<typename S>
    void         setYAxis(const Vec3<S>& v);

    template<typename S>
    void         setZAxis(const Vec3<S>& v);

    template<typename S>
    void         setXYZAxis(const Vec3<S>& x,
                            const Vec3<S>& y,
                            const Vec3<S>& z);

    //! Return the translations or scale of the matrix as a Vec3.
    template<typename S>
    void         getTranslation(Vec3<S>& v) const;
    Vec3<T>      getTranslation()           const { return Vec3<T>(a03, a13, a23); }

    template<typename S>
    void         getScaleAxis(Vec3<S>& v) const;
    Vec3<T>      getScaleAxis()           const { return Vec3<T>(a00, a11, a22); }

    //! Extract the rotation angles (in radians) from the matrix.
    //! The matrix is assumed to have no shear, non-uniform scaling is handled.
    Vec3<T>      getRotations(fdk::RotateOrder order) const;
    void         getRotations(fdk::RotateOrder order,
                              T&               radian_rX,
                              T&               radian_rY,
                              T&               radian_rZ) const;

    //! Return the scaling of the matrix.
    Vec3<T>      getScale() const
    {
      return Vec3<T>(std::sqrt(a00*a00 + a10*a10 + a20*a20 + a30*a30),
                     std::sqrt(a01*a01 + a11*a11 + a21*a21 + a31*a31),
                     std::sqrt(a02*a02 + a12*a12 + a22*a22 + a32*a32));
    }

    //! Return whether all of the components are valid numbers.
    bool         isValid() const;


    /*---------------------------------------------------------*/
    /*                        Assignment                       */
    /*---------------------------------------------------------*/
    //! Set all components from a like-sized array with per-element type conversion
    //! and optional transpose-on-copy.
    template<typename S>
    void         setToArray(const S* src,
                            bool     transpose = false);

    //! Set each component in memory order, unless transposed.
    template<typename S>
    void         setTo(S a, S b, S c, S d,
                       S e, S f, S g, S h,
                       S i, S j, S k, S l,
                       S m, S n, S o, S p, bool transpose = false);

    //! Set all components to 0 or 1.
    void         setToZero() { std::fill(&a00, (&a00) + 16, static_cast<T>(0)); }
    void         setToOne();

    //! Type-specific clear. Sets all components to 0.
    void         clear() { setToZero(); }

    //! Add this to a fdk::Hash object.
    void         append(fdk::Hash& hash) const;


    /*---------------------------------------------------------*/
    /*                       Multiplication                    */
    /*---------------------------------------------------------*/
    Mat4  operator *  (const Mat4& b) const
    {
      Mat4<T> m;
      m.a00 = a00*b.a00 + a01*b.a10 + a02*b.a20 + a03*b.a30;
      m.a10 = a10*b.a00 + a11*b.a10 + a12*b.a20 + a13*b.a30;
      m.a20 = a20*b.a00 + a21*b.a10 + a22*b.a20 + a23*b.a30;
      m.a30 = a30*b.a00 + a31*b.a10 + a32*b.a20 + a33*b.a30;

      m.a01 = a00*b.a01 + a01*b.a11 + a02*b.a21 + a03*b.a31;
      m.a11 = a10*b.a01 + a11*b.a11 + a12*b.a21 + a13*b.a31;
      m.a21 = a20*b.a01 + a21*b.a11 + a22*b.a21 + a23*b.a31;
      m.a31 = a30*b.a01 + a31*b.a11 + a32*b.a21 + a33*b.a31;

      m.a02 = a00*b.a02 + a01*b.a12 + a02*b.a22 + a03*b.a32;
      m.a12 = a10*b.a02 + a11*b.a12 + a12*b.a22 + a13*b.a32;
      m.a22 = a20*b.a02 + a21*b.a12 + a22*b.a22 + a23*b.a32;
      m.a32 = a30*b.a02 + a31*b.a12 + a32*b.a22 + a33*b.a32;

      m.a03 = a00*b.a03 + a01*b.a13 + a02*b.a23 + a03*b.a33;
      m.a13 = a10*b.a03 + a11*b.a13 + a12*b.a23 + a13*b.a33;
      m.a23 = a20*b.a03 + a21*b.a13 + a22*b.a23 + a23*b.a33;
      m.a33 = a30*b.a03 + a31*b.a13 + a32*b.a23 + a33*b.a33;
      return m;
    }

    Mat4& operator *= (const Mat4& b)
    {
      Mat4<T> m;
      m.a00 = a00*b.a00 + a01*b.a10 + a02*b.a20 + a03*b.a30;
      m.a10 = a10*b.a00 + a11*b.a10 + a12*b.a20 + a13*b.a30;
      m.a20 = a20*b.a00 + a21*b.a10 + a22*b.a20 + a23*b.a30;
      m.a30 = a30*b.a00 + a31*b.a10 + a32*b.a20 + a33*b.a30;

      m.a01 = a00*b.a01 + a01*b.a11 + a02*b.a21 + a03*b.a31;
      m.a11 = a10*b.a01 + a11*b.a11 + a12*b.a21 + a13*b.a31;
      m.a21 = a20*b.a01 + a21*b.a11 + a22*b.a21 + a23*b.a31;
      m.a31 = a30*b.a01 + a31*b.a11 + a32*b.a21 + a33*b.a31;

      m.a02 = a00*b.a02 + a01*b.a12 + a02*b.a22 + a03*b.a32;
      m.a12 = a10*b.a02 + a11*b.a12 + a12*b.a22 + a13*b.a32;
      m.a22 = a20*b.a02 + a21*b.a12 + a22*b.a22 + a23*b.a32;
      m.a32 = a30*b.a02 + a31*b.a12 + a32*b.a22 + a33*b.a32;

      m.a03 = a00*b.a03 + a01*b.a13 + a02*b.a23 + a03*b.a33;
      m.a13 = a10*b.a03 + a11*b.a13 + a12*b.a23 + a13*b.a33;
      m.a23 = a20*b.a03 + a21*b.a13 + a22*b.a23 + a23*b.a33;
      m.a33 = a30*b.a03 + a31*b.a13 + a32*b.a23 + a33*b.a33;
      *this = m;
      return *this;
    }

    //! Multiplies every location by d. For transforms this is not a scale,
    //! because it scales the W parameter as well, producing no change.
    Mat4  operator *  (T d) const
    {
      Mat4 C;
      const T* a = &a00;
      T* c = &C.a00;
      T* e = c + 16;
      while (c < e) { *c++ = *a++ * d; }
      return C;
    }

    Mat4& operator *= (T d)
    {
      T* a = &a00;
      T* e = a + 16;
      while (a < e) { *a++ *= d; }
      return *this;
    }

    Mat4  operator /  (T d) const { return (*this) *  (static_cast<T>(1) / d); }
    Mat4& operator /= (T d)       { return (*this) *= (static_cast<T>(1) / d); }

    //! Add each matching location.
    Mat4  operator +  (const Mat4& B) const
    {
      Mat4 C;
      const T* a = &a00;
      const T* b = &B.a00;
      T* c = &C.a00;
      T* e = c + 16;
      while (c < e) { *c++ = *a++ + *b++; }
      return C;
    }

    Mat4& operator += (const Mat4& B)
    {
      T* a = &a00;
      T* e = a + 16;
      const T* b = &B.a00;
      while (a < e) { *a++ += *b++; }
      return *this;
    }

    //! Subtract each matching location.
    Mat4  operator -  (const Mat4& B) const
    {
      Mat4 C;
      const T* a = &a00;
      const T* b = &B.a00;
      T* c = &C.a00;
      T* e = c + 16;
      while (c < e) { *c++ = *a++ - *b++; }
      return C;
    }

    Mat4& operator -= (const Mat4& B)
    {
      T* a = &a00;
      T* e = a + 16;
      const T* b = &B.a00;
      while (a < e) { *a++ -= *b++; }
      return *this;
    }


    /*---------------------------------------------------------*/
    /*                          Equality                       */
    /*---------------------------------------------------------*/
    //! Returns true if all 16 locations are equal. Compares on each element individually.
    bool operator == (const Mat4& b) const;

    //! Returns true if any of the 16 locations are different. Compares on each element individually.
    bool operator != (const Mat4& b) const { return !(*this == b); }


    /*---------------------------------------------------------*/
    /*                   Vector Multiplication                 */
    /*---------------------------------------------------------*/
    template<typename S, typename R>
    Vec3<R>&     transform(const Vec3<S>& in,
                           Vec3<R>&       out) const;

    template<typename S>
    Vec3<S>  operator * (const Vec3<S>& v) const { Vec3<S> o; return transform(v, o); }

    template<typename S>
    Vec3<S>      transform(const Vec3<S>& v) const { Vec3<S> o; return transform(v, o); }
    template<typename S>
    void         transform(Vec3<S>*       dstPoints,
                           const Vec3<S>* srcPoints,
                           size_t         nPoints) const;

    template<typename S, typename R>
    Vec4<R>&     transform(const Vec4<S>& in,
                           Vec4<R>&       out) const;

    template<typename S>
    Vec4<S>  operator * (const Vec4<S>& v) const { Vec4<S> o; return transform(v, o); }

    template<typename S>
    Vec4<S>      transform(const Vec4<S>& v) const { return *this * v; }

    template<typename S, typename R>
    Vec4<S>      transform(const Vec3<S>& v,
                           R              w) const { return *this * Vec4<S>(v, w); }

    //! Transform a Vec2 assuming z = 0.0.
    template<typename S>
    Vec2<S>      transform(const Vec2<S>& v) const;

    //! Transform a Vec2 with no translation applied, assuming z = 0.0.
    template<typename S>
    Vec2<S>      vecTransform(const Vec2<S>& v) const;


    //! Transform a vector with no translation applied.
    template<typename S>
    Vec3<S>      vecTransform(const Vec3<S>& v) const;

    template<typename S>
    void         vecTransform(Vec3<S>*       dstVecs,
                              const Vec3<S>* srcVecs,
                              size_t         nVecs) const;

    //! Transform a normal - same as transpose().transform(n).
    template<typename S>
    Vec3<S>      normalTransform(const Vec3<S>& n) const;

    template<typename S>
    void         normalTransform(Vec3<S>*       dstNormals,
                                 const Vec3<S>* srcNormalss,
                                 size_t         nNormals) const;


    /*---------------------------------------------------------*/
    /*               Bounding Box Multiplication               */
    /*---------------------------------------------------------*/
    template<typename S, typename R>
    Box3<R>&     transform(const Box3<S>& in,
                           Box3<R>&       out) const;

    template<typename S>
    Box3<S>  operator * (const Box3<S>& bbox) const;

    template<typename S>
    Box3<S>      transform(const Box3<S>& bbox) const;


    /*---------------------------------------------------------*/
    /*                        Inversion                        */
    /*---------------------------------------------------------*/
    //! Return the determinant. Non-zero means the matrix can be inverted.
    T            getDeterminant() const;

    //! Replace the contents of \a out with the inverse of this, where \a det
    //! is the already-calculated determinant of this and must be non-zero.
    //! <i>&out must be a different matrix than this!</i>
    void         invert(Mat4& out,
                        T     determinant) const;
    //! Replace the contents of \a out with the inverse of this and return
    //! the determinant.  If this cannot be inverted \a to is unchanged
    //! and zero is returned.
    //! <i>&out must be a different matrix than this!</i>
    T            invert(Mat4& out) const;

    //! Invert this matrix in place.
    Mat4&        invert();

    //! Returns the inverse of this matrix. Returns garbage if this cannot be inverted.
    Mat4         inverse() const;

    //! Returns the inverse of this matrix (must supply a precomputed non-zero determinant)
    Mat4         inverse(T determinant) const;


    /*---------------------------------------------------------*/
    /*                        Identity                         */
    /*---------------------------------------------------------*/
    //! Return the identity matrix object.
    static const Mat4& getIdentity()
    {
      static Mat4<T> identity = Mat4<T>(1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1);
      return identity;
    }

    //! Set the matrix to the identity.
    void               setToIdentity() { *this = Mat4<T>::getIdentity(); }

    //! Returns true if matrix is an identity matrix.
    bool               isIdentity()    const { return (*this == Mat4<T>::getIdentity()); }
    bool               isNotIdentity() const { return !isIdentity(); }


    /*---------------------------------------------------------*/
    /*                    Scale Assignment                     */
    /*---------------------------------------------------------*/
    //! Set the contents to a uniform scale by \a d.
    Mat4&        setToScale(T d);

    //! Set the contents to a scale by \a x,y,z.
    Mat4&        setToScale(T sx,
                            T sy,
                            T sz = static_cast<T>(1));

    template<typename S>
    Mat4&        setToScale(const Vec3<S>& s) { return setToScale(s.x, s.y, s.z); }


    /*---------------------------------------------------------*/
    /*                 Translation Assignment                  */
    /*---------------------------------------------------------*/
    //! Set the contents to a translation by \a x,y,z.
    Mat4&        setToTranslation(T tx,
                                  T ty,
                                  T tz = static_cast<T>(0));

    template<typename S>
    Mat4&        setToTranslation(const Vec3<S>& t) { return setToTranslation(t.x, t.y, t.z); }


    /*---------------------------------------------------------*/
    /*                   Rotation Assignment                   */
    /*---------------------------------------------------------*/
    //! Set the contents to xyz rotation angles (in radians).
    Mat4&        setToRotations(fdk::RotateOrder order,
                                T                radian_x_angle,
                                T                radian_y_angle,
                                T                radian_z_angle);

    template<typename S>
    Mat4&        setToRotations(fdk::RotateOrder order,
                                const Vec3<S>&   v) { return setToRotations(order, v.x, v.y, v.z); }

    //! Set the contents to an angle (in radians) around the X axis.
    Mat4&        setToRotationX(T radian_angle);

    //! Set the contents to an angle (in radians) around the Y axis.
    Mat4&        setToRotationY(T radian_angle);

    //! Set the contents to an angle (in radians) around the Z axis.
    Mat4&        setToRotationZ(T radian_angle);

    //! Set the contents to an angle (in radians) around about the vector x,y,z.
    template<typename S>
    Mat4&        setToRotation(T              radian_angle,
                               const Vec3<S>& v);

    Mat4&        setToRotation(T radian_angle,
                               T x, T y, T z) { return setToRotation(radian_angle, Vec3<T>(x, y, z)); }


    /*---------------------------------------------------------*/
    /*           Single-step SRT Transform Handling            */
    /*           * rotations are assumed to be in degrees      */
    /*             and are internally converted to radians     */
    /*           * skew always follows rotation                */
    /*---------------------------------------------------------*/
    void         applyXform(fdk::XformOrder  xformOrder,
                            fdk::RotateOrder rotateOrder,
                            const Vec3<T>&   translation,
                            const Vec3<T>&   rotationsInDegrees,
                            const Vec3<T>&   scaling,
                            const Vec3<T>&   skewing,
                            const Vec3<T>&   pivotOriginTranslate,
                            const Vec3<T>&   pivotOriginRotateInDegrees);

    void         setToXform(fdk::XformOrder  xformOrder,
                            fdk::RotateOrder rotateOrder,
                            const Vec3<T>&   translation,
                            const Vec3<T>&   rotationsInDegrees,
                            const Vec3<T>&   scaling,
                            const Vec3<T>&   skewing,
                            const Vec3<T>&   pivotOriginTranslate,
                            const Vec3<T>&   pivotOriginRotateInDegrees);


    /*---------------------------------------------------------*/
    /*                  Destructive Modifiers                  */
    /*---------------------------------------------------------*/
    //! Replace the contents with the transposition (reflect through diagonal)
    void         transpose();

    //! Scale the transformation by uniform scale \a d.
    void         scale(T s)
    {
      a00 *= s; a01 *= s; a02 *= s;
      a10 *= s; a11 *= s; a12 *= s;
      a20 *= s; a21 *= s; a22 *= s;
      a30 *= s; a31 *= s; a32 *= s;
    }

    //! Scale columns 0,1,2 by x,y,z.
    void         scale(T x,
                       T y,
                       T z = static_cast<T>(1))
    {
      a00 *= x; a01 *= y; a02 *= z;
      a10 *= x; a11 *= y; a12 *= z;
      a20 *= x; a21 *= y; a22 *= z;
      a30 *= x; a31 *= y; a32 *= z;
    }

    template<typename S>
    void         scale(const Vec3<S>& sv) { scale(sv.x, sv.y, sv.z); }

    //! Rotate the transformation by an angle (in radians) about the X axis.
    void         rotateX(T radian_angle);

    //! Rotate the transformation by an angle (in radians) about the Y axis.
    void         rotateY(T radian_angle);

    //! Rotate the transformation by an angle (in radians) about the Z axis.
    void         rotateZ(T radian_angle);

    //! Apply rotations in each axis (in radians) in specific order.
    void         rotateInOrder(fdk::RotateOrder order,
                               T                radian_x_angle,
                               T                radian_y_angle,
                               T                radian_z_angle);

    template<typename S>
    void         rotateInOrder(fdk::RotateOrder order,
                               const Vec3<S>&   radian_angles) { rotateInOrder(order, radian_angles.x, radian_angles.y, radian_angles.z); }

    //! Rotate the transformation by an angle (in radians) about the vector x,y,z.
    template<typename S>
    void         rotateAboutVector(T              radian_angle,
                                   const Vec3<S>& v);

    void         rotateAboutVector(T radian_angle,
                                   T x, T y, T z) { rotateAboutVector(radian_angle, Vec3<T>(x, y, z)); }

    //! Translate the transformation by an x,y,z offset.
    void         translate(T x,
                           T y,
                           T z = static_cast<T>(0))
    {
      a03 += x*a00 + y*a01 + z*a02;
      a13 += x*a10 + y*a11 + z*a12;
      a23 += x*a20 + y*a21 + z*a22;
      a33 += x*a30 + y*a31 + z*a32;
    }

    template<typename S>
    void         translate(const Vec3<S>& v) { translate(v.x, v.y, v.z); }

    //! Skew the transformation by \a a (X positions have a*Y added to them).
    void         skew(T d);

    template<typename S>
    void         skew(const Vec3<S>& skewv);

    void         skewXY(T x, T y);
    void         skewYX(T x, T y);

    //! Component-wise add all the elements.
    Mat4&        add(const Mat4& b);

    //! Add a constant to all the elements.
    void         add(T t);

    //! Add a constant to all the diagonal elements.
    void         addDiagonal(T t);


    //! Calculate the x,y & z rotation angles, in radians, to align the \a align_axis
    //! direction with \a dir_vector, and the suggested rotation order to apply them
    //! in to best avoid gimbal lock.
    //! Return true if \a rotations have been affected.
    template<typename S, typename R>
    static bool  vectorToRotations(const Vec3<S>&     dir_vec,
                                   fdk::AxisDirection align_axis,
                                   bool               do_rx,
                                   bool               do_ry,
                                   bool               do_rz,
                                   Vec3<R>&           rotations,
                                   fdk::RotateOrder&  rotation_order);

    //! Returns a matrix that is equivalent to this matrix rotated so that its 
    //! \a axis is oriented towards \a lookAtPos.
    //! Can interpolate between the two matrices by setting \a strength
    //! to a value between 0 and 1, where 0 simply returns \a sourceMatrix
    //! and 1 returns \a lookAtMatrix.  It can also prevent the rotation
    //! around any axis by setting \a rotateX, \a rotateY or \a rotateZ
    //! to false.
    template<typename S>
    Mat4 lookAt(fdk::AxisDirection axis,
                const fdk::Vec3<S>& lookAtPos,
                T strength = T(1),
                bool rotateX = true,
                bool rotateY = true,
                bool rotateZ = true);

    //! Linear interpolate two matrices at offset 't' which is between 0.0 and 1.0.
    //! This only interpolates position and rotation, and rotation is only valid
    //! within a certain range since it's a linear interpolation of the xyz axes!
    template<typename S>
    void         interpolate(const Mat4<T>& m0,
                             const Mat4<T>& m1,
                             S              t);

    //! Replace the contents with a camera projection. The camera is
    //! sitting at 0,0,0 and pointing along the Z axis, and the ratio of
    //! it's focal length to the width of the film is \a lens.

    //! The area
    //! that will appear on the film is transformed to be in a square with
    //! X and Y ranging from -1 to 1. The plane Z=minz is transformed to
    //! be at Z=-1 and the plane Z=maxz is transformed to Z=1.
    void         setToProjection(double lens,
                                 double minz,
                                 double maxz,
                                 bool   persp = true);



    /*---------------------------------------------------------------------*/
    /*          Component extraction (decomposition) routines.             */
    /*          Some come from ImathMatAlgo or Graphics Gems.              */
    /*---------------------------------------------------------------------*/

    //! Modify the transformation matrix to represent the translation component only.
    void         setToTranslationOnly();

    //! Modify the transformation matrix to represent the rotation component only.
    void         setToRotationOnly();

    //! Modify the transformation matrix to represent the scale component only.
    void         setToScaleOnly();

    //! Modify the transformation matrix to represent the scale and rotation component only.
    void         setToScaleAndRotationOnly();

#if 0
    //!
    bool         extractScaling(Vec3<T>& scale) const;
#endif

    //! Adapted from ilmbase ImathMatrixAlgo.
    bool         extractScalingAndShear(Vec3<T>& scale,
                                        Vec3<T>& shear) const;

    //! Adapted from ilmbase ImathMatrixAlgo.
    bool         extractAndRemoveScalingAndShear(Vec3<T>& scale,
                                                 Vec3<T>& shear);

    //! Rotations are in degrees. Adapted from ilmbase ImathMatrixAlgo.
    bool         extractSHRT(Vec3<T>&         scaling,
                             Vec3<T>&         shearing,
                             Vec3<T>&         rotationsInDegrees,
                             Vec3<T>&         translation,
                             fdk::RotateOrder order = fdk::RotateOrder::ZXY) const;


  private:
    // This must match for Vec/Mat/Box classes:
    static_assert(fdk::is_arithmetic<T>, "Mat4 can only be instantiated with a floating point type.");

  };



  /*--------------------------------------*/
  /*    Aliases for supported data types  */
  /*--------------------------------------*/
  // half not supported
  using Mat4f = Mat4<float>;
  using Mat4d = Mat4<double>;



  /*----------------------------------*/
  /*        Static operations         */
  /*----------------------------------*/

  //! Print out Mat4 components to a stream.
  template<typename T>
  std::ostream& operator << (std::ostream& o, const Mat4<T>& m);


  //! Linear interpolate between two Mat4s at t, where t=0..1.
  template<typename T, typename S>
  FDK_API Mat4<T> lerpClamped(const Mat4<T>& m0,
                              const Mat4<T>& m1,
                              S              t);




  /*---------------------------------------------------------------------*/
  /*---------------------------------------------------------------------*/
  /*                   Inline Function Implementations                   */
  /*---------------------------------------------------------------------*/
  /*---------------------------------------------------------------------*/

  // Note:
  //  Methods with multiple arg type-combos must be implemented inline so the
  //  compiler can resolve the combinations and avoid linking issues.

  template<typename T>
  inline
  Mat4<T>::Mat4(T d)
  {
    setToZero();
    a00 = a11 = a22 = a33 = d;
  }

  template<typename T>
  template<typename S>
  inline void
  Mat4<T>::setToArray(const S* src,
                      bool     transpose)
  {
    if (!src) {
      return;
    }

    if (transpose) {
      // Copy values in transposed order:
      a00 = static_cast<T>(*src++);
      a01 = static_cast<T>(*src++);
      a02 = static_cast<T>(*src++);
      a03 = static_cast<T>(*src++);

      a10 = static_cast<T>(*src++);
      a11 = static_cast<T>(*src++);
      a12 = static_cast<T>(*src++);
      a13 = static_cast<T>(*src++);

      a20 = static_cast<T>(*src++);
      a21 = static_cast<T>(*src++);
      a22 = static_cast<T>(*src++);
      a23 = static_cast<T>(*src++);

      a30 = static_cast<T>(*src++);
      a31 = static_cast<T>(*src++);
      a32 = static_cast<T>(*src++);
      a33 = static_cast<T>(*src++);
    }
    else {
      // No transpose so we can copy if no type conversion:
      if (std::is_same<S, T>::value) {
        std::copy(src, src + 16, &a00);
      }
      else {
        T* mOut = &a00;
        for (int i = 0; i < 16; ++i) {
          *mOut++ = static_cast<T>(*src++);
        }
      }
    }
  }

  template<typename T>
  inline bool
  Mat4<T>::operator == (const Mat4<T>& b) const
  {
    return (
      a00 == b.a00 && a01 == b.a01 && a02 == b.a02 && a03 == b.a03
      && a10 == b.a10 && a11 == b.a11 && a12 == b.a12 && a13 == b.a13
      && a20 == b.a20 && a21 == b.a21 && a22 == b.a22 && a23 == b.a23
      && a30 == b.a30 && a31 == b.a31 && a32 == b.a32 && a33 == b.a33
    );
  }

  template<typename T>
  template<typename S>
  inline void
  Mat4<T>::setRow0(const Vec3<S>& v)
  {
    a00 = v.x;
    a01 = v.y;
    a02 = v.z;
  }

  template<typename T>
  template<typename S>
  inline void
  Mat4<T>::setRow1(const Vec3<S>& v)
  {
    a10 = v.x;
    a11 = v.y;
    a12 = v.z;
  }

  template<typename T>
  template<typename S>
  inline void
  Mat4<T>::setRow2(const Vec3<S>& v)
  {
    a20 = v.x;
    a21 = v.y;
    a22 = v.z;
  }

  template<typename T>
  template<typename S>
  inline void
  Mat4<T>::xAxis(Vec3<S>& v) const
  {
    v.x = S(a00);
    v.y = S(a10);
    v.z = S(a20);
  }

  template<typename T>
  template<typename S>
  inline void
  Mat4<T>::yAxis(Vec3<S>& v) const
  {
    v.x = S(a01);
    v.y = S(a11);
    v.z = S(a21);
  }

  template<typename T>
  template<typename S>
  inline void
  Mat4<T>::zAxis(Vec3<S>& v) const
  {
    v.x = S(a02);
    v.y = S(a12);
    v.z = S(a22);
  }

  template<typename T>
  template<typename S>
  inline void
  Mat4<T>::setXAxis(const Vec3<S>& v)
  {
    a00 = v.x;
    a10 = v.y;
    a20 = v.z;
  }

  template<typename T>
  template<typename S>
  inline void
  Mat4<T>::setYAxis(const Vec3<S>& v)
  {
    a01 = v.x;
    a11 = v.y;
    a21 = v.z;
  }

  template<typename T>
  template<typename S>
  inline void
  Mat4<T>::setZAxis(const Vec3<S>& v)
  {
    a02 = v.x;
    a12 = v.y;
    a22 = v.z;
  }

  template<typename T>
  template<typename S>
  inline void
  Mat4<T>::setXYZAxis(const Vec3<S>& x,
                      const Vec3<S>& y,
                      const Vec3<S>& z)
  {
    setXAxis(x);
    setYAxis(y);
    setZAxis(z);
  }

  template<typename T>
  template<typename S>
  inline void
  Mat4<T>::getTranslation(Vec3<S>& v) const
  {
    v.x = S(a03);
    v.y = S(a13);
    v.z = S(a23);
  }

  template<typename T>
  template<typename S>
  inline void
  Mat4<T>::getScaleAxis(Vec3<S>& v) const
  {
    v.x = S(a00);
    v.y = S(a11);
    v.z = S(a22);
  }

  //------------------------------------------------------------------------------

  template<typename T>
  template<typename S>
  inline Vec2<S>
  Mat4<T>::transform(const Vec2<S>& v) const
  {
    if (std::is_same<S,T>::value) {
      return Vec2<S>(a00 * v.x + a01 * v.y + a03,
                     a10 * v.x + a11 * v.y + a13);
    }
    else {
      const T x = static_cast<T>(v.x);
      const T y = static_cast<T>(v.y);
      return Vec2<S>(a00 * x + a01 * y + a03,
                     a10 * x + a11 * y + a13);
    }
  }

  //------------------------------------------------------------------------------

  template<typename T>
  template<typename S, typename R>
  inline Vec3<R>&
  Mat4<T>::transform(const Vec3<S>& in,
                     Vec3<R>&       out) const
  {
    if (std::is_same<S, T>::value) {
      out.x = static_cast<R>(a00 * in.x + a01 * in.y + a02 * in.z + a03);
      out.y = static_cast<R>(a10 * in.x + a11 * in.y + a12 * in.z + a13);
      out.z = static_cast<R>(a20 * in.x + a21 * in.y + a22 * in.z + a23);
    }
    else {
      const T x = static_cast<T>(in.x);
      const T y = static_cast<T>(in.y);
      const T z = static_cast<T>(in.z);
      out.x = static_cast<R>(a00 * x + a01 * y + a02 * z + a03);
      out.y = static_cast<R>(a10 * x + a11 * y + a12 * z + a13);
      out.z = static_cast<R>(a20 * x + a21 * y + a22 * z + a23);
    }

    return out;
  }

  template<typename T>
  template<typename S>
  inline void
  Mat4<T>::transform(Vec3<S>*       dstPoints,
                     const Vec3<S>* srcPoints,
                     size_t         nPoints) const
  {
    if (isIdentity()) {
      std::copy(srcPoints, srcPoints + nPoints, dstPoints);
    }
    else {
      for (size_t i = 0; i < nPoints; ++i) {
        transform(*srcPoints++, *dstPoints++);
      }
    }
  }

  template<typename T>
  inline Mat4<float>
  Mat4<T>::asMat4f() const
  {
    if (std::is_same<float, T>::value) {
      return *this;
    }

    return Mat4<float>(static_cast<float>(a00), static_cast<float>(a01), static_cast<float>(a02), static_cast<float>(a03),
                       static_cast<float>(a10), static_cast<float>(a11), static_cast<float>(a12), static_cast<float>(a13),
                       static_cast<float>(a20), static_cast<float>(a21), static_cast<float>(a22), static_cast<float>(a23),
                       static_cast<float>(a30), static_cast<float>(a31), static_cast<float>(a32), static_cast<float>(a33), true);
  }

  template<typename T>
  inline Mat4<T>::operator Mat4<float>() const { return asMat4f(); }

  template<typename T>
  inline Mat4<double>
  Mat4<T>::asMat4d() const
  {
    if (std::is_same<double, T>::value) {
      return *this;
    }

    return Mat4<double>(static_cast<double>(a00), static_cast<double>(a01), static_cast<double>(a02), static_cast<double>(a03),
                        static_cast<double>(a10), static_cast<double>(a11), static_cast<double>(a12), static_cast<double>(a13),
                        static_cast<double>(a20), static_cast<double>(a21), static_cast<double>(a22), static_cast<double>(a23),
                        static_cast<double>(a30), static_cast<double>(a31), static_cast<double>(a32), static_cast<double>(a33), true);
  }

  template<typename T>
  inline Mat4<T>::operator Mat4<double>() const { return asMat4d(); }


  template<typename T>
  inline std::ostream& operator << (std::ostream& o, const Mat4<T>& m)
  {
    o << '['
      //        col0            col1            col2            col3
      << '[' << m.a00 << ' ' << m.a01 << ' ' << m.a02 << ' ' << m.a03 << ']' // row0
      << '[' << m.a10 << ' ' << m.a11 << ' ' << m.a12 << ' ' << m.a13 << ']' // row1
      << '[' << m.a20 << ' ' << m.a21 << ' ' << m.a22 << ' ' << m.a23 << ']' // row2
      << '[' << m.a30 << ' ' << m.a31 << ' ' << m.a32 << ' ' << m.a33 << ']' // row3
    << ']';
    return o;
  }


  //------------------------------------------------------------------------------

  template<typename T>
  template<typename S, typename R>
  inline Vec4<R>&
  Mat4<T>::transform(const Vec4<S>& in,
                     Vec4<R>&       out) const
  {
    if (std::is_same<S, T>::value) {
      out.x = static_cast<R>(a00 * in.x + a01 * in.y + a02 * in.z + a03 * in.w);
      out.y = static_cast<R>(a10 * in.x + a11 * in.y + a12 * in.z + a13 * in.w);
      out.z = static_cast<R>(a20 * in.x + a21 * in.y + a22 * in.z + a23 * in.w);
      out.w = static_cast<R>(a30 * in.x + a31 * in.y + a32 * in.z + a33 * in.w);
    }
    else {
      const T x = static_cast<T>(in.x);
      const T y = static_cast<T>(in.y);
      const T z = static_cast<T>(in.z);
      const T w = static_cast<T>(in.w);
      out.x = static_cast<R>(a00 * x + a01 * y + a02 * z + a03 * w);
      out.y = static_cast<R>(a10 * x + a11 * y + a12 * z + a13 * w);
      out.z = static_cast<R>(a20 * x + a21 * y + a22 * z + a23 * w);
      out.w = static_cast<R>(a30 * x + a31 * y + a32 * z + a33 * w);
    }

    return out;
  }

  //------------------------------------------------------------------------------

  //
  // These are here so we don't have to include Mat4 from the Vec headers.
  // Box2, Box3 implementations are down at end of file so we don't include
  // their headers too soon.
  //
  template<typename T>
  template<typename S>
  inline Vec2<T>
  Vec2<T>::operator *  (const Mat4<S>& m) const
  {
    return m.transform(Vec2<T>{*this});
  }

  template<typename T>
  template<typename S>
  inline Vec2<T>&
  Vec2<T>::operator *= (const Mat4<S>& m)
  {
    *this = m.transform(Vec2<T>{*this});
    return *this;
  }

  template<typename T>
  template<typename S>
  inline Vec3<T>
  Vec3<T>::operator *  (const Mat4<S>& m) const
  {
    Vec3<T> o;
    return m.transform(*this, o);
  }

  template<typename T>
  template<typename S>
  inline Vec3<T>&
  Vec3<T>::operator *= (const Mat4<S>& m)
  {
    Vec3<T> o = *this;
    return m.transform(o, *this);
  }

  template<typename T>
  template<typename S>
  inline Vec4<T>
  Vec4<T>::operator *  (const Mat4<S>& m) const
  {
    Vec4<T> o;
    return m.transform(*this, o);
  }

  template<typename T>
  template<typename S>
  inline Vec4<T>&
  Vec4<T>::operator *= (const Mat4<S>& m)
  {
    Vec4<T> o = *this;
    return m.transform(o, *this);
  }

  //------------------------------------------------------------------------------

  template<typename T>
  template<typename S>
  inline Vec2<S>
  Mat4<T>::vecTransform(const Vec2<S>& v) const
  {
    if (std::is_same<S,T>::value) {
      return Vec2<S>(a00 * v.x + a01 * v.y, a10 * v.x + a11 * v.y);
    }
    else {
      const T x = static_cast<T>(v.x);
      const T y = static_cast<T>(v.y);
      return Vec2<S>(a00 * x + a01 * y, a10 * x + a11 * y);
    }
  }

  template<typename T>
  template<typename S>
  inline Vec3<S>
  Mat4<T>::vecTransform(const Vec3<S>& v) const
  {
    if (std::is_same<S, T>::value) {
      return Vec3<S>(a00 * v.x + a01 * v.y + a02 * v.z,
                     a10 * v.x + a11 * v.y + a12 * v.z,
                     a20 * v.x + a21 * v.y + a22 * v.z);
    }
    else {
      const T x = static_cast<T>(v.x);
      const T y = static_cast<T>(v.y);
      const T z = static_cast<T>(v.z);

      return Vec3<S>(a00 * x + a01 * y + a02 * z,
                     a10 * x + a11 * y + a12 * z,
                     a20 * x + a21 * y + a22 * z);
    }
  }

  template<typename T>
  template<typename S>
  inline void
  Mat4<T>::vecTransform(Vec3<S>*       dstVecs,
                        const Vec3<S>* srcVecs,
                        size_t         nVecs) const
  {
    if (isIdentity()) {
      std::copy(srcVecs, srcVecs + nVecs, dstVecs);
    }
    else {
      for (size_t i = 0; i < nVecs; ++i) {
        *dstVecs++ = vecTransform(*srcVecs++);
      }
    }
  }

  //------------------------------------------------------------------------------

  template<typename T>
  template<typename S>
  inline Vec3<S>
  Mat4<T>::normalTransform(const Vec3<S>& n) const
  {
    if (std::is_same<S, T>::value) {
      return Vec3<S>(static_cast<S>(a00 * n.x + a10 * n.y + a20 * n.z),
                     static_cast<S>(a01 * n.x + a11 * n.y + a21 * n.z),
                     static_cast<S>(a02 * n.x + a12 * n.y + a22 * n.z));
    }
    else
    {
      const T x = static_cast<T>(n.x);
      const T y = static_cast<T>(n.y);
      const T z = static_cast<T>(n.z);

      return Vec3<S>(static_cast<S>(a00 * x + a10 * y + a20 * z),
                     static_cast<S>(a01 * x + a11 * y + a21 * z),
                     static_cast<S>(a02 * x + a12 * y + a22 * z));
    }
  }

  template<typename T>
  template<typename S>
  inline void
  Mat4<T>::normalTransform(Vec3<S>*       dstNormals,
                           const Vec3<S>* srcNormals,
                           size_t         nNormals) const
  {
    if (isIdentity()) {
      std::copy(srcNormals, srcNormals + nNormals, dstNormals);
    }
    else
    {
      for (size_t i = 0; i < nNormals; ++i) {
        *dstNormals++ = normalTransform(*srcNormals++);
      }
    }
  }

  //------------------------------------------------------------------------------

  template<typename T>
  inline T
  Mat4<T>::getDeterminant() const
  {
    return a01 * a23 * a32 * a10 - a01 * a22 * a33 * a10 - a23 * a31 * a02 * a10 + a22 * a31 * a03 * a10 -
           a00 * a23 * a32 * a11 + a00 * a22 * a33 * a11 + a23 * a30 * a02 * a11 - a22 * a30 * a03 * a11 -
           a01 * a23 * a30 * a12 + a00 * a23 * a31 * a12 + a01 * a22 * a30 * a13 - a00 * a22 * a31 * a13 -
           a33 * a02 * a11 * a20 + a32 * a03 * a11 * a20 + a01 * a33 * a12 * a20 - a31 * a03 * a12 * a20 -
           a01 * a32 * a13 * a20 + a31 * a02 * a13 * a20 + a33 * a02 * a10 * a21 - a32 * a03 * a10 * a21 -
           a00 * a33 * a12 * a21 + a30 * a03 * a12 * a21 + a00 * a32 * a13 * a21 - a30 * a02 * a13 * a21;
  }

  template<typename T>
  inline void
  Mat4<T>::invert(Mat4<T>& out,
                  T        determinant) const
  {
    const T idet = (static_cast<T>(1) / determinant);
    out.a00 = (-a23 * a32 * a11 + a22 * a33 * a11 + a23 * a31 * a12 - a22 * a31 * a13 - a33 * a12 * a21 + a32 * a13 * a21) * idet;
    out.a01 = ( a01 * a23 * a32 - a01 * a22 * a33 - a23 * a31 * a02 + a22 * a31 * a03 + a33 * a02 * a21 - a32 * a03 * a21) * idet;
    out.a02 = (-a33 * a02 * a11 + a32 * a03 * a11 + a01 * a33 * a12 - a31 * a03 * a12 - a01 * a32 * a13 + a31 * a02 * a13) * idet;
    out.a03 = ( a23 * a02 * a11 - a22 * a03 * a11 - a01 * a23 * a12 + a01 * a22 * a13 + a03 * a12 * a21 - a02 * a13 * a21) * idet;

    out.a10 = ( a23 * a32 * a10 - a22 * a33 * a10 - a23 * a30 * a12 + a22 * a30 * a13 + a33 * a12 * a20 - a32 * a13 * a20) * idet;
    out.a11 = (-a00 * a23 * a32 + a00 * a22 * a33 + a23 * a30 * a02 - a22 * a30 * a03 - a33 * a02 * a20 + a32 * a03 * a20) * idet;
    out.a12 = ( a33 * a02 * a10 - a32 * a03 * a10 - a00 * a33 * a12 + a30 * a03 * a12 + a00 * a32 * a13 - a30 * a02 * a13) * idet;
    out.a13 = (-a23 * a02 * a10 + a22 * a03 * a10 + a00 * a23 * a12 - a00 * a22 * a13 - a03 * a12 * a20 + a02 * a13 * a20) * idet;

    out.a20 = (-a23 * a31 * a10 + a23 * a30 * a11 - a33 * a11 * a20 + a31 * a13 * a20 + a33 * a10 * a21 - a30 * a13 * a21) * idet;
    out.a21 = (-a01 * a23 * a30 + a00 * a23 * a31 + a01 * a33 * a20 - a31 * a03 * a20 - a00 * a33 * a21 + a30 * a03 * a21) * idet;
    out.a22 = (-a01 * a33 * a10 + a31 * a03 * a10 + a00 * a33 * a11 - a30 * a03 * a11 + a01 * a30 * a13 - a00 * a31 * a13) * idet;
    out.a23 = ( a01 * a23 * a10 - a00 * a23 * a11 + a03 * a11 * a20 - a01 * a13 * a20 - a03 * a10 * a21 + a00 * a13 * a21) * idet;

    out.a30 = ( a22 * a31 * a10 - a22 * a30 * a11 + a32 * a11 * a20 - a31 * a12 * a20 - a32 * a10 * a21 + a30 * a12 * a21) * idet;
    out.a31 = ( a01 * a22 * a30 - a00 * a22 * a31 - a01 * a32 * a20 + a31 * a02 * a20 + a00 * a32 * a21 - a30 * a02 * a21) * idet;
    out.a32 = ( a01 * a32 * a10 - a31 * a02 * a10 - a00 * a32 * a11 + a30 * a02 * a11 - a01 * a30 * a12 + a00 * a31 * a12) * idet;
    out.a33 = (-a01 * a22 * a10 + a00 * a22 * a11 - a02 * a11 * a20 + a01 * a12 * a20 + a02 * a10 * a21 - a00 * a12 * a21) * idet;
  }

  template<typename T>
  inline T
  Mat4<T>::invert(Mat4<T>& out) const
  {
    const T determinant = getDeterminant();
    if (std::abs(determinant) > static_cast<T>(0)) {
      invert(out, determinant);
    }

    return determinant;
  }

  template<typename T>
  inline Mat4<T>&
  Mat4<T>::invert()
  {
    Mat4<T> t;
    invert(t);
    return *this = t;
  }

  template<typename T>
  inline Mat4<T>
  Mat4<T>::inverse() const
  {
    Mat4<T> t;
    invert(t);
    return t;
  }

  template<typename T>
  inline Mat4<T>
  Mat4<T>::inverse(T determinant) const
  {
    Mat4<T> t;
    invert(t, determinant);
    return t;
  }

  //------------------------------------------------------------------------------

  template<typename T>
  template<typename S>
  inline void
  Mat4<T>::setTo(S a, S b, S c, S d,
                 S e, S f, S g, S h,
                 S i, S j, S k, S l,
                 S m, S n, S o, S p, bool transpose)
  {
    if(transpose)
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
    else
    {
      a00 = a;
      a10 = b;
      a20 = c;
      a30 = d;

      a01 = e;
      a11 = f;
      a21 = g;
      a31 = h;

      a02 = i;
      a12 = j;
      a22 = k;
      a32 = l;

      a03 = m;
      a13 = n;
      a23 = o;
      a33 = p;
    }
  }

  template<typename T>
  template<typename S>
  inline Mat4<T>&
  Mat4<T>::setToRotation(T              radian_angle,
                         const Vec3<S>& v)
  {
    const T L = static_cast<T>(1) / v.length();
    const T x = v.x * L;
    const T y = v.y * L;
    const T z = v.z * L;
    const T s = std::sin(radian_angle);
    const T c = std::cos(radian_angle);
    const T c1 = static_cast<T>(1) - c;

    a00 = x * x * c1 + c;
    a01 = y * x * c1 - z * s;
    a02 = z * x * c1 + y * s;
    a03 = static_cast<T>(0);

    a10 = x * y * c1 + z * s;
    a11 = y * y * c1 + c;
    a12 = z * y * c1 - x * s;
    a13 = static_cast<T>(0);

    a20 = x * z * c1 - y * s;
    a21 = y * z * c1 + x * s;
    a22 = z * z * c1 + c;
    a23 = static_cast<T>(0);

    a30 = static_cast<T>(0);
    a31 = static_cast<T>(0);
    a32 = static_cast<T>(0);
    a33 = static_cast<T>(1);

    return *this;
  }

  template<typename T>
  template<typename S>
  inline void
  Mat4<T>::rotateAboutVector(T              radian_angle,
                             const Vec3<S>& v)
  {
    if (std::abs(radian_angle) > 0.0) {
      Mat4<T> m;
      m.setToRotation(radian_angle, v);
      *this *= m;
    }
  }

  template<typename T>
  inline void
  Mat4<T>::skew(T d)
  {
    if (d != static_cast<T>(0)) {
      // duplicate multiplying by identity with a01=a:
      a01 += d * a00;
      a11 += d * a10;
      a21 += d * a20;
      a31 += d * a30;
    }
  }

  template<typename T>
  inline void
  Mat4<T>::skewXY(T x, T y)
  {
    if (x != static_cast<T>(0)) {
      // duplicate multiplying by identity with a01=x:
      a01 += x * a00;
      a11 += x * a10;
      a21 += x * a20;
      a31 += x * a30;
    }

    if (y != static_cast<T>(0)) {
      // duplicate multiplying by identity with a01=y:
      a00 += y * a01;
      a10 += y * a11;
      a20 += y * a21;
      a30 += y * a31;
    }
  }

  template<typename T>
  inline void
  Mat4<T>::skewYX(T x, T y)
  {
    if (y != static_cast<T>(0)) {
      // duplicate multiplying by identity with a01=x:
      a00 += y * a01;
      a10 += y * a11;
      a20 += y * a21;
      a30 += y * a31;
    }

    if (x != static_cast<T>(0)) {
      // duplicate multiplying by identity with a01=y:
      a01 += x * a00;
      a11 += x * a10;
      a21 += x * a20;
      a31 += x * a30;
    }
  }

  template<typename T>
  template<typename S>
  inline void
  Mat4<T>::skew(const Vec3<S>& v)
  {
    if (v.x != static_cast<S>(0) || v.y != static_cast<S>(0) || v.z != static_cast<S>(0)) {
      Mat4<T> mSkew;
      mSkew.setToIdentity();

      T sx = std::fmod(T(v.x), static_cast<T>(180));
      if (sx == static_cast<T>(90)) {
        sx = static_cast<T>(89.9999999);
      }

      mSkew.a01 = static_cast<T>(1) / ::tan(radians(static_cast<T>(90) - sx));

      T sy = std::fmod(T(v.y), static_cast<T>(180));
      if (sy == static_cast<T>(90)) {
        sy = static_cast<T>(89.9999999);
      }

      mSkew.a10 = static_cast<T>(1) / ::tan(radians(static_cast<T>(90) - sy));

      T sz = std::fmod(T(v.z), static_cast<T>(180));
      if (sz == static_cast<T>(90)) {
        sz = static_cast<T>(89.9999999);
      }

      mSkew.a12 = static_cast<T>(1) / ::tan(radians(static_cast<T>(90) - sz));

      *this *= mSkew;
    }
  }

  template<typename T>
  template<typename S, typename R>
  /*static*/ inline bool
  Mat4<T>::vectorToRotations(const Vec3<S>&     dir_vec,
                             fdk::AxisDirection align_axis,
                             bool               do_rx,
                             bool               do_ry,
                             bool               do_rz,
                             Vec3<R>&           rotations,
                             fdk::RotateOrder&  rotation_order)
  {
    rotations.set(static_cast<R>(0));
    S d;
    // Calculate the primary rotation first then the second, and which rotation axis we
    // change is determined by the align axis:
    switch (align_axis) {
      case AxisDirection::X_MINUS:
        if (do_ry) {
          rotations.y = static_cast<R>(::atan2(dir_vec.z,-dir_vec.x));
          d = std::sqrt(dir_vec.z * dir_vec.z + dir_vec.x * dir_vec.x);
        }
        else {
          d = -dir_vec.x;
        }

        if (do_rz) { rotations.z = static_cast<R>(-::atan2(dir_vec.y, d)); }
        rotation_order = RotateOrder::XZY;
        break;
      case AxisDirection::X_PLUS:
        if (do_ry) {
          rotations.y = static_cast<R>(::atan2(-dir_vec.z, dir_vec.x));
          d = std::sqrt(dir_vec.z * dir_vec.z + dir_vec.x * dir_vec.x);
        }
        else {
          d =  dir_vec.x;
        }

        if (do_rz) { rotations.z = static_cast<R>(::atan2( dir_vec.y, d)); }
        rotation_order = RotateOrder::XZY;
        break;
      case AxisDirection::Y_MINUS:
        if (do_rx) {
          rotations.x = static_cast<R>(::atan2(-dir_vec.z,-dir_vec.y));
          d = std::sqrt(dir_vec.z * dir_vec.z + dir_vec.y * dir_vec.y);
        }
        else {
          d = -dir_vec.y;
        }

        if (do_rz) { rotations.z = static_cast<R>(::atan2(dir_vec.x, d)); }
        rotation_order = RotateOrder::YZX;
        break;
      case AxisDirection::Y_PLUS:
        if (do_rx) {
          rotations.x = static_cast<R>(::atan2(dir_vec.z, dir_vec.y));
          d = std::sqrt(dir_vec.z * dir_vec.z + dir_vec.y * dir_vec.y);
        }
        else {
          d =  dir_vec.y;
        }

        if (do_rz) { rotations.z = static_cast<R>(-::atan2(dir_vec.x, d)); }
        rotation_order = RotateOrder::YZX;
        break;
      case AxisDirection::Z_MINUS:
        if (do_ry) {
          rotations.y = static_cast<R>(::atan2(-dir_vec.x,-dir_vec.z));
          d = std::sqrt(dir_vec.x * dir_vec.x + dir_vec.z * dir_vec.z);
        }
        else {
          d = -dir_vec.z;
        }

        if (do_rx) { rotations.x = static_cast<R>(::atan2(dir_vec.y, d)); }
        rotation_order = RotateOrder::ZXY;
        break;
      case AxisDirection::Z_PLUS:
        if (do_ry) {
          rotations.y = static_cast<R>(::atan2(dir_vec.x, dir_vec.z));
          d = std::sqrt(dir_vec.x * dir_vec.x + dir_vec.z * dir_vec.z);
        }
        else {
          d =  dir_vec.z;
        }

        if (do_rx) { rotations.x = static_cast<R>(-::atan2(dir_vec.y, d)); }
        rotation_order = RotateOrder::ZXY;
        break;
    }

    return true;
  }

  template<typename T>
  template<typename S>
  inline Mat4<T>
  Mat4<T>::lookAt(fdk::AxisDirection axis,
                  const fdk::Vec3<S>& lookAtPos,
                  T strength,
                  bool rotateX,
                  bool rotateY,
                  bool rotateZ)
  {
    const fdk::Vec3d dirVector = lookAtPos - getTranslation();

    fdk::Vec3d rotations;
    fdk::RotateOrder rotationOrder;
    Mat4d::vectorToRotations(dirVector, axis, rotateX, rotateY, rotateZ, rotations, rotationOrder);
    
    const auto originalRotations = getRotations(rotationOrder);

    if (!rotateX) {
      rotations.x = originalRotations.x;
    }
    if (!rotateY) {
      rotations.y = originalRotations.y;
    }
    if (!rotateZ) {
      rotations.z = originalRotations.z;
    }
    
    rotations = originalRotations.lerpClampedTo(rotations, strength);

    fdk::Mat4<T> result;
    result.setToTranslation(getTranslation());
    result.rotateInOrder(rotationOrder, rotations);
    result.scale(getScale());

    return result;
  }

  //------------------------------------------------------------------------------

  /* Interpolate two matrices at 't' which is between 0.0 and 1.0.

      TODO: this should use quaternions instead...or decompose the
      matrices into trans/rot/scale and interpolate those...?
  */
  template<typename T, typename S>
  inline Mat4<T>
  lerpClamped(const Mat4<T>& m0,
       const Mat4<T>& m1,
       S              t)
  {
    if (t <= static_cast<S>(0)) {
      return m0;
    }
    else if (t >= static_cast<S>(1)) {
      return m1;
    }
    else
    {
      const T Tt = static_cast<T>(t);
      const T inv_Tt = (static_cast<T>(1) - Tt);

      Vec3<T> axisX(m0.xAxis() * inv_Tt + m1.xAxis() * Tt);
      Vec3<T> axisY(m0.yAxis() * inv_Tt + m1.yAxis() * Tt);
      Vec3<T> axisZ(m0.zAxis() * inv_Tt + m1.zAxis() * Tt);

      // Normalizing the axes returns their lengths to use as scales:
      const Vec3<T> axes_scale(axisX.normalize(),
                               axisY.normalize(),
                               axisZ.normalize());

      Mat4<T> mlerp;
      mlerp.setToTranslation(m0.getTranslation() * inv_Tt + m1.getTranslation() * Tt);
      mlerp.setXYZAxis(axisX, axisY, axisZ);
      mlerp.scale(axes_scale);
      return mlerp;
    }
  }

  template<typename T>
  template<typename S>
  inline void
  Mat4<T>::interpolate(const Mat4<T>& m0,
                       const Mat4<T>& m1,
                       S              t) { *this = fdk::lerpClamped(m0, m1, t); }

}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// Box2/3 transform implementations
//
// Keep the Box includes at end of Mat4.h so it doesn't conflict with
// Vec2/3/4 method implementations up above
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#include "Box3.h"

namespace fdk {

  //! Faster implementation from Graphics Gems I, page 785 -
  //! "Transforming Axis-Aligned Bounding Boxes"
  template<typename T>
  template<typename S, typename R>
  inline Box3<R>&
  Mat4<T>::transform(const Box3<S>& in,
                           Box3<R>& out) const
  {
    // Copy translations out of matrix:
    const Vec3<T> tr = getTranslation();
    out.set(tr, tr);
    for (int i = 0; i < 3; ++i) {
      for (int j = 0; j < 3; ++j) {
        const T t = (*this)[j][i];
        const T a = t * static_cast<T>(in.min[j]);
        const T b = t * static_cast<T>(in.max[j]);
        if (a < b) {
          out.min[i] += static_cast<S>(a);
          out.max[i] += static_cast<S>(b);
        }
        else {
          out.min[i] += static_cast<S>(b);
          out.max[i] += static_cast<S>(a);
        }
      }
    }

    return out;
  }

  template<typename T>
  template<typename S>
  inline Box3<S>
  Mat4<T>::operator * (const Box3<S>& bbox) const
  {
    Box3<S> o;
    return transform(bbox, o);
  }

  template<typename T>
  template<typename S>
  inline Box3<S>
  Mat4<T>::transform(const Box3<S>& bbox) const
  {
    return *this * bbox;
  }

}

#endif
