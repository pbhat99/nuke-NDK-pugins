// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FDK_MATH_BOX2_H
#define FDK_MATH_BOX2_H

#include "fdk/math/Vec2.h"

#include <algorithm>

namespace fdk {

  //! \class fdk::Box2
  //! Axis-aligned 2D bounding-box (AABB)
  template <typename T>
  class Box2
  {
  public:
    static constexpr uint8_t kNumElements = 4; //< The number of basic elements of type T

    //! XY corner identifiers.
    enum class Corner : int32_t {
      MinMin,
      MaxMin,
      MaxMax,
      MinMax,
      NumCorners
    };

    static const char* cornerName(Corner corner);

    //! Edge identifiers.
    enum class Edge : int32_t {
      Bottom,
      Right,
      Top,
      Left,
      NumEdges
    };

    static const char* edgeName(Edge edge);


  public:
    // These are public for ease of access.
    Vec2<T> min;      //!< "Lower-left"
    Vec2<T> max;      //!< "Upper-right"


    /*---------------------------------------------------------*/
    /*                      Constructors                       */
    /*---------------------------------------------------------*/
    //! Default ctor makes an empty-state bbox where min=<T>max() & max=-<T>max()
    Box2();

    //! Copy constructor
    template<typename S>
    explicit Box2(const Box2<S>& b) : min(T(b.min.x), T(b.min.y)),
                                      max(T(b.max.x), T(b.max.y)) {}

    Box2(const T array[4]) { std::copy(array, array + 4, &min.x); }

    Box2(T x, T y) : min(x, y), max(x, y) {}

    Box2(T x, T y,
         T r, T t) : min(x, y), max(r, t) {}

    template<typename S>
    Box2(const Vec2<S>& _min,
         const Vec2<S>& _max) : min(_min), max(_max) {}

    explicit Box2(const Vec2<T>& v) : min(v), max(v) {}

    //! The resulting Box2 is the intersection of all the source points.
    explicit Box2(const Vec2<T>* points,
                  size_t         nPoints) { set(points, nPoints); }

    //! Transmogrify as a specific type:
    operator Box2<fdk::half>() const;
    operator Box2<float>()     const;
    operator Box2<double>()    const;
    operator Box2<int32_t>()   const;

    Box2<fdk::half> asBox2h() const;
    Box2<float>     asBox2f() const;
    Box2<double>    asBox2d() const;
    Box2<int>       asBox2i() const;


    /*---------------------------------------------------------*/
    /*                        Assignment                       */
    /*---------------------------------------------------------*/
    void      set(T x, T y,
                  T r, T t);

    template<typename S>
    void      set(const Vec2<S>& _min,
                  const Vec2<S>& _max);

    void      set(const T array[4])    { std::copy(array, array + 4, &min.x); }

    //!
    void      set(T x, T y);

    template<typename S>
    void      set(const Vec2<S>& v);

    //!
    void      set(const Box2<T>& b) { *this = b; }

    //! The resulting Box2 is the intersection of all the source points.
    template<typename S>
    void      set(const Vec2<S>* points,
                  size_t         nPoints);

    //! Set all components to 0
    void      setToZero() { std::fill(&min.x, (&min.x) + 4, static_cast<T>(0)); }

    //! Set all components to 1
    void      setToOne();

    void      setMin(const Vec2<T>& v) { min = v; }
    void      setMin(T x, T y)         { min.set(x, y); }
    void      setMax(const Vec2<T>& v) { max = v; }
    void      setMax(T x, T y)         { max.set(x, y); }

    //! Sets box to empty state where min=<T>max() & max=-<T>max().
    void      setToEmptyState();

    //! Return true if the box is in an empty state (min=<T>max() & max=-<T>max().)
    bool      isEmpty() const;

    //! Type-specific clear. Sets box to empty state where min=<T>max() & max=-<T>max().
    void      clear() { setToEmptyState(); }

    //! Add this to a fdk::Hash object.
    void      append(fdk::Hash& hash) const;


    /*---------------------------------------------------------*/
    /*                          Equality                       */
    /*---------------------------------------------------------*/
    //! Returns true if min and max are equal. Compares on each element individually.
    bool operator == (const Box2& b) const { return (min == b.min && max == b.max); }
    //! Returns true if any of the min and max values are different. Compares on each element individually.
    bool operator != (const Box2& b) const { return (min != b.min || max != b.max); }


    /*---------------------------------------------------------*/
    /*                   Matrix Multiplication                 */
    /*---------------------------------------------------------*/
    // TODO: implement this in Mat4 as a special-cased flattened matrix transform
    //template<typename S>
    //Box2  operator *  (const Mat4<S>& m) const; // (implemented inline in Mat4.h)
    //template<typename S>
    //Box2& operator *= (const Mat4<S>& m);       // (implemented inline in Mat4.h)


    /*---------------------------------------------------------*/
    /*                     Component Access                    */
    /*---------------------------------------------------------*/
    //! Return a pointer to min.x
    T*        array()       { return &min.x; }
    const T*  array() const { return &min.x; }

    T         x() const { return min.x; }                   //!< left   - min X
    T         y() const { return min.y; }                   //!< bottom - min Y
    T         r() const { return max.x; }                   //!< right  - max X
    T         t() const { return max.y; }                   //!< top    - max Y
    T         w() const { return (max.x - min.x); }         //!< width
    T         h() const { return (max.y - min.y); }         //!< height

    T         cx() const { return (min.x + max.x) / static_cast<T>(2); } //!< center X
    T         cy() const { return (min.y + max.y) / static_cast<T>(2); } //!< center Y

    //! Return a Vec2 with width/height in it.
    Vec2<T>   dimensions() const { return Vec2<T>(w(), h()); }

    //! Return the xy coordinate of the bbox center.
    Vec2<T>   center() const { return (min + max) / static_cast<T>(2); }

    //! Return the xy coordinate of one of the corners.
    Vec2<T>   corner(Corner corner_index) const;

    T         radius() const { return (max - min).length() / static_cast<T>(2); }
    T         minDim() const { return std::min(min.minimumComponent(), max.minimumComponent()); }
    T         maxDim() const { return std::max(min.maximumComponent(), max.maximumComponent()); }


    //! Returns true if point is inside the box.
    template<typename S>
    bool      pointIsInside(const Vec2<S>& p) const;
    bool      pointIsInside(T x, T y)         const;


    //! Expand or contract the box by a set amount.
    void      pad(T d);
    void      pad(const Vec2<T>& p);
    void      pad(T x, T y)         { pad(Vec2<T>(x, y)); }

    //! Shift (translate) the position of the box.
    void      shift(const Vec2<T>& p);
    void      shift(T x, T y)         { shift(Vec2<T>(x, y)); }
    void      shiftMin(T x, T y)      { min += Vec2<T>(x, y); }
    void      shiftMax(T x, T y)      { max += Vec2<T>(x, y); }

    // TODO: these currently shift, is it more natural for them to do pad...?
    Box2<T>  operator +  (const Vec2<T>& v) const { return Box2<T>(min + v, max + v); }
    Box2<T>& operator += (const Vec2<T>& v);

    Box2<T>  operator -  (const Vec2<T>& v) const { return Box2<T>(min - v, max - v); }
    Box2<T>& operator -= (const Vec2<T>& v);


    //! Union the box with another. If this one is empty the other (non-empty) box is copied.
    template<typename S>
    void      expand(const Box2<S>& b,
                     bool           test_empty = true);

    //! Expand the box to contain a point. If empty the box is set to a zero-size at the point.
    template<typename S>
    void      expand(const Vec2<S>& p,
                     bool           test_empty = true);

    void      expand(T x, T y,
                     bool test_empty = true) { expand(Vec2<T>(x, y), test_empty); }

    //! Find the union between the bbox and another.
    Box2<T> intersect(const Box2<T>& v) const;


    //! Interpolate between this Box2 and another at t, where t=0..1.
    template<typename S>
    Box2<T>   interpolateTo(const Box2<T>& b,
                            S              t) const;

    template<typename S>
    Box2<T>   lerpClampedTo(const Box2<T>& b,
                     S              t) const;


  private:
    // This must match for Vec/Mat/Box classes:
    static_assert(fdk::is_arithmetic<T>, "Box2 can only be instantiated with an arithmetic type.");

  };


  /*--------------------------------------*/
  /*    Aliases for supported data types  */
  /*--------------------------------------*/
  using Box2i = Box2<int32_t>;
  using Box2f = Box2<float>;
  using Box2d = Box2<double>;
  using Box2h = Box2<fdk::half>;



  /*----------------------------------*/
  /*        Static operations         */
  /*----------------------------------*/

  //! Print out Box2 components to a stream.
  template<typename T>
  std::ostream& operator << (std::ostream& o, const Box2<T>& b);



  //! Linear-interpolate between two Box2s at t, where t=0..1.
  template<typename T, typename S>
  Box2<T> lerpClamped(const Box2<T>& v0,
               const Box2<T>& v1,
               S              t);

  //! Linear-interpolate between two Box2s at t, where t=0..1, and inv is 1-t.
  template<typename T, typename S>
  Box2<T> lerpClamped(const Box2<T>& v0,
               const Box2<T>& v1,
               S              t,
               S              invt);



  /*---------------------------------------------------------------------*/
  /*---------------------------------------------------------------------*/
  /*                   Inline Function Implementations                   */
  /*---------------------------------------------------------------------*/
  /*---------------------------------------------------------------------*/

  // Note:
  //  Methods with multiple arg type-combos must be implemented inline so the
  //  compiler can resolve the combinations and avoid linking issues.


  template<typename T>
  inline std::ostream& operator << (std::ostream& o, const Box2<T>& b)
  {
    o << '[' << b.min.x << ' ' << b.min.y << ' ' << b.max.x << ' ' << b.max.y << ']';
    return o;
  }

  //-----------------------------------------------------------

  template<typename T>
  inline
  Box2<T>::Box2()
  : min( std::numeric_limits<T>::max())
  , max(-std::numeric_limits<T>::max())
  {
  }

  template<typename T>
  inline void
  Box2<T>::setToEmptyState()
  {
    min.set( std::numeric_limits<T>::max());
    max.set(-std::numeric_limits<T>::max());
  }

  template<typename T>
  inline void
  Box2<T>::setToOne()
  {
    min.x = min.y = static_cast<T>(1);
    max.x = max.y = static_cast<T>(1);
  }

  //-----------------------------------------------------------

  template<typename T>
  inline Box2<fdk::half>
  Box2<T>::asBox2h() const
  {
    if (std::is_same<fdk::half, T>::value) {
      return *this;
    }
    else {
      return Box2<fdk::half>(fdk::half(static_cast<float>(min.x)), fdk::half(static_cast<float>(min.y)),
                         fdk::half(static_cast<float>(max.x)), fdk::half(static_cast<float>(max.y)));
    }
  }

  template<typename T>
  inline Box2<T>::operator Box2<fdk::half>() const { return asBox2h(); }

  template<typename T>
  inline Box2<float>
  Box2<T>::asBox2f() const
  {
    if (std::is_same<float, T>::value) {
      return *this;
    }
    else {
      return Box2<float>(static_cast<float>(min.x), static_cast<float>(min.y),
                         static_cast<float>(max.x), static_cast<float>(max.y));
    }
  }

  template<typename T>
  inline Box2<T>::operator Box2<float>() const { return asBox2f(); }

  template<typename T>
  inline Box2<double>
  Box2<T>::asBox2d() const
  {
    if (std::is_same<double, T>::value) {
      return *this;
    }
    else {
      return Box2<double>(static_cast<double>(min.x), static_cast<double>(min.y),
                          static_cast<double>(max.x), static_cast<double>(max.y));
    }
  }

  template<typename T>
  inline Box2<T>::operator Box2<double>() const { return this->asBox2d(); }

  template<typename T>
  inline Box2<int32_t>
  Box2<T>::asBox2i() const
  {
    if (std::is_same<int32_t, T>::value) {
      return *this;
    }
    else {
      return Box2<int32_t>(static_cast<int32_t>(min.x), static_cast<int32_t>(min.y),
                           static_cast<int32_t>(max.x), static_cast<int32_t>(max.y));
    }
  }
  template<typename T>
  inline Box2<T>::operator Box2<int32_t>() const { return asBox2i(); }

  //-----------------------------------------------------------

  template<typename T>
  inline void
  Box2<T>::set(T x, T y,
               T r, T t)
  {
    min.set(x, y);
    max.set(r, t);
  }

  template<typename T>
  template<typename S>
  inline void
  Box2<T>::set(const Vec2<S>& _min,
               const Vec2<S>& _max)
  {
    min = _min;
    max = _max;
  }

  template<typename T>
  inline void
  Box2<T>::set(T x, T y)
  {
    min.set(x, y);
    max.set(x, y);
  }

  template<typename T>
  template<typename S>
  inline void
  Box2<T>::set(const Vec2<S>& v)
  {
    min = v;
    max = v;
  }

  template<typename T>
  inline void
  Box2<T>::pad(T d)
  {
    min -= d;
    max += d;
  }

  template<typename T>
  inline void
  Box2<T>::pad(const Vec2<T>& p)
  {
    min -= p;
    max += p;
  }

  template<typename T>
  inline void
  Box2<T>::shift(const Vec2<T>& p)
  {
    min += p;
    max += p;
  }

  template<typename T>
  inline Box2<T>&
  Box2<T>::operator += (const Vec2<T>& v)
  {
    min += v;
    max += v;
    return *this;
  }

  template<typename T>
  inline Box2<T>&
  Box2<T>::operator -= (const Vec2<T>& v)
  {
    min -= v;
    max -= v;
    return *this;
  }

  //-----------------------------------------------------------

  template<typename T>
  inline Vec2<T>
  Box2<T>::corner(Corner corner_index) const
  {
    switch (corner_index) {
    default:
    case Corner::MinMin: return min;
    case Corner::MaxMin: return Vec2<T>(max.x, min.y);
    case Corner::MaxMax: return max;
    case Corner::MinMax: return Vec2<T>(min.x, max.y);
    }
  }

  template<typename T>
  inline bool
  Box2<T>::isEmpty() const
  {
    return (max.x < min.x || max.y < min.y);
  }

  template<typename T>
  inline bool
  Box2<T>::pointIsInside(T x, T y) const
  {
    const bool is_outside = (x < min.x || x > max.x
                             || y < min.y || y > max.y);
    return !is_outside;
  }

  template<typename T>
  template<typename S>
  inline bool
  Box2<T>::pointIsInside(const Vec2<S>& p) const
  {
    return pointIsInside(p.x, p.y);
  }

  template<typename T>
  template<typename S>
  inline void
  Box2<T>::expand(const Vec2<S>& p,
                  bool           test_empty)
  {
    if (test_empty && isEmpty()) {
      min = max = p;
    }
    else {
      if (std::is_same<S, T>::value) {
        if (p.x < min.x) {
          min.x = p.x;
        }

        if (p.x > max.x) {
          max.x = p.x;
        }

        if (p.y < min.y) {
          min.y = p.y;
        }

        if (p.y > max.y) {
          max.y = p.y;
        }
      }
      else {
        const Vec2<T> pT(p);
        if (pT.x < min.x) {
          min.x = pT.x;
        }

        if (pT.x > max.x) {
          max.x = pT.x;
        }

        if (pT.y < min.y) {
          min.y = pT.y;
        }

        if (pT.y > max.y) {
          max.y = pT.y;
        }
      }
    }
  }

  template<typename T>
  template<typename S>
  inline void
  Box2<T>::expand(const Box2<S>& b,
                  bool           test_empty)
  {
    if (b.isEmpty()) {
      return; // source bbox is empty, do nothing
    }

    if (test_empty && isEmpty()) {
      *this = b;
    }
    else {
      expand(b.min, false);
      expand(b.max, false);
    }
  }

  template<typename T>
  template<typename S>
  inline void
  Box2<T>::set(const Vec2<S>* points,
               size_t         nPoints)
  {
    if (nPoints == 0) {
      setToEmptyState();
    }
    else {
      min = max = *points++;
      for (size_t i = 1; i < nPoints; ++i) {
        expand(*points++, false);
      }
    }
  }

  //-----------------------------------------------------------

  template<typename T>
  inline Box2<T>
  Box2<T>::intersect(const Box2<T>& b) const
  {
    return Box2<T>(Vec2<T>(std::max(min.x, b.min.x),
                           std::max(min.y, b.min.y)),
                   Vec2<T>(std::min(max.x, b.max.x),
                           std::min(max.y, b.max.y)));
  }

  //-----------------------------------------------------------

  //! Interpolate between two bounding-boxes.
  template<typename T>
  template<typename S>
  inline Box2<T>
  Box2<T>::interpolateTo(const Box2<T>& b,
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
    return Box2<T>(b.min * invtT + b.min * tT,
                   b.max * invtT + b.max * tT); // lerp
  }

  template<typename T>
  template<typename S>
  inline Box2<T>
  Box2<T>::lerpClampedTo(const Box2<T>& b, S t) const
  {
    return interpolateTo(b, t);
  }

  template<typename T, typename S>
  inline Box2<T>
  lerpClamped(const Box2<T>& b0,
       const Box2<T>& b1,
       S              t)
  {
    if (t < std::numeric_limits<S>::epsilon()) {
      return b0; // before or at first
    }
    else if (t > (static_cast<S>(1) - std::numeric_limits<S>::epsilon())) {
      return b1; // at or after last
    }

    const T tT    = static_cast<T>(t);
    const T invtT = static_cast<T>(1) - tT;
    return Box2<T>(b0.min * invtT + b1.min * tT,
                   b0.max * invtT + b1.max * tT);
  }

  template<typename T, typename S>
  inline Box2<T>
  lerpClamped(const Box2<T>& b0,
       const Box2<T>& b1,
       S              t,
       S              invt)
  {
    if (t < std::numeric_limits<S>::epsilon()) {
      return b0; // before or at first
    }
    else if (t > (static_cast<S>(1) - std::numeric_limits<S>::epsilon())) {
      return b1; // at or after last
    }

    const T tT    = static_cast<T>(t);
    const T invtT = static_cast<T>(invt);
    return Box2<T>(b0.min * invtT + b1.min * tT,
                   b0.max * invtT + b1.max * tT);
  }

}

#endif
