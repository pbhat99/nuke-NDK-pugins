// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FDK_MATH_VEC2_H
#define FDK_MATH_VEC2_H

#include "fdk/math/Math.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <vector>

namespace fdk {
  class Hash;
  template <typename T> class Mat4;

  //! \class fdk::Vec2
  //! 2-component vector
  template <typename T>
  class Vec2
  {
  public:
    using value_type = T;
    using reference = T&;
    using const_reference = const T&;

    static constexpr uint8_t kNumElements = 2; //< The number of elements in the vector

    static constexpr uint8_t X = 0; //< Constant index for the X axis
    static constexpr uint8_t Y = 1; //< Constant index for the Y axis

    // These are public for ease of access.
    T   x, y;       //!< the data


    /*---------------------------------------------------------*/
    /*                      Constructors                       */
    /*---------------------------------------------------------*/
    //! Default constructor leaves garbage in contents.
    Vec2() = default;

    //! Copy constructor supports type conversion.
    template<typename S>
    explicit Vec2(const Vec2<S>& b);

    //! Constructor that sets all components to a single value.
    explicit Vec2(T a) : x(a), y(a) {}

    //! Constructor that sets separate components.
    Vec2(T a, T b) : x(a), y(b) {}

    //! Constructor from an array of 2 numbers.
    template<typename S>
    Vec2(      S* v) : x(v[0]), y(v[1]) {}
    template<typename S>
    Vec2(const S* v) : x(v[0]), y(v[1]) {}

    //! Returns value as a specific type:
    operator Vec2<fdk::half>() const;
    operator Vec2<float>()     const;
    operator Vec2<double>()    const;
    operator Vec2<int>()       const;

    Vec2<fdk::half> asVec2h() const;
    Vec2<float>     asVec2f() const;
    Vec2<double>    asVec2d() const;
    Vec2<int>       asVec2i() const;

    const static Vec2 XAxis; //< Constant for the X axis vector
    const static Vec2 YAxis; //< Constant for the Y axis vector

    /*---------------------------------------------------------*/
    /*                     Component Access                    */
    /*---------------------------------------------------------*/
    T&       operator [] (int n)       { return *(&x + n); }
    const T& operator [] (int n) const { return *(&x + n); }

    //! Returns a pointer to the first element.
    T*        array()       { return &x; }
    const T*  array() const { return &x; }


    /*---------------------------------------------------------*/
    /*                        Assignment                       */
    /*---------------------------------------------------------*/
    //! Set all components to a single value.
    void      set(T d) { x = y = d; }

    //! Set all components.
    void      set(T a, T b);

    //! Set components to 0.
    void      setToZero() { x = y = static_cast<T>(0); }

    //! Set components to 1.
    void      setToOne()  { x = y = static_cast<T>(1); }

    //! Type-specific clear. Sets all components to 0.
    void      clear() { setToZero(); }

    //! Add this to a fdk::Hash object.
    void      append(fdk::Hash& hash) const;


    /*---------------------------------------------------------*/
    /*                   Matrix Multiplication                 */
    /*---------------------------------------------------------*/
    template<typename S>
    Vec2  operator *  (const Mat4<S>& m) const;
    template<typename S>
    Vec2& operator *= (const Mat4<S>& m);


    /*---------------------------------------------------------*/
    /*                       Multiplication                    */
    /*---------------------------------------------------------*/
    template<typename S, std::enable_if_t<fdk::is_arithmetic<S>, bool> = true>
    Vec2  operator *  (S d)           const { return Vec2(x*d, y*d); }
    Vec2  operator *  (const Vec2& v) const { return Vec2(x * v.x, y * v.y); }
    template<typename S, std::enable_if_t<fdk::is_arithmetic<S>, bool> = true>
    Vec2& operator *= (S d);
    Vec2& operator *= (const Vec2& v);

    /*---------------------------------------------------------*/
    /*                        Division                         */
    /*---------------------------------------------------------*/
    template<typename S, std::enable_if_t<fdk::is_arithmetic<S>, bool> = true>
    Vec2  operator /  (S d)           const { return Vec2(x / d, y / d); }
    Vec2  operator /  (const Vec2& v) const { return Vec2(x / v.x, y / v.y); }
    template<typename S, std::enable_if_t<fdk::is_arithmetic<S>, bool> = true>
    Vec2& operator /= (S d);
    Vec2& operator /= (const Vec2& v);


    /*---------------------------------------------------------*/
    /*                        Addition                         */
    /*---------------------------------------------------------*/
    template<typename S, std::enable_if_t<fdk::is_arithmetic<S>, bool> = true>
    Vec2  operator +  (S d)           const { return Vec2(x + d, y + d); }
    Vec2  operator +  (const Vec2& v) const { return Vec2(x + v.x, y + v.y); }
    template<typename S, std::enable_if_t<fdk::is_arithmetic<S>, bool> = true>
    Vec2& operator += (S d);
    Vec2& operator += (const Vec2& v);

    /*---------------------------------------------------------*/
    /*                       Subtraction                       */
    /*---------------------------------------------------------*/
    template<typename S, std::enable_if_t<fdk::is_arithmetic<S>, bool> = true>
    Vec2  operator -  (S d)           const { return Vec2(x - d, y - d); }
    Vec2  operator -  (const Vec2& v) const { return Vec2(x - v.x, y - v.y); }
    template<typename S, std::enable_if_t<fdk::is_arithmetic<S>, bool> = true>
    Vec2& operator -= (S d);
    Vec2& operator -= (const Vec2& v);

    /*---------------------------------------------------------*/
    /*                        Negation                         */
    /*---------------------------------------------------------*/
    Vec2  operator -  () const { return Vec2(-x, -y); }
    void negate();


    /*---------------------------------------------------------*/
    /*                        Equality                         */
    /*---------------------------------------------------------*/
    bool operator == (const Vec2& v) const { return (x == v.x && y == v.y); }
    bool operator != (const Vec2& v) const { return (x != v.x || y != v.y); }
    bool operator == (T d)           const { return (x == d && y == d); }
    bool operator != (T d)           const { return (x != d || y != d); }


    /*---------------------------------------------------------*/
    /*                         Sorting                         */
    /*---------------------------------------------------------*/
    //! Fairly arbitrary operator so you can store these in ordered arrays
    bool operator <  (const Vec2& v) const;


    /*---------------------------------------------------------*/
    /*                     Vector Functions                    */
    /*---------------------------------------------------------*/
    //! Also known as the absolute value or magnitude of the vector.
    T         length() const { return std::sqrt(x * x + y * y); }

    //! Same as this dot this, length() squared.
    T         lengthSquared() const { return (x * x + y * y); }

    //! Same as (this-v).length()
    T         distanceBetween(const Vec2& v) const { return std::sqrt((x - v.x) * (x - v.x) + (y - v.y) * (y - v.y)); }

    //! Same as (this-v).lengthSquared()
    T         distanceSquared(const Vec2& v) const { return ((x - v.x) * (x - v.x) + (y - v.y) * (y - v.y)); }

    //! Dot product.
    T         dot(const Vec2& v) const { return (x * v.x + y * v.y); }

    //! Returns the Z component of the cross product, Ux * Vy - Uy * Vx.
    T         cross(const Vec2& v) const { return (x * v.y - y * v.x); }

    //! Change the vector to be unit length. Returns the original length.
    T         normalize();

    //! Returns the normalized unit vector form of this vector.
    //! If this vector is null, then a null vector is returned.
    Vec2      normalized() const;

    Vec2      inverted() const { return Vec2(invert(x), invert(y)); }

    template<typename S>
    Vec2      clamped(S min, S max) const { return Vec2(std::clamp<T>(x, min, max), std::clamp<T>(y, min, max)); }

    Vec2      clamped() const { return clamped(static_cast<T>(0), static_cast<T>(1)); }

    //! Returns the minimum element.
    T         minimumComponent()     const { return std::min(x, y); }
    Vec2      minimum(const Vec2& v) const { return Vec2(std::min(v.x, x), std::min(v.y, y));}

    //! Returns the maximum element.
    T         maximumComponent()     const { return std::max(x, y); }
    Vec2      maximum(const Vec2& v) const { return Vec2(std::max(v.x, x), std::max(v.y, y));}

    //! Returns the absolute value of the largest element.
    T         largestAxis() const { return std::max(std::abs(x), std::abs(y)); }

    //! Linear-interpolate between this Vec2 and another at t, where t=0..1.
    template<typename S>
    Vec2<T>   interpolateTo(const Vec2<T>& b,
                            S              t) const;

    template<typename S>
    Vec2<T>   lerpClampedTo(const Vec2<T>& b,
                            S              t) const;

    template<typename S>
    Vec2<T>   lerpUnclampedTo(const Vec2<T>& b,
                              S              t) const;

    bool      isZero()          const { return !notZero(); }
    bool      notZero()         const { return  (x != static_cast<T>(0) || y != static_cast<T>(0)); }
    bool      greaterThanZero() const { return (x > static_cast<T>(0) || y > static_cast<T>(0)); }

    //! Round off each element if nearly one or zero within the type's epsilon default or user-provided threshold.
    void      roundIfNearlyZero(T threshold = std::numeric_limits<T>::epsilon());
    void      roundIfNearlyOne(T threshold = std::numeric_limits<T>::epsilon());


  private:
    // This must match for Vec/Mat/Box classes:
    static_assert(fdk::is_arithmetic<T>, "Vec2 can only be instantiated with an arithmetic type.");

  };


  /*--------------------------------------*/
  /*    Aliases for supported data types  */
  /*--------------------------------------*/
  using Vec2f = Vec2<float>;
  using Vec2d = Vec2<double>;
  using Vec2i = Vec2<int32_t>;
  using Vec2h = Vec2<fdk::half>;

  template <typename T> const Vec2<T> Vec2<T>::XAxis(static_cast<T>(1), static_cast<T>(0));
  template <typename T> const Vec2<T> Vec2<T>::YAxis(static_cast<T>(0), static_cast<T>(1));

  /*----------------------------------*/
  /*        Static operations         */
  /*----------------------------------*/
  //! Print out Vec2 components to a stream.
  template<typename T>
  std::ostream& operator << (std::ostream& o, const Vec2<T>& v);


  //! Return the dot product of vectors a and b.
  template<typename T>
  T           dot(const Vec2<T>& a,
                  const Vec2<T>& b) { return a.dot(b); }

  //! Returns the cross-product of vectors a and b, which corresponds to the normal vector of a plane defined by a and b.
  template<typename T>
  T           cross(const Vec2<T>& a,
                    const Vec2<T>& b) { return a.cross(b); }


  //! Multiply a vector by a scalar returning a new vector.
  template<typename T, typename S>
  Vec2<T> operator * (S a, const Vec2<T>& b) {  return Vec2<T>(b.x * a, b.y * a); }

  //! Divide a scalar by a vector returning a new vector.
  template<typename T, typename S>
  Vec2<T> operator/(S a, const Vec2<T>& b) { return Vec2<T>(a / b.x, a / b.y); }

  //! Add a scalar to a vector returning a new vector.
  template<typename T, typename S>
  Vec2<T> operator+(S a, const Vec2<T>& b) { return Vec2<T>(a + b.x, a + b.y); }

  //! Subtract a scalar to a vector returning a new vector.
  template<typename T, typename S>
  Vec2<T> operator-(S a, const Vec2<T>& b) { return Vec2<T>(a - b.x, a - b.y); }

  //! Linear-interpolate between two Vec2s at t, where t=0..1.
  template<typename T, typename S>
  Vec2<T>     lerpClamped(const Vec2<T>& v0,
                          const Vec2<T>& v1,
                          S              t);

  //! Linear-interpolate between two Vec2s at t, where t=0..1, and inv is 1-t.
  template<typename T, typename S>
  Vec2<T>     lerpClamped(const Vec2<T>& v0,
                          const Vec2<T>& v1,
                          S              t,
                          S              invt);

  //! Linear-interpolate between two Vec2s at t, where t=0..1.
  template<typename T, typename S>
  Vec2<T>     lerpUnclamped(const Vec2<T>& v0,
                            const Vec2<T>& v1,
                            S              t);

  //! Linear-interpolate between two Vec2s at t, where t=0..1, and inv is 1-t.
  template<typename T, typename S>
  Vec2<T>     lerpUnclamped(const Vec2<T>& v0,
                            const Vec2<T>& v1,
                            S              t,
                            S              invt);

  //! Interpolate between three Vec2s at barycentric coord st.
  template<typename T, typename S>
  Vec2<T>     interpolateAtBaryCoord(const Vec2<T>& v0,
                                     const Vec2<T>& v1,
                                     const Vec2<T>& v2,
                                     const Vec2<S>& st);

  //! Interpolate between three Vec2s at barycentric coord st, with derivatives.
  template<typename T, typename S>
  void        interpolateAtBaryCoord(const Vec2<T>& v0,
                                     const Vec2<T>& v1,
                                     const Vec2<T>& v2,
                                     const Vec2<S>& st,
                                     const Vec2<S>& stdx,
                                     const Vec2<S>& stdy,
                                     Vec2<T>&       out,
                                     Vec2<T>&       duout,
                                     Vec2<T>&       dvout);



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
  Vec2<T>::Vec2(const Vec2<S>& b)
  {
    x = static_cast<T>(b.x);
    y = static_cast<T>(b.y);
  }

  template<typename T>
  inline std::ostream& operator << (std::ostream& o, const Vec2<T>& v)
  {
    o << '[' << v.x << ' ' << v.y << ']';
    return o;
  }

  template<typename T>
  inline Vec2<fdk::half>
  Vec2<T>::asVec2h() const
  {
    if (std::is_same<fdk::half, T>::value) {
      return *this;
    }
    else {
      return Vec2<fdk::half>(fdk::half(static_cast<float>(x)), fdk::half(static_cast<float>(y)));
    }
  }
  template<typename T>
  inline Vec2<T>::operator Vec2<fdk::half>() const { return asVec2h(); }

  template<typename T>
  inline Vec2<float>
  Vec2<T>::asVec2f() const
  {
    if (std::is_same<float, T>::value) {
      return *this;
    }
    else {
      return Vec2<float>(static_cast<float>(x), static_cast<float>(y));
    }
  }
  template<typename T>
  inline Vec2<T>::operator Vec2<float>() const { return asVec2f(); }

  template<typename T>
  inline Vec2<double>
  Vec2<T>::asVec2d() const
  {
    if (std::is_same<double, T>::value) {
      return *this;
    }
    else {
      return Vec2<double>(static_cast<double>(x), static_cast<double>(y));
    }
  }
  template<typename T>
  inline Vec2<T>::operator Vec2<double>() const { return asVec2d(); }

  template<typename T>
  inline Vec2<int>
  Vec2<T>::asVec2i() const
  {
    if (std::is_same<int, T>::value) {
      return *this;
    }
    else {
      return Vec2<int>(static_cast<int>(x), static_cast<int>(y));
    }
  }
  template<typename T>
  inline Vec2<T>::operator Vec2<int>() const { return asVec2i(); }

  template<typename T>
  inline void
  Vec2<T>::set(T a, T b)
  {
    x = a;
    y = b;
  }

  template<typename T>
  inline bool
  Vec2<T>::operator < (const Vec2<T>& v) const
  {
    if (x < v.x) {
      return true;
    }

    if (x > v.x) {
      return false;
    }

    return (y < v.y);
  }

  template<typename T>
  template<typename S, std::enable_if_t<fdk::is_arithmetic<S>, bool>>
  inline Vec2<T>&
  Vec2<T>::operator *= (S d)
  {
    x *= d;
    y *= d;
    return *this;
  }

  template<typename T>
  inline Vec2<T>&
  Vec2<T>::operator *= (const Vec2<T>& v)
  {
    x *= v.x;
    y *= v.y;
    return *this;
  }

  template<typename T>
  template<typename S, std::enable_if_t<fdk::is_arithmetic<S>, bool>>
  inline Vec2<T>&
  Vec2<T>::operator /= (S d)
  {
    x /= d;
    y /= d;
    return *this;
  }

  template<typename T>
  inline Vec2<T>&
  Vec2<T>::operator /= (const Vec2<T>& v)
  {
    x /= v.x;
    y /= v.y;
    return *this;
  }

  template<typename T>
  template<typename S, std::enable_if_t<fdk::is_arithmetic<S>, bool>>
  inline Vec2<T>&
  Vec2<T>::operator += (S d)
  {
    x += d;
    y += d;
    return *this;
  }

  template<typename T>
  inline Vec2<T>&
  Vec2<T>::operator += (const Vec2<T>& v)
  {
    x += v.x;
    y += v.y;
    return *this;
  }

  template<typename T>
  template<typename S, std::enable_if_t<fdk::is_arithmetic<S>, bool>>
  inline Vec2<T>&
  Vec2<T>::operator -= (S d)
  {
    x -= d;
    y -= d;
    return *this;
  }

  template<typename T>
  inline Vec2<T>&
  Vec2<T>::operator -= (const Vec2<T>& v)
  {
    x -= v.x;
    y -= v.y;
    return *this;
  }

  template<typename T>
  inline void
  Vec2<T>::negate()
  {
    x = -x;
    y = -y;
  }

  template<typename T>
  inline T
  Vec2<T>::normalize()
  {
    const T d = length();
    if (d) {
      x /= d;
      y /= d;
    }

    return d;
  }

  template<typename T>
  inline Vec2<T>
  Vec2<T>::normalized() const
  {
    const T d = length();
    return (d > static_cast<T>(0)) ? Vec2<T>(x / d, y / d) : *this * d;
  }

  template<typename T>
  template<typename S>
  inline Vec2<T>
  Vec2<T>::interpolateTo(const Vec2<T>& b,
                         S              t) const
  {
    if (t < std::numeric_limits<S>::epsilon()) {
      return *this; // before or at first
    }
    else if (t > (static_cast<S>(1) - std::numeric_limits<S>::epsilon())) {
       return b; // at or after last
    }

    const T tT    = T(t);
    const T invtT = static_cast<T>(1) - tT;
    return Vec2<T>(x * invtT + b.x * tT, y * invtT + b.y * tT);
  }

  template<typename T>
  template<typename S>
  inline Vec2<T>
  Vec2<T>::lerpClampedTo(const Vec2<T>& b, S t) const
  {
    return interpolateTo(b, t);
  }

  template<typename T>
  template<typename S>
  inline Vec2<T>
  Vec2<T>::lerpUnclampedTo(const Vec2<T>& b, S t) const
  {
    const T tT    = T(t);
    const T invtT = static_cast<T>(1) - tT;
    return Vec2<T>(x * invtT + b.x * tT, y * invtT + b.y * tT);
  }

  template<typename T, typename S>
  inline Vec2<T>
  lerpClamped(const Vec2<T>& v0,
       const Vec2<T>& v1,
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
    return Vec2<T>(v0.x * invtT + v1.x * tT, v0.y * invtT + v1.y * tT);
  }

  template<typename T, typename S>
  inline Vec2<T>
  lerpClamped(const Vec2<T>& v0,
       const Vec2<T>& v1,
       S              t,
       S              invt)
  {
    if (t < std::numeric_limits<S>::epsilon()) {
      return v0; // before or at first
    }
    else if (t > (static_cast<S>(1) - std::numeric_limits<S>::epsilon())) {
      return v1; // at or after last
    }

    const T tT    = T(t);
    const T invtT = T(invt);
    return Vec2<T>(v0.x * invtT + v1.x * tT, v0.y * invtT + v1.y * tT);
  }

  //! Linear-interpolate between two Vec2s at t, where t=0..1.
  template<typename T, typename S>
  inline Vec2<T>
  lerpUnclamped(const Vec2<T>& v0,
                const Vec2<T>& v1,
                S              t)
  {
    const T tT    = static_cast<T>(t);
    const T invtT = static_cast<T>(1) - tT;
    return Vec2<T>(v0.x * invtT + v1.x * tT, v0.y * invtT + v1.y * tT);
  }

  //! Linear-interpolate between two Vec2s at t, where t=0..1, and inv is 1-t.
  template<typename T, typename S>
  inline Vec2<T>
  lerpUnclamped(const Vec2<T>& v0,
                const Vec2<T>& v1,
                S              t,
                S              invt)
  {
    const T tT    = T(t);
    const T invtT = T(invt);
    return Vec2<T>(v0.x * invtT + v1.x * tT, v0.y * invtT + v1.y * tT);
  }

  // Interpolate between three Vec2s at barycentric coord st.
  template<typename T, typename S>
  inline Vec2<T>
  interpolateAtBaryCoord(const Vec2<T>& v0,
                         const Vec2<T>& v1,
                         const Vec2<T>& v2,
                         const Vec2<S>& st)
  {
    return Vec2<T>(v0 + ((v1 - v0) * T(st.x)) + ((v2 - v0) * T(st.y)));
  }

  // Interpolate between three Vec2s at barycentric coord st, with derivatives.
  template<typename T, typename S>
  inline void
  interpolateAtBaryCoord(const Vec2<T>& v0,
                         const Vec2<T>& v1,
                         const Vec2<T>& v2,
                         const Vec2<S>& st,
                         const Vec2<S>& stdx,
                         const Vec2<S>& stdy,
                         Vec2<T>&       out,
                         Vec2<T>&       duout,
                         Vec2<T>&       dvout)
  {
    const Vec2<T> e01 = (v1 - v0);
    const Vec2<T> e02 = (v2 - v0);
    const Vec2<T> dt = (e01 * T(st.x)) + (e02 * T(st.y));
    out   = v0 + dt;
    duout = (e01 * T(stdx.x)) + (e02 * T(stdx.y)) - dt;
    dvout = (e01 * T(stdy.x)) + (e02 * T(stdy.y)) - dt;
  }

  template<typename T>
  inline void
  Vec2<T>::roundIfNearlyZero(T threshold)
  {
    if (isNearlyZero(x, threshold)) {
      x = static_cast<T>(0);
    }

    if (isNearlyZero(y, threshold)) {
      y = static_cast<T>(0);
    }
  }

  template<typename T>
  inline void
  Vec2<T>::roundIfNearlyOne(T threshold)
  {
    if (isNearlyOne(x, threshold)) {
      x = static_cast<T>(1);
    }

    if (isNearlyOne(y, threshold)) {
      y = static_cast<T>(1);
    }
  }

}

#endif
