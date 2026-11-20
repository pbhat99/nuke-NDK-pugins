// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FDK_MATH_BOX3_H
#define FDK_MATH_BOX3_H

#include "fdk/math/Vec3.h"
#include "fdk/math/Vec4.h"
#include "fdk/math/Box2.h"

namespace fdk {

  template <typename T> class Mat4;


  //------------------------------------------------------------------------------


  //! \class fdk::Box3
  //! Axis-aligned 3D bounding-box (AABB)
  template <typename T>
  class Box3
  {
  public:
    static constexpr uint8_t kNumElements = 6; //< The number of basic elements of type T

    //! XYZ corner identifiers.
    enum class Corner : int32_t {
      MinMinMin,
      MaxMinMin,
      MaxMaxMin,
      MinMaxMin,
      MinMinMax,
      MaxMinMax,
      MaxMaxMax,
      MinMaxMax,
      NumCorners
    };

    static const char* cornerName(Corner corner);

    //! Face identifiers.
    enum class Face : int32_t {
      Front,
      Back,
      Bottom,
      Top,
      Left,
      Right,
      NumFaces
    };

    static const char* faceName(Face face);


  public:
    // These are public for ease of access.
    Vec3<T>  min;       //!< "Lower-left-near"
    Vec3<T>  max;       //!< "Upper-right-far"


    /*---------------------------------------------------------*/
    /*                      Constructors                       */
    /*---------------------------------------------------------*/
    //! Default ctor makes an empty-state bbox where min=<T>max() & max=-<T>max()
    Box3();

    //! Copy constructor supports conversion from another type.
    template<typename S>
    Box3(const Box3<S>& b) : min(b.min), max(b.max) {}

    Box3(const T array[6]) { std::copy(array, array + 6, &min.x); }

    Box3(T x, T y, T z,
         T r, T t, T f) : min(x, y, z), max(r, t, f) {}

    template<typename S>
    Box3(const Vec3<S>& _min,
         const Vec3<S>& _max) : min(_min), max(_max) {}

    // Construct from a single point.
    Box3(T x, T y, T z) : min(x, y, z), max(x, y, z) {}
    explicit Box3(const Vec3<T>& p) : min(p), max(p) {}

    //! The resulting Box3 is the intersection of all the source points.
    template<typename S>
    explicit Box3(const Vec3<S>* points,
                  size_t         nPoints) { set(points, nPoints); }

    //! The resulting Box3 is the intersection of all the source points transformed by 'xform'.
    template<typename S>
    explicit Box3(const Vec3<S>* points,
                  size_t         nPoints,
                  const Mat4<T>& xform) { set(points, nPoints, xform); }

    //! Transmogrify as a specific type:
    operator Box3<fdk::half>() const;
    operator Box3<float>()     const;
    operator Box3<double>()    const;
    operator Box3<int32_t>()   const;

    Box3<fdk::half> asBox3h() const;
    Box3<float>     asBox3f() const;
    Box3<double>    asBox3d() const;
    Box3<int>       asBox3i() const;


    /*---------------------------------------------------------*/
    /*                        Assignment                       */
    /*---------------------------------------------------------*/
    void      set(T x, T y, T z,
                  T r, T t, T f);

    template<typename S>
    void      set(const Vec3<S>& _min,
                  const Vec3<S>& _max);

    void      set(const T array[6])    { std::copy(array, array + 6, &min.x); }

    //!
    void      set(T x, T y, T z);

    template<typename S>
    void      set(const Vec3<S>& v);

    //!
    void      set(const Box3<T>& b) { *this = b; }

    //! The resulting Box3 is the intersection of all the source points.
    template<typename S>
    void      set(const Vec3<S>* points,
                  size_t         nPoints);

    //! The resulting Box3 is the intersection of all the source points transformed by 'xform'.
    template<typename S>
    void      set(const Vec3<S>* points,
                  size_t         nPoints,
                  const Mat4<T>& xform);

    //! Set all components to 0 or 1.
    void      setToZero() { std::fill(&min.x, (&min.x) + 6, static_cast<T>(0)); }
    void      setToOne();

    void      setMin(const Vec3<T>& v) { min = v; }
    void      setMin(T x, T y, T z)    { min.set(x, y, z); }
    void      setMax(const Vec3<T>& v) { max = v; }
    void      setMax(T x, T y, T z)    { max.set(x, y, z); }

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
    bool operator == (const Box3& b) const { return (min == b.min && max == b.max); }

    //! Returns true if any of the min and max values are different. Compares on each element individually.
    bool operator != (const Box3& b) const { return (min != b.min || max != b.max); }


    /*---------------------------------------------------------*/
    /*                   Matrix Multiplication                 */
    /*---------------------------------------------------------*/
    template<typename S>
    Box3  operator *  (const Mat4<S>& m) const;

    template<typename S>
    Box3& operator *= (const Mat4<S>& m);


    /*---------------------------------------------------------*/
    /*                     Component Access                    */
    /*---------------------------------------------------------*/
    //! Return a pointer to min.x
    T*        array()       { return &min.x; }
    const T*  array() const { return &min.x; }

    T         x() const { return min.x; }                                //!< left   - min X
    T         y() const { return min.y; }                                //!< bottom - min Y
    T         z() const { return min.z; }                                //!< near   - min Z
    T         r() const { return max.x; }                                //!< right  - max X
    T         t() const { return max.y; }                                //!< top    - max Y
    T         f() const { return max.z; }                                //!< far    - max Z
    T         w() const { return (max.x - min.x); }                      //!< width
    T         h() const { return (max.y - min.y); }                      //!< height
    T         d() const { return (max.z - min.z); }                      //!< depth

    T         cx() const { return (min.x + max.x) / static_cast<T>(2); } //!< center X
    T         cy() const { return (min.y + max.y) / static_cast<T>(2); } //!< center Y
    T         cz() const { return (min.z + max.z) / static_cast<T>(2); } //!< center Z

    //! Return a Vec3 with width/height/depth in it.
    Vec3<T>   dimensions() const { return Vec3<T>(w(), h(), d()); }

    //! Return the xyz coordinate of the bbox center.
    Vec3<T>   center() const { return (min + max) / static_cast<T>(2); }

    //! Return the xyz coordinate of one of the corners.
    Vec3<T>   corner(Corner corner_index) const;

    T         radius() const { return (max - min).length() / static_cast<T>(2); }
    T         minDim() const { return std::min(min.minimumComponent(), max.minimumComponent()); }
    T         maxDim() const { return std::max(min.maximumComponent(), max.maximumComponent()); }

    //! Returns true if point is inside the box.
    template<typename S>
    bool      pointIsInside(const Vec3<S>& p) const;
    bool      pointIsInside(T x, T y, T z) const;

    //! Expand or contract the box by a set amount.
    void      pad(T d);
    void      pad(const Vec3<T>& p);
    void      pad(T x, T y, T z)    { pad(Vec3<T>(x, y, z)); }

    //! Shift the position of the box.
    void      shift(const Vec3<T>& p);
    void      shift(T x, T y, T z)    { shift(Vec3<T>(x, y, z)); }
    void      shiftMin(T x, T y, T z) { min += Vec3<T>(x, y, z); }
    void      shiftMax(T x, T y, T z) { max += Vec3<T>(x, y, z); }

    // TODO: these currently shift, is it more natural for them to do pad...?
    Box3<T>  operator +  (const Vec3<T>& v) const { return Box3<T>(min + v, max + v); }
    Box3<T>& operator += (const Vec3<T>& v);

    Box3<T>  operator -  (const Vec3<T>& v) const { return Box3<T>(min - v, max - v); }
    Box3<T>& operator -= (const Vec3<T>& v);

    //! Union the box with another. If this one is empty the other (non-empty) box is copied.
    template<typename S>
    void      expand(const Box3<S>& b,
                     bool           test_empty = true);

    //! Expand the box to contain a point. If empty the box is set to a zero-size at the point.
    template<typename S>
    void      expand(const Vec3<S>& p,
                     bool           test_empty = true);

    void      expand(T x, T y, T z,
                     bool test_empty = true) { expand(Vec3<T>(x, y, z), test_empty); }

    //! Find the union between the bbox and another.
    Box3<T>   intersect(const Box3<T>& v) const;


    /*! Project the corners of the bbox by a 4x4 matrix onto a 2D plane.
        If all corners project behind the origin return false and an empty box.
    */
    template<typename S>
    bool            project(const Mat4<T>& m,
                            Box2<S>&       box2D_out) const;

    /*! Returns 1 if point projects behind origin, ie an 'outside corner'.
        Expands the passed in Box2.
    */
    template<typename S>
    static uint32_t projectCorner(T x, T y, T z,
                                  const Mat4<T>& m,
                                  Box2<S>&       box2D_out);

    /*! Returns 1 if point projects behind origin, ie an 'outside corner'.
        Expands the passed in Box2.
    */
    template<typename S>
    uint32_t        projectCorner(Corner         corner_index,
                                  const Mat4<T>& m,
                                  Box2<S>&       box2D_out) const;


    //! Interpolate between this Box3 and another at t, where t=0..1.
    template<typename S>
    Box3<T>   interpolateTo(const Box3<T>& b,
                          S              t) const;
    template<typename S>
    Box3<T>   lerpClampedTo(const Box3<T>& b,
                     S              t) const;


    //! Return true if the box intersects with \a box
    bool intersects(const Box3<T>& box) const;

    //! Return true if 'a point is inside the box
    bool intersects(const Vec3<T>& point) const;

    //! Return the surface area of the box
    T surface() const;

    //! Return the longest axis
    unsigned int majorAxis() const;

    //! Split the box along \a axis, keeping the first or second half according to \a firstHalf
    void split(unsigned int axis, bool firstHalf);

    //! Transform each corner with m, and set this box to the axis-aligned bounds of the new corners
    inline void transform(const Mat4<T>& m);

  private:
    // This must match for Vec/Mat/Box classes:
    static_assert(fdk::is_arithmetic<T>, "Box3 can only be instantiated with an arithmetic type.");

  };


  /*--------------------------------------*/
  /*    Aliases for supported data types  */
  /*--------------------------------------*/
  using Box3i = Box3<int32_t>;
  using Box3f = Box3<float>;
  using Box3d = Box3<double>;
  using Box3h = Box3<fdk::half>;



  /*----------------------------------*/
  /*        Static operations         */
  /*----------------------------------*/

  //! Print out Box3 components to a stream.
  template<typename T>
  std::ostream& operator << (std::ostream& o, const Box3<T>& b);


  //! Build untransformed and transformed bboxes in one pass.
  template<typename T, typename S>
  void getLocalAndTransformedBbox(const Vec3<T>* points,
                                  size_t         nPoints,
                                  Box3<T>&       local_bbox_out,
                                  const Mat4<S>& xform,
                                  Box3<S>&       xformed_bbox_out);

  //! Linear-interpolate between two Box3s at t, where t=0..1.
  template<typename T, typename S>
  Box3<T> lerpClamped(const Box3<T>& v0,
               const Box3<T>& v1,
               S              t);
  //! Linear-interpolate between two Box3s at t, where t=0..1, and inv is 1-t.
  template<typename T, typename S>
  Box3<T> lerpClamped(const Box3<T>& v0,
               const Box3<T>& v1,
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
  inline std::ostream& operator << (std::ostream& o, const Box3<T>& b)
  {
    o << '[' << b.min.x << ' ' << b.min.y << ' ' << b.min.z;
    o << ' ' << b.max.x << ' ' << b.max.y << ' ' << b.max.z << ']';
    return o;
  }

  //-----------------------------------------------------------

  template<typename T>
  inline
  Box3<T>::Box3()
  : min( std::numeric_limits<T>::max())
  , max(-std::numeric_limits<T>::max())
  {
  }

  template<typename T>
  inline void
  Box3<T>::setToEmptyState()
  {
    min.set( std::numeric_limits<T>::max());
    max.set(-std::numeric_limits<T>::max());
  }

  template<typename T>
  inline void
  Box3<T>::setToOne()
  {
    min.x = min.y = min.z = static_cast<T>(1);
    max.x = max.y = max.z = static_cast<T>(1);
  }

  //-----------------------------------------------------------

  template<typename T>
  inline Box3<fdk::half>
  Box3<T>::asBox3h() const
  {
    if (std::is_same<fdk::half, T>::value) {
      return *this;
    }
    else {
      return Box3<float>(fdk::half(static_cast<float>(min.x)), fdk::half(static_cast<float>(min.y)), fdk::half(static_cast<float>(min.z)),
                         fdk::half(static_cast<float>(max.x)), fdk::half(static_cast<float>(max.y)), fdk::half(static_cast<float>(max.z)));
    }
  }

  template<typename T>
  inline Box3<T>::operator Box3<fdk::half>() const { return asBox3h(); }

  template<typename T>
  inline Box3<float>
  Box3<T>::asBox3f() const
  {
    if (std::is_same<float, T>::value) {
      return *this;
    }
    else {
      return Box3<float>(static_cast<float>(min.x), static_cast<float>(min.y), static_cast<float>(min.z),
                         static_cast<float>(max.x), static_cast<float>(max.y), static_cast<float>(max.z));
    }
  }
  template<typename T>
  inline Box3<T>::operator Box3<float>() const { return asBox3f(); }

  template<typename T>
  inline Box3<double>
  Box3<T>::asBox3d() const
  {
    if (std::is_same<double, T>::value) {
      return *this;
    }
    else {
      return Box3<double>(static_cast<double>(min.x), static_cast<double>(min.y), static_cast<double>(min.z),
                          static_cast<double>(max.x), static_cast<double>(max.y), static_cast<double>(max.z));
    }
  }
  template<typename T>
  inline Box3<T>::operator Box3<double>() const { return asBox3d(); }

  template<typename T>
  inline Box3<int32_t>
  Box3<T>::asBox3i() const
  {
    if (std::is_same<int32_t, T>::value) {
      return *this;
    }
    else {
      return Box3<int32_t>(static_cast<int32_t>(min.x), static_cast<int32_t>(min.y), static_cast<int32_t>(min.z),
                           static_cast<int32_t>(max.x), static_cast<int32_t>(max.y), static_cast<int32_t>(max.z));
    }
  }

  template<typename T>
  inline Box3<T>::operator Box3<int32_t>() const { return asBox3i(); }

  //-----------------------------------------------------------

  template<typename T>
  inline void
  Box3<T>::set(T x, T y, T z,
               T r, T t, T f)
  {
    min.set(x, y, z);
    max.set(r, t, f);
  }

  template<typename T>
  template<typename S>
  inline void
  Box3<T>::set(const Vec3<S>& _min,
               const Vec3<S>& _max)
  {
    min = _min;
    max = _max;
  }

  template<typename T>
  inline void
  Box3<T>::set(T x, T y, T z)
  {
    min.set(x, y, z);
    max.set(x, y, z);
  }

  template<typename T>
  template<typename S>
  inline void
  Box3<T>::set(const Vec3<S>& v)
  {
    min = v;
    max = v;
  }

  template<typename T>
  inline void
  Box3<T>::pad(T d)
  {
    min -= d;
    max += d;
  }

  template<typename T>
  inline void
  Box3<T>::pad(const Vec3<T>& p)
  {
    min -= p;
    max += p;
  }

  template<typename T>
  inline void
  Box3<T>::shift(const Vec3<T>& p)
  {
    min += p;
    max += p;
  }

  template<typename T>
  inline Box3<T>&
  Box3<T>::operator += (const Vec3<T>& v)
  {
    min += v;
    max += v;
    return *this;
  }

  template<typename T>
  inline Box3<T>&
  Box3<T>::operator -= (const Vec3<T>& v)
  {
    min -= v;
    max -= v;
    return *this;
  }

  //-----------------------------------------------------------

  template<typename T>
  inline Vec3<T>
  Box3<T>::corner(Corner corner_index) const
  {
    switch (corner_index) {
    default:
    case Corner::MinMinMin: return min;
    case Corner::MaxMinMin: return Vec3<T>(max.x, min.y, min.z);
    case Corner::MaxMaxMin: return Vec3<T>(max.x, max.y, min.z);
    case Corner::MinMaxMin: return Vec3<T>(min.x, max.y, min.z);
    case Corner::MinMinMax: return Vec3<T>(min.x, min.y, max.z);
    case Corner::MaxMinMax: return Vec3<T>(max.x, min.y, max.z);
    case Corner::MaxMaxMax: return max;
    case Corner::MinMaxMax: return Vec3<T>(min.x, max.y, max.z);
    }
  }

  template<typename T>
  inline bool
  Box3<T>::isEmpty() const
  {
    return (max.x < min.x
            || max.y < min.y
            || max.z < min.z);
  }

  template<typename T>
  inline bool
  Box3<T>::pointIsInside(T x, T y, T z) const
  {
    const bool is_outside = (x < min.x || x > max.x
                             || y < min.y || y > max.y
                             || z < min.z || z > max.z);
    return !is_outside;
  }

  template<typename T>
  template<typename S>
  inline bool
  Box3<T>::pointIsInside(const Vec3<S>& p) const
  {
    return pointIsInside(p.x, p.y, p.z);
  }

  template<typename T>
  template<typename S>
  inline void
  Box3<T>::expand(const Vec3<S>& p,
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

        if (p.z < min.z) {
          min.z = p.z;
        }

        if (p.z > max.z) {
          max.z = p.z;
        }
      }
      else {
        const Vec3<T> pT(p);
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

        if (pT.z < min.z) {
          min.z = pT.z;
        }

        if (pT.z > max.z) {
          max.z = pT.z;
        }
      }
    }
  }

  template<typename T>
  template<typename S>
  inline void
  Box3<T>::expand(const Box3<S>& b,
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
  Box3<T>::set(const Vec3<S>* points,
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
  inline Box3<T>
  Box3<T>::intersect(const Box3<T>& b) const
  {
    return Box3<T>(Vec3<T>(std::max(min.x, b.min.x),
                           std::max(min.y, b.min.y),
                           std::max(min.z, b.min.z)),
                   Vec3<T>(std::min(max.x, b.max.x),
                           std::min(max.y, b.max.y),
                           std::min(max.z, b.max.z)));
  }

  //-----------------------------------------------------------

  //! Interpolate between two bounding-boxes.
  template<typename T>
  template<typename S>
  inline Box3<T>
  Box3<T>::interpolateTo(const Box3<T>& b,
                         S              t) const
  {
    if (t < std::numeric_limits<T>::epsilon()) {
      return *this; // before or at first
    }
    else if (t > (static_cast<T>(1) - std::numeric_limits<T>::epsilon())) {
      return b; // at or after last
    }

    const T tT    = static_cast<T>(t);
    const T invtT = static_cast<T>(1) - tT;
    return Box3<T>(b.min * invtT + b.min * tT,
                   b.max * invtT + b.max * tT);
  }

  template<typename T>
  template<typename S>
  inline Box3<T>
  Box3<T>::lerpClampedTo(const Box3<T>& b, S t) const
  {
    return interpolateTo(b, t);
  }

  template<typename T, typename S>
  inline Box3<T>
  lerpClamped(const Box3<T>& b0,
       const Box3<T>& b1,
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
    return Box3<T>(b0.min * invtT + b1.min * tT,
                   b0.max * invtT + b1.max * tT);
  }

  template<typename T, typename S>
  inline Box3<T>
  lerpClamped(const Box3<T>& b0,
       const Box3<T>& b1,
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
    return Box3<T>(b0.min*invtT + b1.min*tT,
                   b0.max*invtT + b1.max*tT);
  }


  }


  //------------------------------------------------------------------------------
  //------------------------------------------------------------------------------
  // Mat4 transform implementations
  //
  // Keep the Mat4 include at end of Box3.h so it doesn't conflict with other
  // implementations.
  //
  //------------------------------------------------------------------------------
  //------------------------------------------------------------------------------
  #include "Mat4.h"

  namespace fdk {

  //-----------------------------------------------------------

  template<typename T>
  template<typename S>
  inline void
  Box3<T>::set(const Vec3<S>* points,
               size_t         nPoints,
               const Mat4<T>& xform)
  {
    if (nPoints == 0) {
      setToEmptyState();
    }
    else if (xform.isIdentity()) {
      set(*points++);
      for (size_t i = 1; i < nPoints; ++i) {
        expand(*points++, false);
      }
    }
    else {
      set(xform.transform(*points++));
      for (size_t i = 1; i < nPoints; ++i) {
        expand(xform.transform(*points++), false);
      }
    }
  }

  //-----------------------------------------------------------

  template<typename T>
  template<typename S>
  inline Box3<T>
  Box3<T>::operator *  (const Mat4<S>& m) const
  {
    Box3<T> o;
    return m.transform(*this, o);
  }

  template<typename T>
  template<typename S>
  inline Box3<T>&
  Box3<T>::operator *= (const Mat4<S>& m)
  {
    Box3<T> o = *this;
    return m.transform(o, *this);
  }

  //-----------------------------------------------------------

  //! Returns 1 if xyz point projects behind origin, ie an 'outside corner'.
  template<typename T>
  template<typename S>
  inline /*static*/ uint32_t
  Box3<T>::projectCorner(T x, T y, T z,
                         const Mat4<T>& m,
                         Box2<S>&       box2D_out)
  {
    const Vec4<T> v = m.transform(Vec4<T>(x, y, z, static_cast<T>(1)));
    if (v.w <= static_cast<T>(0)) {
      return 1; // corner is behind or at origin
    }

    if (std::is_same<int32_t, S>::value) {
      // Round nicely if destination type is integer:
      box2D_out.expand(static_cast<int32_t>(floor((v.x/v.w) + static_cast<T>(0.5))),
                       static_cast<int32_t>(floor((v.y/v.w) + static_cast<T>(0.5))),
                       true);
    }
    else {
      box2D_out.expand(v.x/v.w, v.y/v.w, true);
    }

    return 0;
  }

  template<typename T>
  template<typename S>
  inline uint32_t
  Box3<T>::projectCorner(Corner         corner_index,
                         const Mat4<T>& m,
                         Box2<S>&       box2D_out) const
  {
    const Vec3<T> v = getCorner(corner_index);
    return projectCorner(v.x, v.y, v.z, m, box2D_out);
  }

  template<typename T>
  template<typename S>
  inline bool
  Box3<T>::project(const Mat4<T>& m,
                   Box2<S>&       box2D_out) const
  {
    box2D_out.setToEmptyState();
    uint32_t num_outside_corners = 0;
    num_outside_corners += projectCorner(min.x, min.y, min.z, m, box2D_out);
    num_outside_corners += projectCorner(max.x, min.y, min.z, m, box2D_out);
    num_outside_corners += projectCorner(min.x, max.y, min.z, m, box2D_out);
    num_outside_corners += projectCorner(max.x, max.y, min.z, m, box2D_out);
    num_outside_corners += projectCorner(min.x, min.y, max.z, m, box2D_out);
    num_outside_corners += projectCorner(max.x, min.y, max.z, m, box2D_out);
    num_outside_corners += projectCorner(min.x, max.y, max.z, m, box2D_out);
    num_outside_corners += projectCorner(max.x, max.y, max.z, m, box2D_out);
    if (num_outside_corners == (uint32_t)Corner::NumCorners) {
      box2D_out.setToEmptyState(); // all corners behind origin
      return false;
    }

    return true; // at least one corner in projection
  }

  template<typename T>
  inline bool Box3<T>::intersects(const Box3& box) const
  {
    for (unsigned int i = 0; i < 3; i++) {
      if (box.max[i] < min[i] || box.min[i] > max[i]) {
        return false;
      }
    }

    return true;
  }

  template<typename T>
  inline bool Box3<T>::intersects(const Vec3<T>& point) const
  {
    for (unsigned int i = 0; i < 3; i++) {
      if (point[i] < min[i] || point[i] > max[i]) {
        return false;
      }
    }
    return true;
  }

  template<typename T>
  inline T Box3<T>::surface() const
  {
    Vec3<T> d = max - min;
    return 2.0f * (d.x * d.y + d.x * d.z + d.y * d.z);
  }

  template<typename T>
  inline unsigned int Box3<T>::majorAxis() const
  {
    unsigned int major = 0;
    const Vec3<T> s = max - min;

    for (unsigned int i = 1; i < 3; i++) {
      if (s[i] > s[major]) {
        major = i;
      }
    }

    return major;
  }

  template<typename T>
  inline void Box3<T>::split(unsigned int axis, bool firstHalf)
  {
    T half = min[axis] + (max[axis] - min[axis]) * 0.5f;

    if ( firstHalf ) {
      max[axis] = half;
    }
    else {
      min[axis] = half;
    }
  }

  template<typename T>
  inline void Box3<T>::transform(const Mat4<T>& m)
  {
    fdk::Vec3<T> corners[8] = {
      fdk::Vec3<T>(min.x, min.y, min.z),
      fdk::Vec3<T>(min.x, min.y, max.z),
      fdk::Vec3<T>(min.x, max.y, min.z),
      fdk::Vec3<T>(min.x, max.y, max.z),
      fdk::Vec3<T>(max.x, min.y, min.z),
      fdk::Vec3<T>(max.x, min.y, max.z),
      fdk::Vec3<T>(max.x, max.y, min.z),
      fdk::Vec3<T>(max.x, max.y, max.z),
    };
    set(corners, 8, m);
  }

  //-----------------------------------------------------------

  template<typename T, typename S>
  inline void
  getLocalAndTransformedBbox(const Vec3<T>* points,
                             size_t         nPoints,
                             Box3<T>&       local_bbox_out,
                             const Mat4<S>& xform,
                             Box3<S>&       xformed_bbox_out)
  {
    // Calc both bboxes in one pass:
    if (nPoints == 0) {
      local_bbox_out.setToEmptyState();
      xformed_bbox_out.setToEmptyState();
    }
    else {
      if (xform.isIdentity()) {
        local_bbox_out.set(points, nPoints);
        xformed_bbox_out.set(local_bbox_out.min, local_bbox_out.max);
      }
      else {
        local_bbox_out.set(points[0]);
        xformed_bbox_out.set(xform.transform(points[0]));
        for (size_t i = 1; i < nPoints; ++i) {
          local_bbox_out.expand(points[i], false);
          xformed_bbox_out.expand(xform.transform(points[i]), false);
        }
      }
    }
  }

}


#endif
