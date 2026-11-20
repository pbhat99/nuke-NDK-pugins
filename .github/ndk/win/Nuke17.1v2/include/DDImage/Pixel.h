// Pixel.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_Pixel_h
#define DDImage_Pixel_h

#include "DDImage/ChannelSet.h"
#include "DDImage/ChannelPack.h"
#include "DDImage/Channel3D.h"
#include "fdk/math/Vec4.h"
#include <string.h>
#include <limits>

#if kDDImageVersionMajorNum <= 17
#include "DDImage/Deprecation.h"
#else
#error "Remove the above code block in Nuke 18 as they are deprecated in Nuke 14."
#endif

namespace DD
{
  namespace Image
  {

    class InterestRatchet;

    /*! \class Pixel
       Holds a single pixel's worth of image data.  Contains a ChannelSet
       to indicate active planes.
       \a _interestRatchet is only useful when the Pixel is being passed to Iop
       Tile sampling methods as it helps speed up repeated Tile queries.
     */
    class DDImage_API Pixel
    {
    public:
      ChannelSet channels;
      float chan[Chan_Last + 1]; // Needs one extra as chan 0 is the black channel

      //! Default ctor leaves ChannelSet empty (set to Mask_None.)
      Pixel() { chan[0] = 0.0f; }
      Pixel(const Pixel &b) : channels(b.channels) { *this = b; }

      Pixel(const ChannelSetInit c) : channels(c) { chan[0] = 0.0f; }
      Pixel(const ChannelSet& c) : channels(c) { chan[0] = 0.0f; }
      Pixel(Channel c) : channels(c) { chan[0] = 0.0f; }

      //-----------------------------------------------------

      float&       operator [] (Channel z)       { return chan[z]; }
      const float& operator [] (Channel z) const { return chan[z]; }
      float* array() { return chan; }
      const float* array() const { return chan; }

      //-----------------------------------------------------

      //! Set this when Pixel is being passed to Iop Tile sampling methods so that repeated queries of
      //! the same Tile are accelerated.
      //! It is not useful or needed for general Pixel usage.
      InterestRatchet*  interestRatchet() { return _interestRatchet; }
      void              setInterestRatchet(InterestRatchet* interestRatchet) { _interestRatchet = interestRatchet; }
      void              copyInterestRatchet(const Pixel& from) { _interestRatchet = from._interestRatchet; }

      //-----------------------------------------------------

      void set(float d)                       { for (auto z : channels) { chan[z] = d; } }
      void set(const ChannelSet& c, float d)  { channels = c; set(d); }
      void set(const ChannelPack& c, float d) { for (auto z : c) { chan[z] = d; channels = c; } }

      void erase()                        { memset(chan + 1, 0, Chan_Last * sizeof(float)); }
      void erase(Channel z)               { chan[z] = 0.0f; }
      void erase(const ChannelSet& mask)  { for (auto z : mask) { chan[z] = 0.0f; } }
      void erase(const ChannelPack& mask) { for (auto z : mask) { chan[z] = 0.0f; } }

      void copy(const float* array, const ChannelSet& c)  { for (auto z : c) { chan[z] = array[z]; } }
      void copy(const float* array, const ChannelPack& c) { for (auto z : c) { chan[z] = array[z]; } }
      void copy(const Pixel& b, const ChannelSet& c)  { for (auto z : c) { chan[z] = b.chan[z]; } }
      void copy(const Pixel& b, const ChannelPack& c) { for (auto z : c) { chan[z] = b.chan[z]; } }

      void copyWeighted(const float* array, const ChannelSet& c, float w)  { for (auto z : c) { chan[z] = array[z] * w; } }
      void copyWeighted(const float* array, const ChannelPack& c, float w) { for (auto z : c) { chan[z] = array[z] * w; } }
      void copyWeighted(const Pixel& b, const ChannelSet& c, float w)  { for (auto z : c) { chan[z] = b.chan[z] * w; } }
      void copyWeighted(const Pixel& b, const ChannelPack& c, float w) { for (auto z : c) { chan[z] = b.chan[z] * w; } }

      void replace(const Pixel& b, const ChannelSet& c)  { for (auto z : c) { chan[z] = b.chan[z]; } }
      void replace(const Pixel& b, const ChannelPack& c) { for (auto z : c) { chan[z] = b.chan[z]; } }
      void replace(const Pixel& b)                       { for (auto z : b.channels) { chan[z] = b.chan[z]; } }

      //! Return a Pixel filled with \getChans channels from this one.
      Pixel get(const ChannelSet& getChans) const;
      Pixel get(const ChannelPack& getChans) const;

      //-----------------------------------------------------

      // Assignment operators:
      Pixel& operator =  (float d) { for (auto z : channels) { chan[z] = d; } return *this; }

      // Multiply operators:
      Pixel  operator *  (float d) const;
      Pixel& operator *= (float d);
      Pixel  operator *  (const Pixel& v) const;
      Pixel& operator *= (const Pixel& v);

      // Divide operators:
      Pixel  operator /  (float d) const;
      Pixel& operator /= (float d);
      Pixel  operator /  (const Pixel& v) const;
      Pixel& operator /= (const Pixel& v);

      // Addition operators:
      Pixel  operator +  (float d) const;
      Pixel& operator += (float d);
      Pixel  operator +  (const Pixel& v) const;
      Pixel& operator += (const Pixel& v);
      // Subtraction operators:
      Pixel  operator -  (float d) const;
      Pixel& operator -= (float d);
      Pixel  operator -  (const Pixel& v) const;
      Pixel& operator -= (const Pixel& v);

      //-----------------------------------------------------

      //! Destructive linear interpolate between this Pixel and Pixel \a b by \a t(0..1).
      void lerp(const Pixel& b, float t);
      void lerp(const Pixel& b, float t, Channel z);
      void lerp(const Pixel& b, float t, const ChannelSet& channels);
      void lerp(const Pixel& b, float t, const ChannelPack& channels);

      void over(const Pixel& A, float a); //!< Only overs RGBA channels
      void over(const Pixel& A, float a, Channel z);
      void over(const Pixel& A, float a, const ChannelSet& channels);
      void over(const Pixel& A, float a, const ChannelPack& channels);

      // Unders using this Pixel's alpha:
      void under(const Pixel& A); //!< Only overs RGBA channels
      void under(const Pixel& A, Channel z);
      void under(const Pixel& A, const ChannelSet& channels);
      void under(const Pixel& A, const ChannelPack& channels);

      // Unders using a provided B alpha:
      void under(float Ba, const Pixel& A); //!< Only overs RGBA channels
      void under(float Ba, const Pixel& A, Channel z);
      void under(float Ba, const Pixel& A, const ChannelSet& channels);
      void under(float Ba, const Pixel& A, const ChannelPack& channels);

      // Multiply each channel using a separate ChannelSet or ChannelPack:
      void multiply(float d, const ChannelSet& channels);
      void multiply(float d, const ChannelPack& channels);
      void multiply(const Pixel& b, const ChannelSet& channels);
      void multiply(const Pixel& b, const ChannelPack& channels);
      // Convenience for remapped-channel combos:
      void multiplyVec2Chans(float x, float y, Channel* clist);
      void multiplyVec2Chans(const fdk::Vec2f& v, Channel* clist) { multiplyVec2Chans(v.x, v.y, clist); }
      void multiplyVec3Chans(float x, float y, float z, Channel* clist);
      void multiplyVec3Chans(const fdk::Vec3f& v, Channel* clist) { multiplyVec3Chans(v.x, v.y, v.z, clist); }
      void multiplyVec4Chans(float x, float y, float z, float w, Channel* clist);
      void multiplyVec4Chans(const fdk::Vec4f& v, Channel* clist) { multiplyVec4Chans(v.x, v.y, v.z, v.w, clist); }

      // Divide each channel using a separate ChannelSet or ChannelPack:
      void divide(float d, const ChannelSet& channels);
      void divide(float d, const ChannelPack& channels);
      void divide(const Pixel& b, const ChannelSet& channels);
      void divide(const Pixel& b, const ChannelPack& channels);
      // Convenience for remapped-channel combos:
      void divideVec2Chans(float x, float y, Channel* clist);
      void divideVec2Chans(const fdk::Vec2f& v, Channel* clist) { divideVec2Chans(v.x, v.y, clist); }
      void divideVec3Chans(float x, float y, float z, Channel* clist);
      void divideVec3Chans(const fdk::Vec3f& v, Channel* clist) { divideVec3Chans(v.x, v.y, v.z, clist); }
      void divideVec4Chans(float x, float y, float z, float w, Channel* clist);
      void divideVec4Chans(const fdk::Vec4f& v, Channel* clist) { divideVec4Chans(v.x, v.y, v.z, v.w, clist); }

      // Plus each channel using a separate ChannelSet or ChannelPack:
      void plus(float d, const ChannelSet& channels);
      void plus(float d, const ChannelPack& channels);
      void plus(const Pixel& b, const ChannelSet& channels);
      void plus(const Pixel& b, const ChannelPack& channels);
      void plusWeighted(const float* array, const ChannelSet& channels, float w);
      void plusWeighted(const float* array, const ChannelPack& channels, float w);
      void plusWeighted(const Pixel& b, const ChannelSet& channels, float w);
      void plusWeighted(const Pixel& b, const ChannelPack& channels, float w);
      // Convenience for remapped-channel combos:
      void plusVec2Chans(float x, float y, Channel* clist);
      void plusVec2Chans(const fdk::Vec2f& v, Channel* clist) { plusVec2Chans(v.x, v.y, clist); }
      void plusVec3Chans(float x, float y, float z, Channel* clist);
      void plusVec3Chans(const fdk::Vec3f& v, Channel* clist) { plusVec3Chans(v.x, v.y, v.z, clist); }
      void plusVec4Chans(float x, float y, float z, float w, Channel* clist);
      void plusVec4Chans(const fdk::Vec4f& v, Channel* clist) { plusVec4Chans(v.x, v.y, v.z, v.w, clist); }

      // Minus each channel using a separate ChannelSet or ChannelPack:
      void minus(float d, const ChannelSet& channels);
      void minus(float d, const ChannelPack& channels);
      void minus(const Pixel& b, const ChannelSet& channels);
      void minus(const Pixel& b, const ChannelPack& channels);
      // Convenience for remapped-channel combos:
      void minusVec2Chans(float x, float y, Channel* clist);
      void minusVec2Chans(const fdk::Vec2f& v, Channel* clist) { minusVec2Chans(v.x, v.y, clist); }
      void minusVec3Chans(float x, float y, float z, Channel* clist);
      void minusVec3Chans(const fdk::Vec3f& v, Channel* clist) { minusVec3Chans(v.x, v.y, v.z, clist); }
      void minusVec4Chans(float x, float y, float z, float w, Channel* clist);
      void minusVec4Chans(const fdk::Vec4f& v, Channel* clist) { minusVec4Chans(v.x, v.y, v.z, v.w, clist); }

      //-----------------------------------------------------

      //! Convenience color functions
      void setRGBAToBlack() { rgb().set(0.0f); alpha() = 1.0f; }
      void setRGBAToWhite() { rgba().set(1.0f); }

      void setRGBToBlack()  { rgb().set(0.0f); }
      void setRGBToWhite()  { rgb().set(1.0f); }

      void setOpacityToFull() { opacity() = 1.0f; }
      void setOpacityToNone() { opacity() = 0.0f; }

      // Convenience for remapped-channel combo reads.
      fdk::Vec3f getVec3Chans(Channel* clist) const;
      fdk::Vec3f getVec3Chans(Channel x, Channel y, Channel z) const;

      fdk::Vec4f getVec4Chans(Channel* clist) const;
      fdk::Vec4f getVec4Chans(Channel x, Channel y, Channel z, Channel w) const;

      // Convenience for remapped-channel combo writes
      void setVec2Chans(float x, float y, Channel* clist);
      void setVec2Chans(const fdk::Vec2f& v, Channel* clist) { setVec2Chans(v.x, v.y, clist); }

      void setVec3Chans(float x, float y, float z, Channel* clist);
      void setVec3Chans(const fdk::Vec3f& v, Channel* clist) { setVec3Chans(v.x, v.y, v.z, clist); }

      void setVec4Chans(float x, float y, float z, float w, Channel* clist);
      void setVec4Chans(const fdk::Vec4f& v, Channel* clist) { setVec4Chans(v.x, v.y, v.z, v.w, clist); }

      //-----------------------------------------------------

      // Read/write vector attribute access convenience methods.
      fdk::Vec4f&        P()       { return reinterpret_cast<      fdk::Vec4f&>(chan[Chan_P_]); }
      const fdk::Vec4f&  P() const { return reinterpret_cast<const fdk::Vec4f&>(chan[Chan_P_]); }
      float&             x()       { return P().x; }
      float              x() const { return P().x; }
      float&             y()       { return P().y; }
      float              y() const { return P().y; }
      float&             z()       { return P().z; }
      float              z() const { return P().z; }
      float&             w()       { return P().w; }
      float              w() const { return P().w; }

      float&             Z()       { return chan[Chan_Z        ]; }
      float              Z() const { return chan[Chan_Z        ]; }
      float&            Zf()       { return chan[Chan_DeepFront]; }
      float             Zf() const { return chan[Chan_DeepFront]; }
      float&            Zb()       { return chan[Chan_DeepBack ]; }
      float             Zb() const { return chan[Chan_DeepBack ]; }

      fdk::Vec3f&           PL()       { return reinterpret_cast<      fdk::Vec3f&>(chan[Chan_PL_ ]); }
      const fdk::Vec3f&     PL() const { return reinterpret_cast<const fdk::Vec3f&>(chan[Chan_PL_ ]); }
      fdk::Vec3f&           PW()       { return reinterpret_cast<      fdk::Vec3f&>(chan[Chan_PW_ ]); }
      const fdk::Vec3f&     PW() const { return reinterpret_cast<const fdk::Vec3f&>(chan[Chan_PW_ ]); }
      fdk::Vec3f&           MB()       { return reinterpret_cast<      fdk::Vec3f&>(chan[Chan_MB_ ]); }
      const fdk::Vec3f&     MB() const { return reinterpret_cast<const fdk::Vec3f&>(chan[Chan_MB_ ]); }
      fdk::Vec4f&           UV()       { return reinterpret_cast<      fdk::Vec4f&>(chan[Chan_UV_ ]); }
      const fdk::Vec4f&     UV() const { return reinterpret_cast<const fdk::Vec4f&>(chan[Chan_UV_ ]); }
      fdk::Vec3f&            N()       { return reinterpret_cast<      fdk::Vec3f&>(chan[Chan_N_  ]); }
      const fdk::Vec3f&      N() const { return reinterpret_cast<const fdk::Vec3f&>(chan[Chan_N_  ]); }
      fdk::Vec3f&          VEL()       { return reinterpret_cast<      fdk::Vec3f&>(chan[Chan_VEL_]); }
      const fdk::Vec3f&    VEL() const { return reinterpret_cast<const fdk::Vec3f&>(chan[Chan_VEL_]); }

      fdk::Vec4f&           Cf()       { return reinterpret_cast<      fdk::Vec4f&>(chan[Chan_Red]); }
      const fdk::Vec4f&     Cf() const { return reinterpret_cast<const fdk::Vec4f&>(chan[Chan_Red]); }
      fdk::Vec4f&         rgba()       { return reinterpret_cast<      fdk::Vec4f&>(chan[Chan_Red]); }
      const fdk::Vec4f&   rgba() const { return reinterpret_cast<const fdk::Vec4f&>(chan[Chan_Red]); }
      fdk::Vec3f&          rgb()       { return reinterpret_cast<      fdk::Vec3f&>(chan[Chan_Red]); }
      const fdk::Vec3f&    rgb() const { return reinterpret_cast<const fdk::Vec3f&>(chan[Chan_Red]); }
      float&                 r()       { return chan[Chan_Red  ]; }
      float                  r() const { return chan[Chan_Red  ]; }
      float&                 g()       { return chan[Chan_Green]; }
      float                  g() const { return chan[Chan_Green]; }
      float&                 b()       { return chan[Chan_Blue ]; }
      float                  b() const { return chan[Chan_Blue ]; }
      float&                 a()       { return chan[Chan_Alpha]; }
      float                  a() const { return chan[Chan_Alpha]; }
      float&               red()       { return chan[Chan_Red  ]; }
      float                red() const { return chan[Chan_Red  ]; }
      float&             green()       { return chan[Chan_Green]; }
      float              green() const { return chan[Chan_Green]; }
      float&              blue()       { return chan[Chan_Blue ]; }
      float               blue() const { return chan[Chan_Blue ]; }
      float&             alpha()       { return chan[Chan_Alpha]; }
      float              alpha() const { return chan[Chan_Alpha]; }

      float&            coverage()       { return chan[Chan_Coverage]; }
      float             coverage() const { return chan[Chan_Coverage]; }
      float&         cutoutAlpha()       { return chan[Chan_Cutout_Alpha]; }
      float          cutoutAlpha() const { return chan[Chan_Cutout_Alpha]; }
      float&         cutoutDepth()       { return chan[Chan_Cutout_Depth]; }
      float          cutoutDepth() const { return chan[Chan_Cutout_Depth]; }

      fdk::Vec3f&          color()       { return   rgb(); }
      const fdk::Vec3f&    color() const { return   rgb(); }
      float&             opacity()       { return alpha(); }
      float              opacity() const { return alpha(); }

      fdk::Vec3f&       position()       { return  PW(); }
      const fdk::Vec3f& position() const { return  PW(); }
      fdk::Vec3f&       velocity()       { return VEL(); }
      const fdk::Vec3f& velocity() const { return VEL(); }
      fdk::Vec3f&         normal()       { return   N(); }
      const fdk::Vec3f&   normal() const { return   N(); }


#if kDDImageVersionMajorNum <= 17
    public:
      //--------------------------------------------------------------------------------
      // Deprecated blending functions with incorrect math - will disappear in a Nuke major rev after 16
      //--------------------------------------------------------------------------------
      mFnDeprecatedInNuke14("under(Pixel, float, ChannelSet) has been deprecated, use under(Pixel, ChannelSet) instead.")
        void under(const Pixel& A, float a, const ChannelSet& channels);
      mFnDeprecatedInNuke14("under(Pixel, float) has been deprecated, use under(Pixel) instead.")
        void under(const Pixel& A, float a);
#else
#error "Remove the above code block in Nuke 18 as they are deprecated in Nuke 14."
#endif

    private:
      InterestRatchet* _interestRatchet = nullptr;  //!< Only useful when Pixel is passed to Iop Tile sampling methods
    };


    inline std::ostream& operator << (std::ostream& o, const Pixel& p)
    {
      o << "[";
      for (auto z : p.channels) {
        o << " " << z << "(" << p[z] << ")";
      }
      o << " ]";
      return o;
    }

//------------------------------------------------
inline fdk::Vec3f Pixel::getVec3Chans(Channel x, Channel y, Channel z) const { return fdk::Vec3f(chan[x], chan[y], chan[z]); }
inline fdk::Vec3f Pixel::getVec3Chans(Channel* clist) const { return getVec3Chans(clist[0], clist[1], clist[2]); }

inline fdk::Vec4f Pixel::getVec4Chans(Channel x, Channel y, Channel z, Channel w) const { return fdk::Vec4f(chan[x], chan[y], chan[z], chan[w]); }
inline fdk::Vec4f Pixel::getVec4Chans(Channel* clist) const { return getVec4Chans(clist[0], clist[1], clist[2], clist[3]); }

inline void Pixel::setVec2Chans(float x, float y, Channel* clist)
{
  chan[clist[0]] = x;
  chan[clist[1]] = y;
}
inline void Pixel::setVec3Chans(float x, float y, float z, Channel* clist)
{
  chan[clist[0]] = x;
  chan[clist[1]] = y;
  chan[clist[2]] = z;
}
inline void Pixel::setVec4Chans(float x, float y, float z, float w, Channel* clist)
{
  chan[clist[0]] = x;
  chan[clist[1]] = y;
  chan[clist[2]] = z;
  chan[clist[3]] = w;
}
//------------------------------------------------
inline Pixel
Pixel::operator *  (float d) const
{
  Pixel p(channels);
  for (auto z : channels) { p.chan[z] = chan[z] * d; }
  return p;
}
inline Pixel&
Pixel::operator *= (float d)
{
  for (auto z : channels) { chan[z] *= d; }
  return *this;
}
inline Pixel
Pixel::operator *  (const Pixel& v) const
{
  Pixel p(channels);
  for (auto z : v.channels) { p.chan[z] = chan[z] * v.chan[z]; }
  return p;
}
inline Pixel&
Pixel::operator *= (const Pixel& v)
{
  for (auto z : v.channels) { chan[z] *= v.chan[z]; }
  return *this;
}
//------------------------------------------------
inline Pixel
Pixel::operator /  (float d) const
{
  const float id = 1.0f / d;
  Pixel p(channels);
  for (auto z : channels) { p.chan[z] = chan[z] * id; }
  return p;
}
inline Pixel&
Pixel::operator /= (float d)
{
  const float id = 1.0f / d;
  for (auto z : channels) { chan[z] *= id; }
  return *this;
}
inline Pixel
Pixel::operator /  (const Pixel& v) const
{
  Pixel p(channels);
  for (auto z : v.channels) { p.chan[z] = chan[z] / v.chan[z]; }
  return p;
}
inline Pixel&
Pixel::operator /= (const Pixel& v)
{
  for (auto z : v.channels) { chan[z] /= v.chan[z]; }
  return *this;
}
//------------------------------------------------
inline Pixel
Pixel::operator +  (float d) const
{
  Pixel p(channels);
  for (auto z : channels) { p.chan[z] = chan[z] + d; }
  return p;
}
inline Pixel&
Pixel::operator += (float d)
{
  for (auto z : channels) { chan[z] += d; }
  return *this;
}
inline Pixel
Pixel::operator +  (const Pixel& v) const
{
  Pixel p(channels);
  for (auto z : v.channels) { p.chan[z] = chan[z] + v.chan[z]; }
  return p;
}
inline Pixel&
Pixel::operator += (const Pixel& v)
{
  for (auto z : v.channels) { chan[z] += v.chan[z]; }
  return *this;
}
//------------------------------------------------
inline Pixel
Pixel::operator -  (float d) const
{
  Pixel p(channels);
  for (auto z : channels) { p.chan[z] = chan[z] - d; }
  return p;
}
inline Pixel&
Pixel::operator -= (float d)
{
  for (auto z : channels) { chan[z] -= d; }
  return *this;
}
inline Pixel
Pixel::operator -  (const Pixel& v) const
{
  Pixel p(channels);
  for (auto z : v.channels) { p.chan[z] = chan[z] - v.chan[z]; }
  return p;
}
inline Pixel&
Pixel::operator -= (const Pixel& v)
{
  for (auto z : v.channels) { chan[z] -= v.chan[z]; }
  return *this;
}
//------------------------------------------------

inline void
Pixel::lerp(const Pixel& b, float t)
{
  const float invt = 1.0f - t;
  for (auto z : b.channels) { chan[z] = (chan[z] * invt) + (b.chan[z] * t); }
}

inline void
Pixel::lerp(const Pixel& b, float t, Channel z)
{
  chan[z] = (chan[z] * (1.0f - t)) + b.chan[z] * t;
}

inline void
Pixel::lerp(const Pixel& b, float t, const ChannelSet& channels)
{
  const float invt = 1.0f - t;
  for (auto z : channels) { chan[z] = (chan[z] * invt) + (b.chan[z] * t); }
}

inline void
Pixel::lerp(const Pixel& b, float t, const ChannelPack& channels)
{
  const float invt = 1.0f - t;
  for (auto z : channels) { chan[z] = (chan[z] * invt) + (b.chan[z] * t); }
}

inline void
Pixel::over(const Pixel& A, float a)
{
  if (a < std::numeric_limits<float>::epsilon()) {
    for (unsigned z = 1; z <= 4; ++z) { chan[z] += A.chan[z]; }
  }
  else if (a < 1.0f) {
    a = 1.0f - a;
    for (unsigned z = 1; z <= 4; ++z) { chan[z] = (chan[z] * a) + A.chan[z]; }
  }
  else {
    for (unsigned z = 1; z <= 4; ++z) { chan[z] = A.chan[z]; }
  }
}

inline void
Pixel::over(const Pixel& A, float a, Channel z)
{
  if (a < std::numeric_limits<float>::epsilon()) {
    chan[z] += A.chan[z];
  }
  else if (a < 1.0f) {
    chan[z] = (chan[z] * (1.0f - a)) + A.chan[z];
  }
  else {
    chan[z] = A.chan[z];
  }
}

inline void
Pixel::over(const Pixel& A, float a, const ChannelSet& channels)
{
  if (a < std::numeric_limits<float>::epsilon()) {
    for (auto z : channels) { chan[z] += A.chan[z]; }
  }
  else if (a < 1.0f) {
    a = 1.0f - a;
    for (auto z : channels) { chan[z] = (chan[z] * a) + A.chan[z]; }
  }
  else {
    for (auto z : channels) { chan[z] = A.chan[z]; }
  }
}

inline void
Pixel::over(const Pixel& A, float a, const ChannelPack& channels)
{
  if (a < std::numeric_limits<float>::epsilon()) {
    for (auto z : channels) { chan[z] += A.chan[z]; }
  }
  else if (a < 1.0f) {
    a = 1.0f - a;
    for (auto z : channels) { chan[z] = (chan[z] * a) + A.chan[z]; }
  }
  else {
    for (auto z : channels) { chan[z] = A.chan[z]; }
  }
}

inline void
Pixel::under(float Ba, const Pixel& A)
{
  if (Ba < std::numeric_limits<float>::epsilon()) {
    for (unsigned z = 1; z <= 4; ++z) { chan[z] += A.chan[z]; }
  }
  else if (Ba < 1.0f) {
    Ba = 1.0f - Ba;
    for (unsigned z = 1; z <= 4; ++z) { chan[z] += A.chan[z] * Ba; }
  }
}

inline void
Pixel::under(float Ba, const Pixel& A, Channel z)
{
  if (Ba < std::numeric_limits<float>::epsilon()) {
    chan[z] += A.chan[z];
  }
  else if (Ba < 1.0f) {
    chan[z] += A.chan[z] * (1.0f - Ba);
  }
}

inline void
Pixel::under(float Ba, const Pixel& A, const ChannelSet& channels)
{
  if (Ba < std::numeric_limits<float>::epsilon()) {
    for (auto z : channels) { chan[z] += A.chan[z]; }
  }
  else if (Ba < 1.0f) {
    Ba = 1.0f - Ba;
    for (auto z : channels) { chan[z] += A.chan[z] * Ba; }
  }
}

inline void
Pixel::under(float Ba, const Pixel& A, const ChannelPack& channels)
{
  if (Ba < std::numeric_limits<float>::epsilon()) {
    for (auto z : channels) { chan[z] += A.chan[z]; }
  }
  else if (Ba < 1.0f) {
    Ba = 1.0f - Ba;
    for (auto z : channels) { chan[z] += A.chan[z] * Ba; }
  }
}

inline void Pixel::under(const Pixel& A) { under(alpha(), A); }
inline void Pixel::under(const Pixel& A, Channel z) { under(alpha(), A, z); }
inline void Pixel::under(const Pixel& A, const ChannelSet& channels) { under(alpha(), A, channels); }
inline void Pixel::under(const Pixel& A, const ChannelPack& channels) { under(alpha(), A, channels); }

inline Pixel Pixel::get(const ChannelSet& getChans) const
{
  Pixel b(getChans);
  for (auto z : getChans) { b.chan[z] = chan[z]; }
  return b;
}

inline Pixel Pixel::get(const ChannelPack& getChans) const
{
  Pixel b;
  b.channels = Mask_None;
  for (auto z : getChans) {
    b.chan[z] = chan[z];
    b.channels += z;
  }
  return b;
}

//------------------------------------------------
inline void Pixel::multiply(float d, const ChannelSet& channels) { for (auto z : channels) { chan[z] *= d; } }
inline void Pixel::multiply(float d, const ChannelPack& channels) { for (auto z : channels) { chan[z] *= d; } }
inline void Pixel::multiply(const Pixel& b, const ChannelSet& channels) { for (auto z : channels) { chan[z] *= b.chan[z]; } }
inline void Pixel::multiply(const Pixel& b, const ChannelPack& channels) { for (auto z : channels) { chan[z] *= b.chan[z]; } }
inline void Pixel::multiplyVec2Chans(float x, float y, Channel* clist)
{
  chan[clist[0]] *= x;
  chan[clist[1]] *= y;
}
inline void Pixel::multiplyVec3Chans(float x, float y, float z, Channel* clist)
{
  chan[clist[0]] *= x;
  chan[clist[1]] *= y;
  chan[clist[2]] *= z;
}
inline void Pixel::multiplyVec4Chans(float x, float y, float z, float w, Channel* clist)
{
  chan[clist[0]] *= x;
  chan[clist[1]] *= y;
  chan[clist[2]] *= z;
  chan[clist[3]] *= w;
}
//------------------------------------------------
inline void Pixel::divide(float d, const ChannelSet& channels)
{
  const float id = 1.0f / d;
  for (auto z : channels) { chan[z] *= id; }
}
inline void Pixel::divide(float d, const ChannelPack& channels)
{
  const float id = 1.0f / d;
  for (auto z : channels) { chan[z] *= id; }
}
inline void Pixel::divide(const Pixel& b, const ChannelSet& channels) { for (auto z : channels) { chan[z] /= b.chan[z]; } }
inline void Pixel::divide(const Pixel& b, const ChannelPack& channels) { for (auto z : channels) { chan[z] /= b.chan[z]; } }
inline void Pixel::divideVec2Chans(float x, float y, Channel* clist)
{
  chan[clist[0]] /= x;
  chan[clist[1]] /= y;
}
inline void Pixel::divideVec3Chans(float x, float y, float z, Channel* clist)
{
  chan[clist[0]] /= x;
  chan[clist[1]] /= y;
  chan[clist[2]] /= z;
}
inline void Pixel::divideVec4Chans(float x, float y, float z, float w, Channel* clist)
{
  chan[clist[0]] /= x;
  chan[clist[1]] /= y;
  chan[clist[2]] /= z;
  chan[clist[3]] /= w;
}
//------------------------------------------------
inline void Pixel::plus(float d, const ChannelSet& channels) { for (auto z : channels) { chan[z] += d; } }
inline void Pixel::plus(float d, const ChannelPack& channels) { for (auto z : channels) { chan[z] += d; } }
inline void Pixel::plus(const Pixel& b, const ChannelSet& channels) { for (auto z : channels) { chan[z] += b.chan[z]; } }
inline void Pixel::plus(const Pixel& b, const ChannelPack& channels) { for (auto z : channels) { chan[z] += b.chan[z]; } }
inline void Pixel::plusWeighted(const float* array, const ChannelSet& channels, float w) { for (auto z : channels) { chan[z] += array[z] * w; } }
inline void Pixel::plusWeighted(const float* array, const ChannelPack& channels, float w) { for (auto z : channels) { chan[z] += array[z] * w; } }
inline void Pixel::plusWeighted(const Pixel& b, const ChannelSet& channels, float w) { for (auto z : channels) { chan[z] += b.chan[z] * w; } }
inline void Pixel::plusWeighted(const Pixel& b, const ChannelPack& channels, float w) { for (auto z : channels) { chan[z] += b.chan[z] * w; } }
inline void Pixel::plusVec2Chans(float x, float y, Channel* clist)
{
  chan[clist[0]] += x;
  chan[clist[1]] += y;
}
inline void Pixel::plusVec3Chans(float x, float y, float z, Channel* clist)
{
  chan[clist[0]] += x;
  chan[clist[1]] += y;
  chan[clist[2]] += z;
}
inline void Pixel::plusVec4Chans(float x, float y, float z, float w, Channel* clist)
{
  chan[clist[0]] += x;
  chan[clist[1]] += y;
  chan[clist[2]] += z;
  chan[clist[3]] += w;
}
//------------------------------------------------
inline void Pixel::minus(float d, const ChannelSet& channels) { for (auto z : channels) { chan[z] -= d; } }
inline void Pixel::minus(float d, const ChannelPack& channels) { for (auto z : channels) { chan[z] -= d; } }
inline void Pixel::minus(const Pixel& b, const ChannelSet& channels) { for (auto z : channels) { chan[z] -= b.chan[z]; } }
inline void Pixel::minus(const Pixel& b, const ChannelPack& channels) { for (auto z : channels) { chan[z] -= b.chan[z]; } }
inline void Pixel::minusVec2Chans(float x, float y, Channel* clist)
{
  chan[clist[0]] -= x;
  chan[clist[1]] -= y;
}
inline void Pixel::minusVec3Chans(float x, float y, float z, Channel* clist)
{
  chan[clist[0]] -= x;
  chan[clist[1]] -= y;
  chan[clist[2]] -= z;
}
inline void Pixel::minusVec4Chans(float x, float y, float z, float w, Channel* clist)
{
  chan[clist[0]] -= x;
  chan[clist[1]] -= y;
  chan[clist[2]] -= z;
  chan[clist[3]] -= w;
}
//------------------------------------------------

#if kDDImageVersionMajorNum <= 17
// Deprecated, uses incorrect math
inline void
Pixel::under(const Pixel& A, float a, const ChannelSet& channels)
{
  if (a < 0.0001f) {
    for (auto z : channels) { chan[z] = A.chan[z]; }
  }
  else if (a < 0.9999f) {
    a = 1.0f - a;
    for (auto z : channels) { chan[z] += A.chan[z] * a; }
  }
  else {
    for (auto z : channels) { chan[z] += A.chan[z]; }
  }
}

// Deprecated, uses incorrect math
inline void
Pixel::under(const Pixel& A, float a)
{
  if (a < 0.0001f) {
    for (unsigned z = 1; z <= 4; ++z) { chan[z] = A.chan[z]; }
  }
  else if (a < 0.9999f) {
    a = 1.0f - a;
    for (unsigned z = 1; z <= 4; ++z) { chan[z] += A.chan[z] * a; }
  }
  else {
    for (unsigned z = 1; z <= 4; ++z) { chan[z] += A.chan[z]; }
  }
}
#else
#error "Remove the above code block in Nuke 18 as they are deprecated in Nuke 14."
#endif


  }
}

#endif
