// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FDK_MATH_VEC3_H
#define FDK_MATH_VEC3_H

#include "fdk/math/Math.h"
#include "fdk/math/Vec2.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <type_traits>
#include <vector>

namespace fdk {

  template <typename T> class Mat4;


  //------------------------------------------------------------------------------


  // These match the rotation order enums in DD::Image::Axis_KnobI:
  enum class RotateOrder : int32_t {
    XYZ,
    XZY,
    YXZ,
    YZX,
    ZXY,
    ZYX
  };

  // These match the enum order in DD::Image::LookAt:
  enum class AxisDirection : int32_t {
    X_MINUS,
    X_PLUS,
    Y_MINUS,
    Y_PLUS,
    Z_MINUS,
    Z_PLUS
  };


  //------------------------------------------------------------------------------


  //! \class fdk::Vec3
  //! 3-component vector
  template <typename T>
  class Vec3
  {
  public:
    using value_type = T;
    using reference = T&;
    using const_reference = const T&;

    static constexpr uint8_t kNumElements = 3; //< The number of elements in the vector

    static constexpr uint8_t X = 0; //< Constant index for the X axis
    static constexpr uint8_t Y = 1; //< Constant index for the Y axis
    static constexpr uint8_t Z = 2; //< Constant index for the Z axis

    // These are public for ease of access.
    T   x, y, z;    //!< the data


    /*---------------------------------------------------------*/
    /*                      Constructors                       */
    /*---------------------------------------------------------*/
    //! Default constructor leaves garbage in contents.
    Vec3() = default;

    //! Copy constructor supports type conversion.
    template<typename S>
    explicit Vec3(const Vec3<S>& b);

    //! Constructor that normalizes vector after copy if 'n' is > 0.
    template<typename S>
    explicit Vec3(const Vec3<S>& v,
                  float          n);

    //! Constructor that sets all components to a single value.
    explicit Vec3(T a) : x(a), y(a), z(a) {}

    //! Constructors that set separate components, with default z=0
    Vec3(T a, T b)      : x(a), y(b), z(static_cast<T>(0)) {}
    Vec3(T a, T b, T c) : x(a), y(b), z(c) {}

    //! Constructor from an array of 3 numbers.
    template<typename S>
    Vec3(      S* v) : x(v[0]), y(v[1]), z(v[2]) {}

    template<typename S>
    Vec3(const S* v) : x(v[0]), y(v[1]), z(v[2]) {}

    // Compatibility with other vector sizes.
    Vec3(const Vec2<T>& v, T vz=(T)0) : x(v.x), y(v.y), z(vz) {}


    //! Returns value as a specific type:
    operator Vec3<fdk::half>() const;
    operator Vec3<float>()     const;
    operator Vec3<double>()    const;
    operator Vec3<int>()       const;

    Vec3<fdk::half> asVec3h() const;
    Vec3<float>     asVec3f() const;
    Vec3<double>    asVec3d() const;
    Vec3<int>       asVec3i() const;

    const static Vec3 XAxis; //< Constant for the X axis vector
    const static Vec3 YAxis; //< Constant for the Y axis vector
    const static Vec3 ZAxis; //< Constant for the Z axis vector

    /*---------------------------------------------------------*/
    /*                     Component Access                    */
    /*---------------------------------------------------------*/
    T&       operator [] (int n)       { return *(&x + n); }
    const T& operator [] (int n) const { return *(&x + n); }

    //! Returns a pointer to the first element.
    T*        array()       { return &x; }
    const T*  array() const { return &x; }

    Vec2<T>   xy()  const { return Vec2<T>(x, y); }


    /*---------------------------------------------------------*/
    /*                        Assignment                       */
    /*---------------------------------------------------------*/
    //! Set all components to a single value.
    void      set(T d) { x = y = z = d; }

    //! Set all components.
    void      set(T a, T b, T c);

    //! Set components to 0.
    void      setToZero() { x = y = z = static_cast<T>(0); }

    //! Set components to 1.
    void      setToOne()  { x = y = z = static_cast<T>(1); }

    //! Type-specific clear. Set all components to 0.
    void      clear() { setToZero(); }


    //! Add this to a fdk::Hash object.
    void      append(fdk::Hash& hash) const;


    /*---------------------------------------------------------*/
    /*                   Matrix Multiplication                 */
    /*---------------------------------------------------------*/
    template<typename S>
    Vec3  operator *  (const Mat4<S>& m) const;
    template<typename S>
    Vec3& operator *= (const Mat4<S>& m);

    /*---------------------------------------------------------*/
    /*                       Multiplication                    */
    /*---------------------------------------------------------*/
    template<typename S, std::enable_if_t<fdk::is_arithmetic<S>, bool> = true>
    Vec3  operator *  (S d)           const { return Vec3(x * d, y * d, z * d); }
    Vec3  operator *  (const Vec3& v) const { return Vec3(x * v.x, y * v.y, z * v.z); }

    template<typename S, std::enable_if_t<fdk::is_arithmetic<S>, bool> = true>
    Vec3& operator *= (S d);
    Vec3& operator *= (const Vec3& v);

    /*---------------------------------------------------------*/
    /*                        Division                         */
    /*---------------------------------------------------------*/
    template<typename S, std::enable_if_t<fdk::is_arithmetic<S>, bool> = true>
    Vec3  operator /  (S d)           const { return Vec3(x / d, y / d, z / d); }
    Vec3  operator /  (const Vec3& v) const { return Vec3(x / v.x, y / v.y, z / v.z); }

    template<typename S, std::enable_if_t<fdk::is_arithmetic<S>, bool> = true>
    Vec3& operator /= (S d);
    Vec3& operator /= (const Vec3& v);

    /*---------------------------------------------------------*/
    /*                        Addition                         */
    /*---------------------------------------------------------*/
    template<typename S, std::enable_if_t<fdk::is_arithmetic<S>, bool> = true>
    Vec3  operator +  (S d)           const { return Vec3(x + d, y + d, z + d); }
    Vec3  operator +  (const Vec3& v) const { return Vec3(x + v.x, y + v.y, z + v.z); }

    template<typename S, std::enable_if_t<fdk::is_arithmetic<S>, bool> = true>
    Vec3& operator += (S d);
    Vec3& operator += (const Vec3& v);

    /*---------------------------------------------------------*/
    /*                       Subtraction                       */
    /*---------------------------------------------------------*/
    template<typename S, std::enable_if_t<fdk::is_arithmetic<S>, bool> = true>
    Vec3  operator -  (S d)           const { return Vec3(x - d, y - d, z - d); }
    Vec3  operator -  (const Vec3& v) const { return Vec3(x - v.x, y - v.y, z - v.z); }

    template<typename S, std::enable_if_t<fdk::is_arithmetic<S>, bool> = true>
    Vec3& operator -= (S d);
    Vec3& operator -= (const Vec3& v);

    /*---------------------------------------------------------*/
    /*                        Negation                         */
    /*---------------------------------------------------------*/
    Vec3  operator -  () const { return Vec3(-x, -y, -z); }
    void negate();

    /*---------------------------------------------------------*/
    /*                        Equality                         */
    /*---------------------------------------------------------*/
    bool operator == (const Vec3& v) const { return (x == v.x && y == v.y && z == v.z); }
    bool operator != (const Vec3& v) const { return (x != v.x || y != v.y || z != v.z); }
    template<typename S>
    bool operator == (S d)           const { return (x == d && y == d && z == d); }
    template<typename S>
    bool operator != (S d)           const { return (x != d || y != d || z != d); }

    /*---------------------------------------------------------*/
    /*                         Sorting                         */
    /*---------------------------------------------------------*/
    //! Fairly arbitrary operator so you can store these in ordered arrays
    bool operator <  (const Vec3& v) const;


    /*---------------------------------------------------------*/
    /*                     Vector Functions                    */
    /*---------------------------------------------------------*/

    //! Also known as the absolute value or magnitude of the vector.
    T         length() const { return std::sqrt(x * x + y * y + z * z); }

    //! Same as this dot this, length() squared.
    T         lengthSquared() const { return (x * x + y * y + z * z); }

    //! Same as (this-v).length()
    T         distanceBetween(const Vec3& v) const { return std::sqrt((x - v.x) * (x - v.x) + (y - v.y) * (y - v.y) + (z - v.z) * (z - v.z)); }

    //! Same as (this-v).lengthSquared()
    T         distanceSquared(const Vec3& v) const { return (x - v.x) * (x - v.x) + (y - v.y) * (y - v.y) + (z - v.z) * (z - v.z); }

    //! Return the scalar distance to the plane defined by ABCD.
    T         distanceFromPlane(T A, T B, T C, T D) const { return (A * x + B * y + C * z + D); }

    //! Dot product. Twice the area of the triangle between the vectors.
    T         dot(const Vec3& v) const { return (x * v.x + y * v.y + z * v.z); }

    //! Cross product. Returns a vector at right angles to the vectors.
    Vec3      cross(const Vec3& v) const { return Vec3(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x); }

    //! Change the vector to be unit length. Returns the original length.
    T         normalize();

    //! Returns the normalized unit vector form of this vector.
    //! If this vector is null, then a null vector is returned.
    Vec3      normalized() const;

    //! Return a vector of this one reflected around a normal vector.
    Vec3      reflect(const Vec3& N) const { return (N * (dot(N) * static_cast<T>(2)) - *this); }

    //! Negate (flip) vector if it points in the opposite direction of N.
    void      faceForward(const Vec3& N);

    //! Returns the minimum element.
    T         minimumComponent()     const { return std::min(x, std::min(y, z)); }
    Vec3      minimum(const Vec3& v) const { return Vec3(std::min(v.x, x), std::min(v.y, y), std::min(v.z, z)); }

    //! Returns the maximum element.
    T         maximumComponent()     const { return std::max(x, std::max(y, z)); }
    Vec3      maximum(const Vec3& v) const { return Vec3(std::max(v.x, x), std::max(v.y, y), std::max(v.z, z)); }

    //! Returns the absolute value of the largest element.
    T         largestAxis() const { return std::max(std::abs(x), std::max(std::abs(y), std::abs(z))); }

    //! Linear-interpolate between this Vec3 and another at t, where t=0..1.
    template<typename S>
    Vec3<T>   interpolateTo(const Vec3<T>& b,
                            S              t) const;

    template<typename S>
    Vec3<T>   lerpClampedTo(const Vec3<T>& b,
                            S              t) const;

    template<typename S>
    Vec3<T>   lerpUnclapmedTo(const Vec3<T>& b,
                              S              t) const;

    Vec3      inverted() const { return Vec3(invert(x), invert(y), invert(z)); }

    template<typename S>
    Vec3      clamped(S min, S max) const { return Vec3(std::clamp<T>(x, min, max), std::clamp<T>(y, min, max), std::clamp<T>(z, min, max)); }

    Vec3      clamped() const { return clamped(static_cast<T>(0), static_cast<T>(1)); }

    //! Convert to/from radians/degrees.
    void      toRadians();

    Vec3      asRadians() const { return Vec3(radians(x), radians(y), radians(z)); }
    void      toDegrees();

    Vec3      asDegrees() const { return Vec3(degrees(x), degrees(y), degrees(z)); }

    bool      isZero()          const { return !notZero(); }
    bool      notZero()         const { return  (x != static_cast<T>(0) || y != static_cast<T>(0) || z != static_cast<T>(0)); }
    bool      greaterThanZero() const { return (x > static_cast<T>(0) || y > static_cast<T>(0) || z > static_cast<T>(0)); }

    //! Round off each element if nearly one or zero within the type's epsilon default or user-provided threshold.
    void      roundIfNearlyZero(T threshold = std::numeric_limits<T>::epsilon());
    void      roundIfNearlyOne(T threshold = std::numeric_limits<T>::epsilon());

    /*! Orient vector relative to a normal's frame.
        The +Z axis of this vector is rotated to line up with the normal.
        If N.z is negative then the up orientation of the resulting vector is
        flipped to avoid the degenerate case where N.z gets near -1.0 and there's
        no rotation solution.
    */
    void      orientAroundNormal(Vec3 N,
                                 bool auto_flip = true);

    //! Rotate a vector by an angle around a center axis vector.
    void      rotateAroundAxis(T           angle,
                               const Vec3& axis);


  private:
    // This must match for Vec/Mat/Box classes:
    static_assert(fdk::is_arithmetic<T>, "Vec3 can only be instantiated with an arithmetic type.");

  };


  /*--------------------------------------*/
  /*    Aliases for supported data types  */
  /*--------------------------------------*/
  using Vec3f = Vec3<float>;
  using Vec3d = Vec3<double>;
  using Vec3i = Vec3<int32_t>;
  using Vec3h = Vec3<fdk::half>;



  /*----------------------------------*/
  /*        Static operations         */
  /*----------------------------------*/

  template <typename T> const Vec3<T> Vec3<T>::XAxis(static_cast<T>(1), static_cast<T>(0), static_cast<T>(0));
  template <typename T> const Vec3<T> Vec3<T>::YAxis(static_cast<T>(0), static_cast<T>(1), static_cast<T>(0));
  template <typename T> const Vec3<T> Vec3<T>::ZAxis(static_cast<T>(0), static_cast<T>(0), static_cast<T>(1));

  //! Print out Vec3 components to a stream.
  template<typename T>
  std::ostream& operator << (std::ostream& o, const Vec3<T>& v);


  //!
  template<typename T>
  T           dot(const Vec3<T>& a,
                  const Vec3<T>& b) { return a.dot(b); }

  template<typename T>
  Vec3<T>     cross(const Vec3<T>& a,
                    const Vec3<T>& b) { return a.cross(b); }


  //! Multiply a vector by a scalar returning a new vector.
  template<typename T, typename S, std::enable_if_t<fdk::is_arithmetic<S>, bool> = true>
  Vec3<T> operator * (S a, const Vec3<T>& b) { return Vec3<T>(b.x * a, b.y * a, b.z * a); }

  //! Divide a vector by a scalar returning a new vector.
  template<typename T, typename S, std::enable_if_t<fdk::is_arithmetic<S>, bool> = true>
  Vec3<T> operator / (S a, const Vec3<T>& b) { return Vec3<T>(a / b.x, a / b.y, a / b.z); }

  //! Add a vector by a scalar returning a new vector.
  template<typename T, typename S, std::enable_if_t<fdk::is_arithmetic<S>, bool> = true>
  Vec3<T> operator + (S a, const Vec3<T>& b) { return Vec3<T>(a + b.x, a + b.y, a + b.z); }

  //! Subtract a vector by a scalar returning a new vector.
  template<typename T, typename S, std::enable_if_t<fdk::is_arithmetic<S>, bool> = true>
  Vec3<T> operator - (S a, const Vec3<T>& b) { return Vec3<T>(a - b.x, a - b.y, a - b.z); }


  /*! Apply a euler rotation filter to a series of rotation keyframes (in degrees).
      rot_order can be 'XYZ', 'XZY', 'YXZ', 'YZX', 'ZXY', or 'ZYX'.  Default is 'ZXY'.
  */
  template<typename T>
  void        eulerFilterRotations(std::vector<Vec3<T> >& rotations_in_degrees,
                                   RotateOrder            rot_order = RotateOrder::ZXY);


  //! Linear-interpolate between two Vec3s at t, where t=0..1.
  template<typename T, typename S>
  Vec3<T>     lerpClamped(const Vec3<T>& v0,
                          const Vec3<T>& v1,
                          S              t);

  //! Linear-interpolate between two Vec3s at t, where t=0..1, and inv is 1-t.
  template<typename T, typename S>
  Vec3<T>     lerpClamped(const Vec3<T>& v0,
                          const Vec3<T>& v1,
                          S              t,
                          S              invt);

  //! Unclamped inear-interpolate between two Vec3s at t, where t=0..1.
  template<typename T, typename S>
  Vec3<T>     lerpUnclamped(const Vec3<T>& v0,
                   const Vec3<T>& v1,
                   S              t);

  //! Unclamped linear-interpolate between two Vec3s at t, where t=0..1, and inv is 1-t.
  template<typename T, typename S>
  Vec3<T>     lerpUnclamped(const Vec3<T>& v0,
                            const Vec3<T>& v1,
                            S              t,
                            S              invt);

  //! Interpolate between three Vec3s at barycentric coord st.
  template<typename T, typename S>
  Vec3<T>     interpolateAtBaryCoord(const Vec3<T>& v0,
                                     const Vec3<T>& v1,
                                     const Vec3<T>& v2,
                                     const Vec2<S>& st);

  //! Interpolate between three Vec3s at barycentric coord st, with derivatives.
  template<typename T, typename S>
  void        interpolateAtBaryCoord(const Vec3<T>& v0,
                                     const Vec3<T>& v1,
                                     const Vec3<T>& v2,
                                     const Vec2<S>& st,
                                     const Vec2<S>& stdx,
                                     const Vec2<S>& stdy,
                                     Vec3<T>&       out,
                                     Vec3<T>&       duout,
                                     Vec3<T>&       dvout);



  /*---------------------------------------------------------------------*/
  /*---------------------------------------------------------------------*/
  /*                   Inline Function Implementations                   */
  /*---------------------------------------------------------------------*/
  /*---------------------------------------------------------------------*/

  // Note:
  //  Methods with multiple arg type-combos must be implemented inline so the
  //  compiler can resolve the combinations and avoid linking issues.


  template<typename T>
  template<typename S>
  inline
  Vec3<T>::Vec3(const Vec3<S>& b)
  {
    x = static_cast<T>(b.x);
    y = static_cast<T>(b.y);
    z = static_cast<T>(b.z);
  }

  // Copy and normalize in same constructor.
  template<typename T>
  template<typename S>
  inline Vec3<T>::Vec3(const Vec3<S>& v,
                       float          n) :
      x(static_cast<T>(v.x)),
      y(static_cast<T>(v.y)),
      z(static_cast<T>(v.z))
  {
    if (n > 0.0f) {
      T d = length();
      if (d > static_cast<T>(0)) {
        d = static_cast<T>(1) / d;
        x *= d;
        y *= d;
        z *= d;
      }
    }
  }

  template<typename T>
  inline std::ostream& operator << (std::ostream& o, const Vec3<T>& v)
  {
    o << '[' << v.x << ' ' << v.y << ' ' << v.z << ']';
    return o;
  }

  //-----------------------------------------------------------

  template<typename T>
  inline Vec3<fdk::half>
  Vec3<T>::asVec3h() const
  {
    if (std::is_same<fdk::half, T>::value) {
      return *this;
    }
    else {
      return Vec3<fdk::half>(fdk::half(static_cast<float>(x)), fdk::half(static_cast<float>(y)), fdk::half(static_cast<float>(z)));
    }
  }
  template<typename T>
  inline Vec3<T>::operator Vec3<fdk::half>() const { return asVec3h(); }

  template<typename T>
  inline Vec3<float>
  Vec3<T>::asVec3f() const
  {
    if (std::is_same<float, T>::value) {
      return *this;
    }
    else {
      return Vec3<float>(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
    }
  }

  template<typename T>
  inline Vec3<T>::operator Vec3<float>() const { return asVec3f(); }

  template<typename T>
  inline Vec3<double>
  Vec3<T>::asVec3d() const
  {
    if (std::is_same<double, T>::value) {
      return *this;
    }
    else {
      return Vec3<double>(static_cast<double>(x), static_cast<double>(y), static_cast<double>(z));
    }
  }

  template<typename T>
  inline Vec3<T>::operator Vec3<double>() const { return asVec3d(); }

  template<typename T>
  inline Vec3<int>
  Vec3<T>::asVec3i() const
  {
    if (std::is_same<int, T>::value) {
      return *this;
    }
    else {
      return Vec3<int>(static_cast<int>(x), static_cast<int>(y), static_cast<int>(z));
    }
  }

  template<typename T>
  inline Vec3<T>::operator Vec3<int>() const { return asVec3i(); }

  //-----------------------------------------------------------

  template<typename T>
  inline void
  Vec3<T>::set(T a, T b, T c)
  {
    x = a;
    y = b;
    z = c;
  }

  template<typename T>
  template<typename S, std::enable_if_t<fdk::is_arithmetic<S>, bool>>
  inline Vec3<T>&
  Vec3<T>::operator *= (S d)
  {
    x *= d;
    y *= d;
    z *= d;
    return *this;
  }

  template<typename T>
  inline Vec3<T>&
  Vec3<T>::operator *= (const Vec3<T>& v)
  {
    x *= v.x;
    y *= v.y;
    z *= v.z;
    return *this;
  }

  template<typename T>
  template<typename S, std::enable_if_t<fdk::is_arithmetic<S>, bool>>
  inline Vec3<T>&
  Vec3<T>::operator /= (S d)
  {
    x /= d;
    y /= d;
    z /= d;
    return *this;
  }

  template<typename T>
  inline Vec3<T>&
  Vec3<T>::operator /= (const Vec3<T>& v)
  {
    x /= v.x;
    y /= v.y;
    z /= v.z;
    return *this;
  }

  template<typename T>
  template<typename S, std::enable_if_t<fdk::is_arithmetic<S>, bool>>
  inline Vec3<T>&
  Vec3<T>::operator += (S d)
  {
    x += d;
    y += d;
    z += d;
    return *this;
  }

  template<typename T>
  inline Vec3<T>&
  Vec3<T>::operator += (const Vec3<T>& v)
  {
    x += v.x;
    y += v.y;
    z += v.z;
    return *this;
  }

  template<typename T>
  template<typename S, std::enable_if_t<fdk::is_arithmetic<S>, bool>>
  inline Vec3<T>&
  Vec3<T>::operator -= (S d)
  {
    x -= d;
    y -= d;
    z -= d;
    return *this;
  }

  template<typename T>
  inline Vec3<T>&
  Vec3<T>::operator -= (const Vec3<T>& v)
  {
    x -= v.x;
    y -= v.y;
    z -= v.z;
    return *this;
  }

  template<typename T>
  inline void
  Vec3<T>::negate()
  {
    x = -x;
    y = -y;
    z = -z;
  }

  template<typename T>
  inline T
  Vec3<T>::normalize()
  {
    const T d = length();
    if (d > static_cast<T>(0)) {
      *this /= d;
    }

    return d;
  }

  template<typename T>
  inline Vec3<T>
  Vec3<T>::normalized() const
  {
    const T d = length();
    return (d > static_cast<T>(0)) ? Vec3<T>(x / d, y / d, z / d) : *this * d;
  }

  template<typename T>
  inline void
  Vec3<T>::faceForward(const Vec3<T>& N)
  {
    if (dot(N) < (T)0) {
      x = -x;
      y = -y;
      z = -z;
    }
  }

  template<typename T>
  inline void
  Vec3<T>::toRadians()
  {
    x = radians(x);
    y = radians(y);
    z = radians(z);
  }

  template<typename T>
  inline void
  Vec3<T>::toDegrees()
  {
    x = degrees(x);
    y = degrees(y);
    z = degrees(z);
  }

  //-----------------------------------------------------------

  template<typename T>
  inline bool
  Vec3<T>::operator < (const Vec3<T>& v) const
  {
    if (x < v.x) {
      return true;
    }

    if (x > v.x) {
      return false;
    }

    if (y < v.y) {
      return true;
    }

    if (y > v.y) {
      return false;
    }

    return (z < v.z);
  }

  template<typename T>
  template<typename S>
  inline Vec3<T>
  Vec3<T>::interpolateTo(const Vec3<T>& b,
                         S              t) const
  {
    if (t < std::numeric_limits<S>::epsilon()) {
      return *this; // before or at first
    }
    else if (t > (static_cast<S>(1) - std::numeric_limits<S>::epsilon())) {
      return b; // at or after last
    }

    const T tT    = static_cast<T>(t);
    const T invtT = static_cast<T>(1) - tT;
    return Vec3<T>(x * invtT + b.x * tT, y * invtT + b.y * tT, z * invtT + b.z * tT);
  }

  template<typename T>
  template<typename S>
  inline Vec3<T>
  Vec3<T>::lerpClampedTo(const Vec3<T>& b, S t) const
  {
    return interpolateTo(b, t);
  }

  template<typename T>
  template<typename S>
  inline Vec3<T>
  Vec3<T>::lerpUnclapmedTo(const Vec3<T>& b, S t) const
  {
    const T tT    = static_cast<T>(t);
    const T invtT = static_cast<T>(1) - tT;
    return Vec3<T>(x * invtT + b.x * tT, y * invtT + b.y * tT, z * invtT + b.z * tT);
  }

  template<typename T, typename S>
  inline Vec3<T>
  lerpClamped(const Vec3<T>& v0,
              const Vec3<T>& v1,
              S              t)
  {
    if (t < std::numeric_limits<S>::epsilon()) {
      return v0; // before or at first
    }
    else if (t > (static_cast<S>(1) - std::numeric_limits<S>::epsilon())) {
      return v1; // at or after last
    }

    const T tT    = static_cast<T>(t);
    const T invtT = static_cast<T>(1) - tT;
    return Vec3<T>(v0.x * invtT + v1.x * tT, v0.y * invtT + v1.y * tT, v0.z * invtT + v1.z * tT);
  }

  template<typename T, typename S>
  inline Vec3<T>
  lerpClamped(const Vec3<T>& v0,
              const Vec3<T>& v1,
              S              t,
              S              invt)
  {
    if (t < std::numeric_limits<S>::epsilon()) {
      return v0; // before or at first
    }
    else if (t > (static_cast<S>(1) - std::numeric_limits<S>::epsilon())) {
      return v1; // at or after last
    }

    const T tT    = static_cast<T>(t);
    const T invtT = static_cast<T>(invt);
    return Vec3<T>(v0.x * invtT + v1.x * tT, v0.y * invtT + v1.y * tT, v0.z * invtT + v1.z * tT);
  }

  template<typename T, typename S>
  inline Vec3<T>
  lerpUnclamped(const Vec3<T>& v0,
                   const Vec3<T>& v1,
                   S              t)
  {
    const T tT    = static_cast<T>(t);
    const T invtT = static_cast<T>(1) - tT;
    return Vec3<T>(v0.x * invtT + v1.x * tT, v0.y * invtT + v1.y * tT, v0.z * invtT + v1.z * tT);
  }

  template<typename T, typename S>
  inline Vec3<T>
  lerpUnclamped(const Vec3<T>& v0,
                   const Vec3<T>& v1,
                   S              t,
                   S              invt)
  {
    const T tT    = static_cast<T>(t);
    const T invtT = static_cast<T>(invt);
    return Vec3<T>(v0.x * invtT + v1.x * tT, v0.y * invtT + v1.y * tT, v0.z * invtT + v1.z * tT);
  }

  // Interpolate between three Vec3s at barycentric coord st.
  template<typename T, typename S>
  inline Vec3<T>
  interpolateAtBaryCoord(const Vec3<T>& v0,
                         const Vec3<T>& v1,
                         const Vec3<T>& v2,
                         const Vec2<S>& st)
  {
    return Vec3<T>(v0 + ((v1 - v0) * static_cast<T>(st.x)) + ((v2 - v0) * static_cast<T>(st.y)));
  }

  // Interpolate between three Vec3s at barycentric coord st, with derivatives.
  template<typename T, typename S>
  inline void
  interpolateAtBaryCoord(const Vec3<T>& v0,
                         const Vec3<T>& v1,
                         const Vec3<T>& v2,
                         const Vec2<S>& st,
                         const Vec2<S>& stdx,
                         const Vec2<S>& stdy,
                         Vec3<T>&       out,
                         Vec3<T>&       duout,
                         Vec3<T>&       dvout)
  {
    const Vec3<T> e01 = (v1 - v0);
    const Vec3<T> e02 = (v2 - v0);
    const Vec3<T> dt = (e01 * static_cast<T>(st.x)) + (e02 * static_cast<T>(st.y));
    out   = v0 + dt;
    duout = (e01 * static_cast<T>(stdx.x)) + (e02 * static_cast<T>(stdx.y)) - dt;
    dvout = (e01 * static_cast<T>(stdy.x)) + (e02 * static_cast<T>(stdy.y)) - dt;
  }

  //-----------------------------------------------------------

  template<typename T>
  inline void
  Vec3<T>::roundIfNearlyZero(T threshold)
  {
    if (isNearlyZero(x, threshold)) {
      x = static_cast<T>(0);
    }

    if (isNearlyZero(y, threshold)) {
      y = static_cast<T>(0);
    }

    if (isNearlyZero(z, threshold)) {
      z = static_cast<T>(0);
    }
  }

  template<typename T>
  inline void
  Vec3<T>::roundIfNearlyOne(T threshold)
  {
    if (isNearlyOne(x, threshold)) {
      x = static_cast<T>(1);
    }

    if (isNearlyOne(y, threshold)) {
      y = static_cast<T>(1);
    }

    if (isNearlyOne(z, threshold)) {
      z = static_cast<T>(1);
    }
  }

  //-----------------------------------------------------------

  template<typename T>
  inline void
  Vec3<T>::orientAroundNormal(Vec3<T> N,
                              bool    auto_flip)
  {
    Vec3<T> in(*this);
    // Flip normal to solve degenerate case of N.z near -1.0, then flip result back:
    if (N.z < 0.0f && auto_flip) {
      const Vec3<T> iN = -N;
      const T s = static_cast<T>(1) / (static_cast<T>(1) + N.z);
      x = -(in.x * ( iN.z + ( iN.y * iN.y * s)) + in.y * (        (-iN.x * iN.y * s)) + in.z * iN.x),
      y = -(in.x * (        (-iN.x * iN.y * s)) + in.y * ( iN.z + ( iN.x * iN.x * s)) + in.z * iN.y),
      z = -(in.x * (-iN.x                 ) + in.y * (-iN.y                 ) + in.z * iN.z);
    }
    // No flipping required:
    const T s = static_cast<T>(1) / (static_cast<T>(1) + N.z);
    x = in.x * ( N.z + ( N.y * N.y * s)) + in.y * (       (-N.x * N.y * s)) + in.z * N.x;
    y = in.x * (       (-N.x * N.y * s)) + in.y * ( N.z + ( N.x * N.x * s)) + in.z * N.y;
    z = in.x * (-N.x               ) + in.y * (-N.y               ) + in.z * N.z;
  }

  /*! Rotate a vector by an angle around a center axis vector.
      ex. Nnew = rotateAroundAxis(dir, -acosf(N.z), Vec3f(N.y, -N.x, 0.0f));
  */
  template<typename T>
  inline void
  Vec3<T>::rotateAroundAxis(T              angle,
                            const Vec3<T>& axis)
  {
    const T s  = std::sin(angle);
    const T c  = std::cos(angle);
    const T ic = (static_cast<T>(1) - c);
    Vec3<T> in(*this);
    x = in.x * (axis.x * axis.x * ic + c         ) + in.y * (axis.y * axis.x * ic - (axis.z * s)) + in.z * (axis.z * axis.x * ic + (axis.y * s));
    y = in.x * (axis.x * axis.y * ic + (axis.z * s)) + in.y * (axis.y * axis.y * ic + c         ) + in.z * (axis.z * axis.y * ic - (axis.x * s));
    z = in.x * (axis.x * axis.z * ic - (axis.y * s)) + in.y * (axis.y * axis.z * ic + (axis.x * s)) + in.z * (axis.z * axis.z * ic + c         );
  }

  //-----------------------------------------------------------

  //! Align a rotation angle to another angle by iteritively 'unwinding' it.
  template<typename T>
  inline T alignAngle(T angle,
                      T to)
  {
    while (std::abs(to - angle) > static_cast<T>(180)) {
      if (angle > to) {
        angle -= static_cast<T>(360);
      }
      else {
        angle += static_cast<T>(360);
      }
    }
    return angle;
  }


  template<typename T>
  inline void
  eulerFilterRotations(std::vector<Vec3<T> >& rotations_in_degrees,
                       RotateOrder            rot_order)
  {
    const size_t nKeys = rotations_in_degrees.size();
    if (nKeys < 2) {
      return; // don't bother...
    }

    // Get the axis order to rotate about:
    int axis0, axis1, axis2;
    switch (rot_order) {
      default:
      case RotateOrder::XYZ: axis0 = 0; axis1 = 1; axis2 = 2; break;
      case RotateOrder::XZY: axis0 = 0; axis1 = 2; axis2 = 1; break;
      case RotateOrder::YXZ: axis0 = 1; axis1 = 0; axis2 = 2; break;
      case RotateOrder::YZX: axis0 = 1; axis1 = 2; axis2 = 0; break;
      case RotateOrder::ZXY: axis0 = 2; axis1 = 0; axis2 = 1; break;
      case RotateOrder::ZYX: axis0 = 2; axis1 = 1; axis2 = 0; break;
    }

    // Find rotation keys with an angle change exceeding 180 degrees
    // and flip them by 180 if so:
    Vec3<T> prev, cur, flip;
    prev = rotations_in_degrees[0];
    for (size_t i = 1; i < nKeys; ++i) {
      Vec3<T>& rotation = rotations_in_degrees[i];
      cur.x = alignAngle(rotation.x, prev.x);
      cur.y = alignAngle(rotation.y, prev.y);
      cur.z = alignAngle(rotation.z, prev.z);
      flip[axis0] =  cur[axis0] + static_cast<T>(180);
      flip[axis1] =- cur[axis1] + static_cast<T>(180);
      flip[axis2] =  cur[axis2] + static_cast<T>(180);
      flip.x = alignAngle(flip.x, prev.x);
      flip.y = alignAngle(flip.y, prev.y);
      flip.z = alignAngle(flip.z, prev.z);
      // Update the rotation value from the smallest angle delta:
      const double dRot  = std::abs(prev.x -  cur.x) + std::abs(prev.y -  cur.y) + std::abs(prev.z -  cur.z);
      const double dFlip = std::abs(prev.x - flip.x) + std::abs(prev.y - flip.y) + std::abs(prev.z - flip.z);
      rotation = (dFlip < dRot) ? flip:cur;
      prev = cur;
    }
  }

}


#endif
