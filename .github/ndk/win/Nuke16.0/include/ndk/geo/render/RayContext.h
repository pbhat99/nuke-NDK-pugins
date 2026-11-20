// Copyright (c) 2023 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef NDK_GEO_RENDER_RAYCONTEXT_H
#define NDK_GEO_RENDER_RAYCONTEXT_H

#include "ndk/api.h"
#include "ndk/geo/render/SurfaceTraits.h"

#include "fdk/math/Vec3.h"
#include "fdk/math/Mat4.h"
#include "fdk/base/Time.h"

#include <vector>

namespace ndk {

  //! Hit point information about an intersected surface/object.
  //! If distance is <= 0.0 or inf then no intersection occurred.
  template<class T>
  struct NDK_API RayHitPoint
  {
    int32_t            hitIndex;        //!< Index of this hit in a list of intersections
    int32_t            sampleIndex;     //!< Index of the sample creating this intersection
    //
    fdk::Vec3<T>       P;               //!< Hit point location
    fdk::Vec3f         Ng;              //!< Geometric normal at hit point
    T                  distance;        //!< Distance to hit point - 0.0 (or inf) means a miss (no hit)
    float              u, v, w;         //!< Barycentric triangle uvw coordinates at intersection P
    fdk::Vec2f         duv;             //!< uv ray radius
    //
    int32_t            renderableIndex; //!< Index of intersected object
    uint64_t           primId;          //!< Unique Id from the hit geometry (to avoid self-intersections)
    OpacityTraitsMask  opacityTraits;   //!< Opacity/transmission trait flags from material
  };

  using RayHitPointf = RayHitPoint<float>;
  using RayHitPointd = RayHitPoint<double>;
  using RayHitPointfList = std::vector<RayHitPointf>;
  using RayHitPointdList = std::vector<RayHitPointd>;


  //----------------------------------------------------------------------


  /*! \class RayContext

      \a directionDx & \a directionDy differential direction vectors define a
      cone around the ray's direction vector.

      The difference in angle between direction and directionDx is the cone's
      horizontal radius, and the angle between direction and directionDy is
      the cone's vertical radius.

  */
  class NDK_API RayContext
  {
    public:
      using TypeMask = uint16_t;    //!< Used for ray type and object visibility tests
      using OriginMask = uint16_t;

      // These are public for ease of access:
      fdk::Vec3f      origin;           //!< Ray origin
      fdk::Vec3f      direction;        //!< Ray direction
      float           minT = 0.0f;      //!< The minimum distance for the ray
      float           maxT = 0.0f;      //!< The maximum distance for the ray
      fdk::Vec3f      directionDx;      //!< Ray spread X-direction offset.
      fdk::Vec3f      directionDy;      //!< Ray spread Y-direction offset.
      //
      fdk::TimeValue  time;             //!< The current time for the ray
      TypeMask        typeMask = 0;     //!< Type of ray
      OriginMask      originMask = 0;   //!< What type of 'space' is the ray origin starting in?
      int16_t         id = 0;           //!< Id of ray, usually the hit index

      //----------------------------------------------------------------------------
      // Ray type flags, defined as uints vs. enums so they can be OR'd & AND'd together:
      static constexpr TypeMask         undefinedRay() { return 0x0000; } //!< Undefined ray (error case)
      //
      static constexpr TypeMask            cameraRay() { return 0x0001; } //!< Ray emitted from a camera
      static constexpr TypeMask            shadowRay() { return 0x0002; } //!< Ray from surface to light (direct illum)
      static constexpr TypeMask        reflectionRay() { return 0x0004; } //!< Ray reflected off a surface
      static constexpr TypeMask        refractionRay() { return 0x0008; } //!< Ray refracted into a surface (possible scatter/volume start)
      static constexpr TypeMask  thinTransmissionRay() { return 0x0010; } //!< Ray passing through a partially-transparent surface with no refraction
      static constexpr TypeMask          allRayTypes() { return 0x00ff; } //!< All ray types
      //
      static constexpr TypeMask          diffusePath() { return 0x0100; } //!< Path is a diffuse absorption into a surface
      static constexpr TypeMask           glossyPath() { return 0x0200; } //!< Path is a glossy reflection off a surface
      static constexpr TypeMask         allPathTypes() { return 0x0f00; } //!< All path flags
      //
      static constexpr TypeMask    frontSideTestOnly() { return 0x1000; } //!< Ray only hits the 'frontside' of a surface
      static constexpr TypeMask     backSideTestOnly() { return 0x2000; } //!< Ray only hits the 'backside' of a surface
      static constexpr TypeMask         allSideTypes() { return 0x3000; } //!< All side flags
      static constexpr TypeMask       noPresenceTest() { return 0x4000; } //!< Ray does not test presence to determine solidity
      static constexpr TypeMask             allTests() { return 0xf000; } //!< All geometry test options
      //
      static constexpr TypeMask             allTypes() { return 0xffff; } //!< All type and path flags

      static TypeMask   TurnOn(TypeMask v, TypeMask flags) { return (v | flags); }
      static TypeMask  TurnOff(TypeMask v, TypeMask flags) { return (v & ~flags); }

      // Commonly-used ray type combos:
      static constexpr TypeMask  shadowTransmissionPath() { return (shadowRay() | thinTransmissionRay()); }
      static constexpr TypeMask   diffuseReflectionPath() { return (diffusePath() | reflectionRay()); }
      static constexpr TypeMask    glossyReflectionPath() { return (glossyPath() | reflectionRay()); }

      // TypeMask convenience methods:
      TypeMask           rayType() const { return (typeMask & allRayTypes()); }
      TypeMask          pathType() const { return (typeMask & allPathTypes()); }
      TypeMask         sidesType() const { return (typeMask & (frontSideTestOnly() | backSideTestOnly())); }
      TypeMask          testType() const { return (typeMask & allTests()); }
      //
      bool           isValidPath() const { return (rayType() !=        undefinedRay()); }
      bool          isCameraPath() const { return (rayType() ==           cameraRay()); }
      bool          isShadowPath() const { return (rayType() ==           shadowRay()); }
      bool       isReflectedPath() const { return (rayType() ==       reflectionRay()); }
      bool       isRefractedPath() const { return (rayType() ==       refractionRay()); }
      bool isThinTransmittedPath() const { return (rayType() == thinTransmissionRay()); }
      //
      bool  isDiffuseContributor() const { return (pathType() & diffusePath()); }
      bool   isGlossyContributor() const { return (pathType() &  glossyPath()); }

      bool     testFrontSideOnly() const { return (testType() & frontSideTestOnly()); }
      bool      testBackSideOnly() const { return (testType() & backSideTestOnly()); }
      bool  testPresenceDisabled() const { return (testType() & noPresenceTest()); }

      //----------------------------------------------------------------------------
      //! Ray origin spaces, defined as uints vs. enums so they can be OR'd & AND'd together:
      static constexpr OriginMask  undefinedOrigin() { return 0x0000; } //!< Undefined interface case (error case)
      //
      static constexpr OriginMask    outsideOrigin() { return 0x0001; } //!< Ray originating outside object
      static constexpr OriginMask     insideOrigin() { return 0x0002; } //!< Ray originating inside object
      //
      static constexpr OriginMask       allOrigins() { return 0xffff; }


    public:
      //! Default constructor leaves vars -intentionally- uninitialized for performance reasons.
      //! For example so that constructing a vector of RayContexts can be as fast as possible.
      RayContext() = default;

      RayContext(const fdk::Vec3f& _origin,
                 const fdk::Vec3f& _dir,
                 TypeMask          _type,
                 fdk::TimeValue    _time,
                 int16_t           _id,
                 float             _minDist,
                 float             _maxDist);

      //! Ray inverse direction vector (reciprocal)
      const fdk::Vec3f& invDir() const { return _inv_dir; }

      //! Get the point along ray direction at distance \a t.
      template <typename T>
      fdk::Vec3<T>  getPositionAt(T t) const;

      //! Sets the ray context and updates slopes.
      void  set(const fdk::Vec3f& _origin,
                const fdk::Vec3f& _dir,
                TypeMask          _type,
                fdk::TimeValue    _time,
                int16_t           _id,
                float             _minDist,
                float             _maxDist);

      //! Set the ray geometry values, leaving rest of parameters untouched.
      void  setGeom(const fdk::Vec3f& _origin,
                    const fdk::Vec3f& _dir,
                    float             _minDist,
                    float             _maxDist);

      //! Sets origin, direction vector and updates slopes.
      void  setOriginAndDirection(const fdk::Vec3f& _origin,
                                  const fdk::Vec3f& _dir);

      //! Sets direction vector and updates slopes.
      void  setDirection(const fdk::Vec3f& dir) { direction = dir; updateSlopes(); }

      //! Sets direction vector and distances and updates slopes.
      void  setDirAndDistance(const fdk::Vec3f& dir,
                              float             min,
                              float             max) { direction = dir; minT = min; maxT = max; updateSlopes(); }

      //! Initialize the differentials from two rotation angles (in radians).
      void  setXYDirsFromAngle(float  radian_rX,
                               float  radian_rY);


      //! Recalcs the slope test speedup vars - do this after any change to ray direction.
      void  updateSlopes();
      bool  isSlopePositive(int32_t i) const { return (_slope_positive & (0x01 << i)); }
      bool  isXSlopePositive() const { return (_slope_positive & 0x01); }
      bool  isYSlopePositive() const { return (_slope_positive & 0x02); }
      bool  isZSlopePositive() const { return (_slope_positive & 0x04); }

      //! Reverse the direction of the ray, which is just negation and update of slopes.
      void  invert();

      //! Transform the ray origin and direction in place by a matrix and updates slopes.
      void  transform(const fdk::Mat4d& xform);

      //! Transform the ray origin and direction in place by two matrices, interpolated at t, and updates slopes.
      void  transform(const fdk::Mat4d& m0,
                      const fdk::Mat4d& m1,
                      float             t);


    public:
      //! For internal use only (for constructing error rays.)
      //! Other vars left intentionally uninitialized for performance reasons.
      RayContext(int16_t i) : id(i) {}

    protected:
      fdk::Vec3f  _inv_dir;               //!< Direction reciprocal, for AABB intersection test speedups
      uint8_t     _slope_positive = 0;    //!< xyz positive slope indicator - true if slope is positive for an axis
      uint8_t     pad1 = 0;

  };

  using RayContextList         = std::vector<RayContext>;
  using RayContextPtrList      = std::vector<RayContext*>;
  using RayContextConstPtrList = std::vector<const RayContext*>;


  //----------------------------------------------------------------------


  struct RayContextBundle
  {
    RayContextBundle() : _list(nullptr), _start(0), _size(0) {}
    RayContextBundle(RayContextList* list,
                     int32_t         startIndex) : _list(list), _start(startIndex), _size(0) {}

    //! Is the bundle bound to valid data?
    bool     isValid() const { return (_list != nullptr); }

    int32_t  startIndex() const { return _start; }
    int32_t  endIndex() const { return (_start + _size); }
    int32_t  size() const { return _size; }

    bool     contains(int32_t rayIndex) const { return !(rayIndex < 0 || rayIndex >= _size); }
    int32_t  absoluteRayIndex(int32_t rayIndex) const { return _start + rayIndex; }

    //! Add another RayContext to the bundle, returning a reference to it and incrementing
    //! the current bundle count. This method assumes the bundle currently 'owns' the
    //! RayContextList by a single thread and is always appending to the end of the list.
    RayContext&  add(const RayContext& rtx = RayContext()); // copy
    RayContext&  add(RayContext&& rtx); // move

          RayContext&  operator [] (int i)       { return (*_list)[_start + i]; }
    const RayContext&  operator [] (int i) const { return (*_list)[_start + i]; }

    //! Returns the first RayContext in the bundle. Caution - if this bundle is empty the returned
    //! RayContext may be from a previous bundle!
    RayContext&        front() { return (*_list)[_start]; }
    //! Returns the last RayContext in the bundle. Caution - if this bundle is empty the returned
    //! RayContext may be from a previous bundle!
    RayContext&        back() { return _list->back(); }

    RayContext*        array() { return (_list->data() + _start); }
    const RayContext*  array() const { return (_list->data() + _start); }

    //! Iterator support
    typedef RayContextList::iterator       iterator;
    typedef RayContextList::const_iterator const_iterator;

    const_iterator  begin() const { return _list->begin() + _start; }
    const_iterator  end()   const { return _list->begin() + (_start + _size); }

    //! Sets size of bundle to 0 and resizes list to start of bundle (retaining contents of prior bundles.)
    void clear() { _list->resize(_start); _size = 0; }


  private:
    RayContextList* _list;          //!< The list of RayContexts this bundle is adding to
    int32_t         _start = 0;     //!< Absolute starting index cache list
    int32_t         _size = 0;      //!< Absolute inclusive-end index (loop is index=start; index < end)
  };

  using RayContextBundleList = std::vector<RayContextBundle>;



  /*----------------------------------*/
  /*        Static operations         */
  /*----------------------------------*/


  //! Print out components to a stream.
  template<typename T>
  std::ostream& operator << (std::ostream& o, const RayContext& r);


/*---------------------------------------------------------------------*/
/*---------------------------------------------------------------------*/
/*                   Inline Function Implementations                   */
/*---------------------------------------------------------------------*/
/*---------------------------------------------------------------------*/


template<class T>
inline std::ostream& operator << (std::ostream& o, const RayHitPoint<T>& hp)
{
  //o.setf(ios::fixed, ios::floatfield);
  //o.precision(7);
  o << "[";
  o << "idx[" << hp.hitIndex << "], sample[" << hp.sampleIndex << "]";
  o << ", P" << hp.P << ", Ng" << hp.Ng << ", t=" << hp.distance;
  o << " bry(" << hp.u << " " << hp.v << " " << hp.w << ")";
  o << ", renderable=" << hp.renderableIndex;
  o << ", primId=0x" << std::hex << hp.primId << std::dec;
  o << " ]";
  return o;
}

inline std::ostream& operator << (std::ostream& o, const RayContext& rtx)
{
  //o.setf(ios::fixed, ios::floatfield);
  //o.precision(7);
  o << rtx.origin << ",dir" << rtx.direction;
  o << "(" << rtx.minT << ".." << rtx.maxT << ")";
  o << ",dx" << rtx.directionDx << ",dy" << rtx.directionDy;
  o << ",type=0x" << std::hex << rtx.typeMask << std::dec;
  //o << rtx.invDir();
  //o << "/[" << rtx.isXSlopePositive() << " " << rtx.isYSlopePositive() << " " << rtx.isZSlopePositive() << "]";
  return o;
}

inline void
RayContext::updateSlopes()
{
  _inv_dir.set(1.0f / direction.x,
               1.0f / direction.y,
               1.0f / direction.z);
  _slope_positive = 0x00;
  if (_inv_dir.x < 0.0f) { _slope_positive |= 0x01; }
  if (_inv_dir.y < 0.0f) { _slope_positive |= 0x02; }
  if (_inv_dir.z < 0.0f) { _slope_positive |= 0x04; }
}

inline
RayContext::RayContext(const fdk::Vec3f& _origin,
                       const fdk::Vec3f& _dir,
                       TypeMask          _type,
                       fdk::TimeValue    _time,
                       int16_t           _id,
                       float             _minDist,
                       float             _maxDist) :
  origin(_origin),
  direction(_dir),
  minT(_minDist),
  maxT(_maxDist),
  directionDx(0.0f),
  directionDy(0.0f),
  time(_time),
  typeMask(_type),
  originMask(outsideOrigin()),
  id(_id)
{
  updateSlopes();
}

inline void
RayContext::set(const fdk::Vec3f& _origin,
                const fdk::Vec3f& _dir,
                TypeMask          _type,
                fdk::TimeValue    _time,
                int16_t           _id,
                float             _minDist,
                float             _maxDist)
{
  origin = _origin;
  direction = _dir;
  minT = _minDist;
  maxT = _maxDist;
  directionDx.set(0.0f);
  directionDy.set(0.0f);
  time = _time;
  typeMask = _type;
  originMask = outsideOrigin();
  id = _id;
  updateSlopes();
}

inline void
RayContext::setGeom(const fdk::Vec3f& _origin,
                    const fdk::Vec3f& _dir,
                    float             _minDist,
                    float             _maxDist)
{
  origin = _origin;
  direction = _dir;
  minT = _minDist;
  maxT = _maxDist;
  updateSlopes();
}

inline void
RayContext::setOriginAndDirection(const fdk::Vec3f& _origin,
                                  const fdk::Vec3f& _dir)
{
  origin = _origin;
  direction = _dir;
  updateSlopes();
}

template <typename T>
inline fdk::Vec3<T>
RayContext::getPositionAt(T t) const
{
  return fdk::Vec3<T>(origin.x + (direction.x * t),
                      origin.y + (direction.y * t),
                      origin.z + (direction.z * t));
}

inline void
RayContext::invert()
{
  direction.negate();
  updateSlopes();
}

inline void
RayContext::transform(const fdk::Mat4d& xform)
{
  origin = xform.transform(origin);
  direction = xform.vecTransform(direction);
  updateSlopes();
}

// Transform by two matrices, interpolated at t.
inline void
RayContext::transform(const fdk::Mat4d& m0,
                      const fdk::Mat4d& m1,
                      float             t)
{
  origin = m0.transform(origin).interpolateTo(m1.transform(origin), t);
  direction = m0.vecTransform(direction).interpolateTo(m1.vecTransform(direction), t);
  updateSlopes();
}

inline void
RayContext::setXYDirsFromAngle(float radian_rX,
                               float radian_rY)
{
  radian_rX = std::fabs(radian_rX);
  if (radian_rX > std::numeric_limits<float>::epsilon()) {
    if (radian_rX > M_PI) {
      radian_rX = M_PI;
    }
    directionDx.set(std::sin(radian_rX), 0.0f, std::cos(radian_rX));
    directionDx.orientAroundNormal(direction);
  }
  else {
    directionDx = direction;
  }

  radian_rY = std::fabs(radian_rY);
  if (radian_rY > std::numeric_limits<float>::epsilon()) {
    if (radian_rY > M_PI) {
      radian_rY = M_PI;
    }
    directionDy.set(0.0f, std::sin(radian_rY), std::cos(radian_rY));
    directionDy.orientAroundNormal(direction);
  }
  else {
    directionDy = direction;
  }
}

//------------------------------------------------------------

inline RayContext&
RayContextBundle::add(const RayContext& rtx)
{
  _list->push_back(rtx);
  return _list->back();
}

inline RayContext&
RayContextBundle::add(RayContext&& rtx)
{
  _list->emplace_back(std::move(rtx));
  return _list->back();
}

} // namespace ndk

#endif
