// Copyright (c) 2023 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef NDK_GEO_RENDER_SAMPLINGRNG_H
#define NDK_GEO_RENDER_SAMPLINGRNG_H

#include "ndk/api.h"

#include "fdk/math/types.h"

#include <vector>
#include <limits>

namespace ndk {


  /*! \class SamplingRng
  */
  class NDK_API SamplingRng
  {
  public:
    //!
    SamplingRng(int32_t seed = 27) { setSeed(seed); }

    //! Reset to default set.
    void  resetToDefault() { setSeed(27); }

    //! Assign a new seed value to the Rng.
    void     setSeed(int32_t seed);
    //! Change the Rng seed to a new pseudo-random value, and returning it.
    int32_t  scrambleSeed();

    //! Return the current permutation value for the current seed.
    int32_t  value() const { return _val; }
    //! Change the permutation value and scramble the seed.
    int32_t  newValue();

    float    newFloatValue() { return MakeFloat(newValue()); }
    double   newDoubleValue() { return MakeDouble(newValue()); }

    //! Normalize a permutation value to float range 0..1
    static inline float   MakeFloat(int32_t v);

    //! Normalize a permutation value to double range 0..1
    static inline double  MakeDouble(int32_t v);

    //! Build a 32-bit seed by hashing/scrambling one value.
    static int32_t  BuildHashedSeed(int32_t v);

    //! Build a 32-bit seed by hashing/scrambling three values.
    static int32_t  BuildHashedSeed(int32_t vX, int32_t vY, int32_t vZ);

    //! Build a 32-bit seed by hashing/scrambling four values.
    static int32_t  BuildHashedSeed(int32_t vX, int32_t vY, int32_t vZ, int32_t vW);


  public:
    //! Create a set of jittered, well-stratified 1D samples, changing the Rng seed.
    void  generate1dSamples(int32_t n, fdk::FloatList& samples);

    //! Create a set of multi-jittered, well-stratified 2D samples, changing the Rng seed.
    //! The values are distributed in the range -1...+1 in both x & y.
    void  generate2dSamples(int32_t w, int32_t h, fdk::Vec2fList& samples);

    //! Create a set of uniformly-distributed 2D samples, NOT changing the Rng seed.
    //! The values are distributed in the range -1...+1 in both x & y.
    void  generate2dUniformSamples(int32_t w, int32_t h, fdk::Vec2fList& samples);


  private:
    int32_t _seed, _val;
    int32_t _table[32];

    //! Temporary vectors used in sample generation.
    //! They don't need to be member variables, they are just stored here to
    //! avoid reallocations when iterating through scanlines.
    //! Please note that the that this makes the SamplingRng objects non-threadsafe.
    std::vector<int32_t> _perm1d;
    std::vector<int32_t> _perm2d;
    std::vector<int32_t> _index;
    std::vector<fdk::Vec2f> _bin_grid;
  };

  using SamplingRngList = std::vector<SamplingRng>;


  //--------------------------------------------------------------------------


  /*! \class SamplingRngSet
  */
  class NDK_API SamplingRngSet
  {
  public:
    //!
    SamplingRngSet(int32_t nPnts = 0) : _rngs(nPnts) {}

    int32_t  size() const { return _rngs.size(); }
    void  resize(int32_t nPnts) { _rngs.resize(nPnts); }

    //! SamplingRng for index i
    SamplingRng&  rng(int32_t i) { return _rngs[i]; }

    SamplingRng&       operator [] (int i)       { return _rngs[i]; }
    const SamplingRng& operator [] (int i) const { return _rngs[i]; }


  protected:
    SamplingRngList  _rngs;   //!< List of SamplingRngs, usually 1 per sample point

  };


//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

inline float SamplingRng::MakeFloat(int32_t v)
{
  return std::min(static_cast<float>(v) / static_cast<float>(std::numeric_limits<int32_t>::max()),
                    (1.0f - std::numeric_limits<float>::epsilon()));
}

inline double SamplingRng::MakeDouble(int32_t v)
{
  return std::min(static_cast<double>(v) / static_cast<double>(std::numeric_limits<int32_t>::max()),
                    (1.0  - std::numeric_limits<double>::epsilon()));
}

inline int32_t SamplingRng::BuildHashedSeed(int32_t seed)
{
  seed += (seed << 10u);
  seed ^= (seed >>  6u);
  seed += (seed <<  3u);
  seed ^= (seed >> 11u);
  seed += (seed << 15u);
  return seed;
}

inline int32_t SamplingRng::BuildHashedSeed(int32_t vX, int32_t vY, int32_t vZ)
{
  uint32_t seedX = reinterpret_cast<uint32_t&>(vX) * 1664525u + 1013904223u;
  uint32_t seedY = reinterpret_cast<uint32_t&>(vY) * 1664525u + 1013904223u;
  uint32_t seedZ = reinterpret_cast<uint32_t&>(vZ) * 1664525u + 1013904223u;

  seedX += seedY * seedZ;
  seedY += seedZ * seedX;
  seedZ += seedX * seedY;

  seedX ^= seedX >> 16u;
  seedY ^= seedY >> 16u;
  seedZ ^= seedZ >> 16u;

  seedX += seedY * seedZ;

  return reinterpret_cast<int32_t&>(seedX);
}

inline int32_t SamplingRng::BuildHashedSeed(int32_t vX, int32_t vY, int32_t vZ, int32_t vW)
{
  uint32_t seedX = reinterpret_cast<uint32_t&>(vX) * 1664525u + 1013904223u;
  uint32_t seedY = reinterpret_cast<uint32_t&>(vY) * 1664525u + 1013904223u;
  uint32_t seedZ = reinterpret_cast<uint32_t&>(vZ) * 1664525u + 1013904223u;
  uint32_t seedW = reinterpret_cast<uint32_t&>(vW) * 1664525u + 1013904223u;

  seedX += seedY * seedW;
  seedY += seedZ * seedX;
  seedZ += seedX * seedY;
  seedW += seedY * seedZ;

  seedX ^= seedX >> 16u;
  seedY ^= seedY >> 16u;
  seedZ ^= seedZ >> 16u;
  seedW ^= seedW >> 16u;

  seedX += seedY * seedW;

  return reinterpret_cast<int32_t&>(seedX);
}

} // namespace ndk

#endif
