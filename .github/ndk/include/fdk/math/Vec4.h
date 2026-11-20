// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FDK_MATH_VEC4_H
#define FDK_MATH_VEC4_H

#include "fdk/math/Math.h"
#include "fdk/math/Vec2.h"
#include "fdk/math/Vec3.h"

#include <cmath>
#include <limits>
#include <type_traits>
#include <vector>


namespace fdk {

  template <typename T> class Mat4;


  //------------------------------------------------------------------------------


  //! \class fdk::Vec4
  //! 4-component vector
  template <typename T>
  class Vec4
  {
  public:
    using value_type = T;
    using reference = T&;
    using const_reference = const T&;

    static constexpr uint8_t kNumElements = 4; //< The number of elements in the vector

    static constexpr uint8_t X = 0; //< Constant index for the X axis
    static constexpr uint8_t Y = 1; //< Constant index for the Y axis
    static constexpr uint8_t Z = 2; //< Constant index for the Z axis
    static constexpr uint8_t W = 3; //< Constant index for the W axis


    // These are public for ease of access.
    T   x, y, z, w;     //!< the data


    /*---------------------------------------------------------*/
    /*                      Constructors                       */
    /*---------------------------------------------------------*/
    //! Default constructor leaves garbage in contents.
    Vec4() = default;

    //! Copy constructor supports type conversion.
    template<typename S>
    explicit Vec4(const Vec4<S>& b);

    //! Constructor that sets all components to a single value.
    template<typename S>
    explicit Vec4(S a) : x(a), y(a), z(a), w(a) {}

    //! Constructors that set separate components with defaults z=0 and w=1.
    Vec4(T a, T b)           : x(a), y(b), z(static_cast<T>(0)), w(static_cast<T>(1)) {}
    Vec4(T a, T b, T c)      : x(a), y(b), z(c), w(static_cast<T>(0)) {}
    Vec4(T a, T b, T c, T d) : x(a), y(b), z(c), w(d) {}

    //! Constructor from an array of 4 numbers.
    template<typename S>
    Vec4(      S* v) : x(v[0]), y(v[1]), z(v[2]), w(v[3]) {}

    template<typename S>
    Vec4(const S* v) : x(v[0]), y(v[1]), z(v[2]), w(v[3]) {}

    // Conversion from other vector sizes.
    template<typename S>
    Vec4(const Vec2<S>& v) : x(v.x), y(v.y), z(static_cast<T>(0)), w(static_cast<T>(1)) {}

    template<typename S>
    Vec4(const Vec2<S>& v,
         T              vz,
         T              vw = static_cast<T>(1)) : x(v.x), y(v.y), z(vz), w(vw) {}

    template<typename S>
    Vec4(const Vec3<S>& v) : x(v.x), y(v.y), z(v.z), w(static_cast<T>(1)) {}

    template<typename S>
    Vec4(const Vec3<S>& v,
         T              vw) : x(v.x), y(v.y), z(v.z), w(vw) {}


    //! Returns value as a specific type:
    operator Vec4<fdk::half>() const;
    operator Vec4<float>()     const;
    operator Vec4<double>()    const;
    operator Vec4<int>()       const;

    Vec4<fdk::half> asVec4h() const;
    Vec4<float>     asVec4f() const;
    Vec4<double>    asVec4d() const;
    Vec4<int>       asVec4i() const;

    const static Vec4 XAxis; //< Constant for the X axis vector
    const static Vec4 YAxis; //< Constant for the Y axis vector
    const static Vec4 ZAxis; //< Constant for the Z axis vector
    const static Vec4 WAxis; //< Constant for the W axis vector


    /*---------------------------------------------------------*/
    /*                     Component Access                    */
    /*---------------------------------------------------------*/
    T&       operator [] (int n)       { return *(&x + n); }
    const T& operator [] (int n) const { return *(&x + n); }

    //! Returns a pointer to the first element.
    T*        array()       { return &x; }
    const T*  array() const { return &x; }

    Vec2<T>   xy()  const { return Vec2<T>(x, y); }
    Vec3<T>   xyz() const { return Vec3<T>(x, y, z); }


    /*---------------------------------------------------------*/
    /*                        Assignment                       */
    /*---------------------------------------------------------*/
    //! Set xyz to a single value and w to 1.
    void      set(T d);

    //! Set all components, w defaults to 1.
    void      set(T a, T b, T c, T d = static_cast<T>(1));

    //! Set xyz and w separately.
    void      set(const Vec3<T>& v,
                  T              vw = static_cast<T>(1)) { set(v.x, v.y, v.z, vw); }

    //! Set components to 0.
    void      setToZero() { x = y = z = w = static_cast<T>(0); }

    //! Set components to 1.
    void      setToOne()  { x = y = z = w = static_cast<T>(1); }

    //! Type-specific clear. Set xyz to 0 and w to 1.
    void      clear();

    //! Add this to a fdk::Hash object.
    void      append(fdk::Hash& hash) const;


    /*---------------------------------------------------------*/
    /*                   Matrix Multiplication                 */
    /*---------------------------------------------------------*/
    template<typename S>
    Vec4  operator *  (const Mat4<S>& m) const; // (implemented inline in Mat4.h)

    template<typename S>
    Vec4& operator *= (const Mat4<S>& m);       // (implemented inline in Mat4.h)

    /*---------------------------------------------------------*/
    /*                       Multiplication                    */
    /*---------------------------------------------------------*/
    template<typename S, std::enable_if_t<fdk::is_arithmetic<S>, bool> = true>
    Vec4  operator *  (S d)           const { return Vec4(x * d, y * d, z * d, w * d); }
    Vec4  operator *  (const Vec4& v) const { return Vec4(x * v.x, y * v.y, z * v.z, w * v.w); }

    template<typename S, std::enable_if_t<fdk::is_arithmetic<S>, bool> = true>
    Vec4& operator *= (S d);
    Vec4& operator *= (const Vec4& v);

    /*---------------------------------------------------------*/
    /*                        Division                         */
    /*---------------------------------------------------------*/
    template<typename S, std::enable_if_t<fdk::is_arithmetic<S>, bool> = true>
    Vec4  operator /  (S d)           const { return Vec4(x / d, y / d, z / d, w / d); }
    Vec4  operator /  (const Vec4& v) const { return Vec4(x / v.x, y / v.y, z / v.z, w / v.w); }

    template<typename S, std::enable_if_t<fdk::is_arithmetic<S>, bool> = true>
    Vec4& operator /= (S d);
    Vec4& operator /= (const Vec4& v);

    /*---------------------------------------------------------*/
    /*                        Addition                         */
    /*---------------------------------------------------------*/
    template<typename S, std::enable_if_t<fdk::is_arithmetic<S>, bool> = true>
    Vec4  operator +  (S d)           const { return Vec4(x + d, y + d, z + d, w + d); }
    Vec4  operator +  (const Vec4& v) const { return Vec4(x + v.x, y + v.y, z + v.z, w + v.w); }

    template<typename S, std::enable_if_t<fdk::is_arithmetic<S>, bool> = true>
    Vec4& operator += (S d);
    Vec4& operator += (const Vec4& v);

    /*---------------------------------------------------------*/
    /*                       Subtraction                       */
    /*---------------------------------------------------------*/
    template<typename S, std::enable_if_t<fdk::is_arithmetic<S>, bool> = true>
    Vec4  operator -  (S d)           const { return Vec4(x - d, y - d, z - d, w - d); }
    Vec4  operator -  (const Vec4& v) const { return Vec4(x - v.x, y - v.y, z - v.z, w - v.w);}

    template<typename S, std::enable_if_t<fdk::is_arithmetic<S>, bool> = true>
    Vec4& operator -= (S d);
    Vec4& operator -= (const Vec4& v);

    /*---------------------------------------------------------*/
    /*                        Negation                         */
    /*---------------------------------------------------------*/
    Vec4  operator -  () const { return Vec4(-x, -y, -z, -w); }
    void negate();

    /*---------------------------------------------------------*/
    /*                        Equality                         */
    /*---------------------------------------------------------*/
    bool operator == (const Vec4& v) const { return (x == v.x && y == v.y && z == v.z && w == v.w); }
    bool operator != (const Vec4& v) const { return (x != v.x || y != v.y || z != v.z || w != v.w); }
    bool operator == (T d)           const { return (x == d && y == d && z == d && w == d); }
    bool operator != (T d)           const { return (x != d || y != d || z != d || w != d); }

    /*---------------------------------------------------------*/
    /*                         Sorting                         */
    /*---------------------------------------------------------*/
    //! Fairly arbitrary operator so you can store these in ordered arrays
    bool operator <  (const Vec4& v) const;


    /*---------------------------------------------------------*/
    /*                     Vector Functions                    */
    /*---------------------------------------------------------*/

    //! Divide x, y, and z by w. If w is zero, the behaviour is undefined.
    Vec4&     wNormalize();
    Vec3<T>   wNormalized() const { return Vec3<T>(x / w, y / w, z / w); }

    //! Trims off w, same as xyz() method.
    Vec3<T>   wTruncate()   const { return xyz(); }

    //! Returns the minimum XYZ element - w is ignored.
    T         minimumComponent()     const { return std::min(x, std::min(y, z)); }
    Vec4      minimum(const Vec4& v) const { return Vec4(std::min(v.x, x), std::min(v.y, y), std::min(v.z, z)); }

    //! Returns the maximum XYZ element - w is ignored.
    T         maximumComponent()     const { return std::max(x, std::max(y, z)); }
    Vec4      maximum(const Vec4& v) const { return Vec4(std::max(v.x, x), std::max(v.y, y), std::max(v.z, z)); }

    //! Returns the absolute value of the largest XYZ element - w is ignored.
    T         largestAxis() const { return std::max(std::abs(x), std::max(std::abs(y), std::abs(z))); }

    //! Linear-interpolate between this Vec4 and another at t, where t=0..1.
    template<typename S>
    Vec4<T>   interpolateTo(const Vec4<T>& b,
                            S              t) const;

    Vec4      inverted() const { return Vec4(invert(x), invert(y), invert(z), invert(w)); }

    template<typename S>
    Vec4<T>   lerpClampedTo(const Vec4<T>& b,
                            S              t) const;

    template<typename S>
    Vec4<T>   lerpUnclampedTo(const Vec4<T>& b,
                              S              t) const;
    template<typename S>
    Vec4      clamped(S min, S max) const { return Vec4(std::clamp<T>(x, min, max),
                                                        std::clamp<T>(y, min, max),
                                                        std::clamp<T>(z, min, max),
                                                        std::clamp<T>(w, min, max)); }

    Vec4      clamped() const { return clamped(static_cast<T>(0), static_cast<T>(1)); }

    bool      isZero()          const { return !notZero(); }
    bool      notZero()         const { return  (x != static_cast<T>(0) 
                                                 || y != static_cast<T>(0)
                                                 || z != static_cast<T>(0)
                                                 || w != static_cast<T>(0)); }
    bool      greaterThanZero() const { return (x > static_cast<T>(0)
                                                || y > static_cast<T>(0)
                                                || z > static_cast<T>(0)
                                                || w > static_cast<T>(0)); }


  private:
    // This must match for Vec/Mat/Box classes:
    static_assert(fdk::is_arithmetic<T>, "Vec4 can only be instantiated with an arithmetic type.");

  };


  /*--------------------------------------*/
  /*    Aliases for supported data types  */
  /*--------------------------------------*/
  using Vec4f = Vec4<float>;
  using Vec4d = Vec4<double>;
  using Vec4i = Vec4<int32_t>;
  using Vec4h = Vec4<fdk::half>;


  template <typename T> const Vec4<T> Vec4<T>::XAxis(static_cast<T>(1), static_cast<T>(0), static_cast<T>(0), static_cast<T>(0));
  template <typename T> const Vec4<T> Vec4<T>::YAxis(static_cast<T>(0), static_cast<T>(1), static_cast<T>(0), static_cast<T>(0));
  template <typename T> const Vec4<T> Vec4<T>::ZAxis(static_cast<T>(0), static_cast<T>(0), static_cast<T>(1), static_cast<T>(0));
  template <typename T> const Vec4<T> Vec4<T>::WAxis(static_cast<T>(0), static_cast<T>(0), static_cast<T>(0), static_cast<T>(1));

  /*----------------------------------*/
  /*        Static operations         */
  /*----------------------------------*/

  //! Print out Vec4 components to a stream.
  template<typename T>
  std::ostream& operator << (std::ostream& o, const Vec4<T>& v);

  //! Multiply a vector by a scalar returning a new vector.
  template<typename T, typename S, std::enable_if_t<fdk::is_arithmetic<S>, bool> = true>
  Vec4<T> operator * (S a, const Vec4<T>& b) {  return Vec4<T>(b.x * a, b.y * a, b.z * a, b.w * a); }

  //! Divide a vector by a scalar returning a new vector.
  template<typename T, typename S>
  Vec4<T> operator / (S a, const Vec4<T>& b) { return Vec4<T>(a / b.x, a / b.y, a / b.z, a / b.w); }

  //! Add a vector by a scalar returning a new vector.
  template<typename T, typename S>
  Vec4<T> operator + (S a, const Vec4<T>& b) { return Vec4<T>(a + b.x, a + b.y, a + b.z, a + b.w); }

  //! Subtract a vector by a scalar returning a new vector.
  template<typename T, typename S>
  Vec4<T> operator - (S a, const Vec4<T>& b) { return Vec4<T>(a - b.x, a - b.y, a - b.z, a - b.w); }


  //! Linear-interpolate between two Vec3s at t, where t=0..1.
  template<typename T, typename S>
  Vec4<T>     lerpClamped(const Vec4<T>& v0,
                          const Vec4<T>& v1,
                          S              t);

  //! Linear-interpolate between two Vec3s at t, where t=0..1, and inv is 1-t.
  template<typename T, typename S>
  Vec4<T>     lerpClamped(const Vec4<T>& v0,
                          const Vec4<T>& v1,
                          S              t,
                          S              invt);

  //! Unclamped linear-interpolate between two Vec3s at t, where t=0..1.
  template<typename T, typename S>
  Vec4<T>     lerpUnclamped(const Vec4<T>& v0,
                            const Vec4<T>& v1,
                            S              t);

  //! Unclamped linear-interpolate between two Vec3s at t, where t=0..1, and inv is 1-t.
  template<typename T, typename S>
  Vec4<T>     lerpUnclamped(const Vec4<T>& v0,
                            const Vec4<T>& v1,
                            S              t,
                            S              invt);

  //! Interpolate between three Vec3s at barycentric coord st.
  template<typename T, typename S>
  Vec4<T>     interpolateAtBaryCoord(const Vec4<T>& v0,
                                     const Vec4<T>& v1,
                                     const Vec4<T>& v2,
                                     const Vec2<S>& st);

  //! Interpolate between three Vec4s at barycentric coord st, with derivatives.
  template<typename T, typename S>
  void        interpolateAtBaryCoord(const Vec4<T>& v0,
                                     const Vec4<T>& v1,
                                     const Vec4<T>& v2,
                                     const Vec2<S>& st,
                                     const Vec2<S>& stdx,
                                     const Vec2<S>& stdy,
                                     Vec4<T>&       out,
                                     Vec4<T>&       duout,
                                     Vec4<T>&       dvout);



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
  Vec4<T>::Vec4(const Vec4<S>& b)
  {
    x = static_cast<T>(b.x);
    y = static_cast<T>(b.y);
    z = static_cast<T>(b.z);
    w = static_cast<T>(b.w);
  }

  template<typename T>
  inline std::ostream& operator << (std::ostream& o, const Vec4<T>& v)
  {
    o << '[' << v.x << ' ' << v.y << ' ' << v.z << ' ' << v.w << ']';
    return o;
  }

  //-----------------------------------------------------------

  template<typename T>
  inline Vec4<fdk::half>
  Vec4<T>::asVec4h() const
  {
    if (std::is_same<fdk::half, T>::value) {
      return *this;
    }
    else {
      return Vec4<fdk::half>(fdk::half(static_cast<float>(x)), fdk::half(static_cast<float>(y)), fdk::half(static_cast<float>(z)), fdk::half(static_cast<float>(w)));
    }
  }

  template<typename T>
  inline Vec4<T>::operator Vec4<fdk::half>() const { return asVec4h(); }

  template<typename T>
  inline Vec4<float>
  Vec4<T>::asVec4f() const
  {
    if (std::is_same<float, T>::value) {
      return *this;
    }
    else {
      return Vec4<float>(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), static_cast<float>(w));
    }
  }

  template<typename T>
  inline Vec4<T>::operator Vec4<float>() const { return asVec4f(); }

  template<typename T>
  inline Vec4<double>
  Vec4<T>::asVec4d() const
  {
    if (std::is_same<double, T>::value) {
      return *this;
    }
    else {
      return Vec4<double>(static_cast<double>(x), static_cast<double>(y), static_cast<double>(z), static_cast<double>(w));
    }
  }

  template<typename T>
  inline Vec4<T>::operator Vec4<double>() const { return asVec4d(); }

  template<typename T>
  inline Vec4<int>
  Vec4<T>::asVec4i() const
  {
    if (std::is_same<int, T>::value) {
      return *this;
    }
    else {
      return Vec4<int>(static_cast<int>(x), static_cast<int>(y), static_cast<int>(z), static_cast<int>(w));
    }
  }

  template<typename T>
  inline Vec4<T>::operator Vec4<int>() const { return asVec4i(); }

  //-----------------------------------------------------------

  template<typename T>
  inline void
  Vec4<T>::set(T d)
  {
    x = y = z = d;
    w = static_cast<T>(1);
  }

  template<typename T>
  inline void
  Vec4<T>::set(T a, T b, T c, T d)
  {
    x = a;
    y = b;
    z = c;
    w = d;
  }

  template<typename T>
  inline void
  Vec4<T>::clear()
  {
    x = y = z = static_cast<T>(0);
    w = static_cast<T>(1);
  }

  template<typename T>
  template<typename S, std::enable_if_t<fdk::is_arithmetic<S>, bool>>
  inline Vec4<T>&
  Vec4<T>::operator *= (S d)
  {
    x *= d;
    y *= d;
    z *= d;
    w *= d;
    return *this;
  }

  template<typename T>
  inline Vec4<T>&
  Vec4<T>::operator *= (const Vec4<T>& v)
  {
    x *= v.x;
    y *= v.y;
    z *= v.z;
    w *= v.w;
    return *this;
  }

  template<typename T>
  template<typename S, std::enable_if_t<fdk::is_arithmetic<S>, bool>>
  inline Vec4<T>&
  Vec4<T>::operator /= (S d)
  {
    x /= d;
    y /= d;
    z /= d;
    w /= d;
    return *this;
  }

  template<typename T>
  inline Vec4<T>&
  Vec4<T>::operator /= (const Vec4<T>& v)
  {
    x /= v.x;
    y /= v.y;
    z /= v.z;
    w /= v.w;
    return *this;
  }

  template<typename T>
  template<typename S, std::enable_if_t<fdk::is_arithmetic<S>, bool>>
  inline Vec4<T>&
  Vec4<T>::operator += (S d)
  {
    x += d;
    y += d;
    z += d;
    w += d;
    return *this;
  }

  template<typename T>
  inline Vec4<T>&
  Vec4<T>::operator += (const Vec4<T>& v)
  {
    x += v.x;
    y += v.y;
    z += v.z;
    w += v.w;
    return *this;
  }

  template<typename T>
  template<typename S, std::enable_if_t<fdk::is_arithmetic<S>, bool>>
  inline Vec4<T>&
  Vec4<T>::operator -= (S d)
  {
    x -= d;
    y -= d;
    z -= d;
    w -= d;
    return *this;
  }

  template<typename T>
  inline Vec4<T>&
  Vec4<T>::operator -= (const Vec4<T>& v)
  {
    x -= v.x;
    y -= v.y;
    z -= v.z;
    w -= v.w;
    return *this;
  }

  template<typename T>
  inline void
  Vec4<T>::negate()
  {
    x = -x;
    y = -y;
    z = -z;
    w = -w;
  }

  template<typename T>
  inline Vec4<T>&
  Vec4<T>::wNormalize()
  {
    x /= w;
    y /= w;
    z /= w;
    w = static_cast<T>(1);
    return *this;
  }

  //-----------------------------------------------------------

  template<typename T>
  inline bool
  Vec4<T>::operator < (const Vec4<T>& v) const
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

    if (z < v.z) {
      return true;
    }

    if (z > v.z) {
      return false;
    }

    return (w < v.w);
  }

  template<typename T>
  template<typename S>
  inline Vec4<T>
  Vec4<T>::interpolateTo(const Vec4<T>& b,
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
    return Vec4<T>(x * invtT + b.x * tT, y * invtT + b.y * tT, z * invtT + b.z * tT, w * invtT + b.w * tT);
  }

  template<typename T>
  template<typename S>
  inline Vec4<T>
  Vec4<T>::lerpClampedTo(const Vec4<T>& b, S t) const
  {
    return interpolateTo(b, t);
  }


  template<typename T>
  template<typename S>
  inline Vec4<T>
  Vec4<T>::lerpUnclampedTo(const Vec4<T>& b, S t) const
  {
    const T tT    = static_cast<T>(t);
    const T invtT = static_cast<T>(1) - tT;
    return Vec4<T>(x * invtT + b.x * tT, y * invtT + b.y * tT, z * invtT + b.z * tT, w * invtT + b.w * tT);
  }

  template<typename T, typename S>
  inline Vec4<T>
  lerpClamped(const Vec4<T>& v0,
       const Vec4<T>& v1,
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
    return Vec4<T>(v0.x * invtT + v1.x * tT, v0.y * invtT + v1.y * tT, v0.z * invtT + v1.z * tT);
  }

  template<typename T, typename S>
  inline Vec4<T>
  lerpClamped(const Vec4<T>& v0,
       const Vec4<T>& v1,
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
    return Vec4<T>(v0.x * invtT + v1.x * tT, v0.y * invtT + v1.y * tT, v0.z * invtT + v1.z * tT, v0.w * invtT + v1.w * tT);
  }

  //! Unclamped linear-interpolate between two Vec3s at t, where t=0..1.
  template<typename T, typename S>
  inline Vec4<T>
  lerpUnclamped(const Vec4<T>& v0,
                const Vec4<T>& v1,
                S              t)
  {
    const T tT    = static_cast<T>(t);
    const T invtT = static_cast<T>(1) - tT;
    return Vec4<T>(v0.x * invtT + v1.x * tT, v0.y * invtT + v1.y * tT, v0.z * invtT + v1.z * tT);
  }

  //! Unclamped linear-interpolate between two Vec3s at t, where t=0..1, and inv is 1-t.
  template<typename T, typename S>
  inline Vec4<T>
  lerpUnclamped(const Vec4<T>& v0,
                const Vec4<T>& v1,
                S              t,
                S              invt)
  {
    const T tT    = static_cast<T>(t);
    const T invtT = static_cast<T>(invt);
    return Vec4<T>(v0.x * invtT + v1.x * tT, v0.y * invtT + v1.y * tT, v0.z * invtT + v1.z * tT, v0.w * invtT + v1.w * tT);
  }

  // Interpolate between three Vec4s at barycentric coord st.
  template<typename T, typename S>
  inline Vec4<T>
  interpolateAtBaryCoord(const Vec4<T>& v0,
                         const Vec4<T>& v1,
                         const Vec4<T>& v2,
                         const Vec2<S>& st)
  {
    return Vec4<T>(v0 + ((v1 - v0) * static_cast<T>(st.x)) + ((v2 - v0) * static_cast<T>(st.y)));
  }

  // Interpolate between three Vec3s at barycentric coord st, with derivatives.
  template<typename T, typename S>
  inline void
  interpolateAtBaryCoord(const Vec4<T>& v0,
                         const Vec4<T>& v1,
                         const Vec4<T>& v2,
                         const Vec2<S>& st,
                         const Vec2<S>& stdx,
                         const Vec2<S>& stdy,
                         Vec4<T>&       out,
                         Vec4<T>&       duout,
                         Vec4<T>&       dvout)
  {
    const Vec4<T> e01 = (v1 - v0);
    const Vec4<T> e02 = (v2 - v0);
    const Vec4<T> dt = (e01 * static_cast<T>(st.x)) + (e02 * static_cast<T>(st.y));
    out   = v0 + dt;
    duout = (e01 * T(stdx.x)) + (e02 * static_cast<T>(stdx.y)) - dt;
    dvout = (e01 * T(stdy.x)) + (e02 * static_cast<T>(stdy.y)) - dt;
  }

}

#endif
