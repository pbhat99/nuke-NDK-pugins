// Copyright (c) 2024 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef SLR_AOVLAYER_H
#define SLR_AOVLAYER_H

#include "ndk/base/Channel.h"
#include "DDImage/Channel.h"
#include "DDImage/ChannelSet.h"
#include "DDImage/ChannelPack.h"
#include "usg/base/Value.h" // for Value::Type

#include <vector>
#include <map>

namespace slr
{
  class ShadingSample;


  /*! Stores input and output channel indice mappings.
  */
  class SlrAovLayer
  {
  public:
    //!
    enum UnpremultMode
    {
      UNPREMULT_BY_COVERAGE,
      UNPREMULT_BY_ALPHA,
      NO_UNPREMULT
    };

    //!
    enum MergeMode
    { 
      MERGE_PREMULT_BLEND,
      MERGE_BLEND,
      MERGE_PLUS,
      MERGE_MIN,
      MERGE_MID,
      MERGE_MAX
    };

    enum XformToSpace
    {
      XFORM_TOSPACE_OFF,
      XFORM_TOSPACE_SHADING,
      XFORM_TOSPACE_CAMERA,
      XFORM_TOSPACE_WORLD
    };

    typedef fdk::Vec3f (*Handler)(const SlrAovLayer& aov, ShadingSample&); // for internal use only


  public:
    std::string           name;             //!< Layer name - 'P', 'N', 'Ns', etc.
    bool                  enabled;          //!< Aov enabled
    ndk::Channel          inChan;           //!< Starting input chan (assumes chans in sequential order!)
    int32_t               nInChans;         //!< Number of input channels to copy, 1,2or3
    usg::Value::Type      inValueType;      //!< Float2, Float3, Normal, Point, etc.
    ndk::Channel::Type    inPurpose;        //!< Vec2, Vec3, etc
    ndk::Channel::Interpretation inInterp;  //!< PositionXYZ, NormalXYZ, etc
    SlrAovLayer::Handler  inHandler;        //!< Source data retrieval routine
    //
    DD::Image::Channel    outChan[3];       //!< List of output channels, max of 3
    UnpremultMode         unpremultMode;    //!< Unpremult by coverage mode
    MergeMode             mergeMode;        //!< Merging mode
    XformToSpace          xformToSpace;     //!< XformToSpace mode

    SlrAovLayer(const char* _name = "", int32_t _nInChans = 0, SlrAovLayer::Handler _handler = nullptr);
  };

  using SlrAovLayerList = std::vector<SlrAovLayer>;
  using SlrAovIndexMap  = std::map<std::string, uint32_t>;


  //------------------------------


  //! Packed list of A->B channel mappings. Used to quickly copy values from
  //! an source channel array to a destination channel array.
  struct SlrAovChannelMap
  {
    struct AovMapping
    {
      const SlrAovLayer* aovLayer;
      ndk::Channel       srcParam;
      DD::Image::Channel outChan;
      AovMapping(const SlrAovLayer* aov, ndk::Channel src, DD::Image::Channel dst) : aovLayer(aov), srcParam(src), outChan(dst) {}
    };
    std::vector<AovMapping> mappings;

    void  clear() { mappings.clear(); }

    // Add mappings from the SlrAovLayer channels.
    void  addAov(const SlrAovLayer& aovLayer);

    void  copy(const float* srcArray, float* outArray);
    void  copy(const float* srcArray, float* outArray, float weight);

    DD::Image::ChannelSet   channels();
    DD::Image::ChannelPack  channelPack();

    //! Iterator support
    std::vector<AovMapping>::const_iterator  begin() const { return mappings.begin(); }
    std::vector<AovMapping>::const_iterator  end()   const { return mappings.end(); }
  };

  std::ostream& operator<<(std::ostream&, const SlrAovChannelMap&);


  //-------------------------------------------------------------------------


  //! Output DDImage channel index iterator to support simple for loops in shaders.
  //!   for ( Channel z : aovLayer ) { ... }
  class SlrAovLayerIterator {
  public:
    SlrAovLayerIterator(const SlrAovLayer& aov, bool end = false) : _aov(aov), _current(end ? aov.nInChans : 0) {}

    const DD::Image::Channel& operator++() { return _aov.outChan[++_current]; }

    bool operator!=(const SlrAovLayerIterator& other) { return _current != other._current; }

    const DD::Image::Channel& operator*() { return _aov.outChan[_current]; }

  private:
    const SlrAovLayer& _aov;
    int32_t            _current;
  };

  // Implementation of begin() & end() needed to support ranged-for loops.
  inline SlrAovLayerIterator  begin(const SlrAovLayer& channels) { return SlrAovLayerIterator(channels); }
  inline SlrAovLayerIterator  end(const SlrAovLayer& channels) { return SlrAovLayerIterator(channels, true/*end*/); }


//------------------------------------------------------------------
//------------------------------------------------------------------
//------------------------------------------------------------------

inline SlrAovLayer::SlrAovLayer(const char* _name, int32_t _nInChans, SlrAovLayer::Handler _handler)
  : name((_name) ? _name : "")
  , enabled(false)
  , inChan(ndk::Chan_Empty)
  , nInChans(_nInChans)
  , inValueType(usg::Value::Type::InvalidType)
  , inPurpose(ndk::Channel::Type::NoType)
  , inInterp(ndk::Channel::Interpretation::NoInterp)
  , inHandler(_handler)
  , unpremultMode(UNPREMULT_BY_COVERAGE)
  , mergeMode(MERGE_BLEND)
  , xformToSpace(XFORM_TOSPACE_OFF)
{
  outChan[0] = outChan[1] = outChan[2] = DD::Image::Chan_Black;
}

inline std::ostream& operator<<(std::ostream& o, const SlrAovChannelMap& mapper)
{
  o << "[";
  for (const auto& map : mapper.mappings) {
    o << " " << map.srcParam.index() << "->" << static_cast<uint32_t>(map.outChan);
  }
  o << " ]";
  return o;
}


inline void SlrAovChannelMap::addAov(const SlrAovLayer& aovLayer)
{
  for (int32_t i = 0; i < aovLayer.nInChans; ++i) {
    mappings.emplace_back(AovMapping(&aovLayer, aovLayer.inChan + i, aovLayer.outChan[i]));
  }
}

inline DD::Image::ChannelSet SlrAovChannelMap::channels()
{
  DD::Image::ChannelSet chans;
  for (const auto& map : mappings) {
    chans += map.outChan;
  }
  return chans;
}

inline DD::Image::ChannelPack SlrAovChannelMap::channelPack()
{
  return DD::Image::ChannelPack(channels());
}


} // namespace slr


#endif
