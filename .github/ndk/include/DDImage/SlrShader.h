// Copyright (c) 2021 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef SLR_SHADER_H
#define SLR_SHADER_H

#include "DDImage/PluginObject.h"
#include "DDImage/Pixel.h"             // convenience for SlrShader plugins
#include "DDImage/Sampler.h"           // convenience for SlrShader plugins
#include "DDImage/SlrEngineContext.h"  // convenience for SlrShader plugins
#include "DDImage/SlrShadingContext.h" // convenience for SlrShader plugins

#include "fdk/base/Hash.h"
#include "fdk/math/Mat4.h"

#include "usg/geom/ShaderDesc.h"
#include "usg/geom/Prim.h"
#include "usg/geom/LightPrim.h"

#include <unordered_map>
#include <unordered_set>


namespace slr
{
  class SlrCoShader;
  class SlrSurfaceShader;
  class SlrGeometryShader;
  class SlrLightShader;
  class SlrVolumeShader;
  class SlrLensShader;

  //------------------------------------------------------------------------

  //
  // Helper functions for shaders to convert common property types into Nuke objects.
  //

  //! Convert a local, uniform Float usg::ShaderProperty (non animating, non connecting) into a single float.
  DDImage_API
  float    GetUniformInputFloat(const usg::ShaderDesc& shaderDesc, const char* propName, float defVal = 0.0f);

  //! Convert a local, uniform Int usg::ShaderProperty (non animating, non connecting) into a single int32.
  DDImage_API
  int32_t  GetUniformInputInt(const usg::ShaderDesc& shaderDesc, const char* propName, int32_t defVal = 0);

  //! Convert a local, uniform Int usg::ShaderProperty (non animating, non connecting) into a single bool.
  DDImage_API
  bool     GetUniformInputIntAsBool(const usg::ShaderDesc& shaderDesc, const char* propName, bool defVal = false);

  //-------------------------

  //! Convert an Int usg::ShaderProperty (non animating, non connecting) into a DD::Image::Channel.
  DDImage_API
  DD::Image::Channel  GetInputIntValueAsChannel(const usg::ShaderDesc&    shaderDesc,
                                                const char*               propName,
                                                const DD::Image::Channel& defChan = DD::Image::Chan_Black);

  //! Convert an IntArray or Int usg::ShaderProperty (non animating, non connecting) into a list
  //! of DD::Image::Channels.
  //! Default size must be > 0 otherwise nothing happens, because the property may not provide
  //! the expected array size. Typically this will be 3 (an RGB knob) or 4 (an RGBA knob).
  DDImage_API
  std::vector<DD::Image::Channel>  GetInputIntArrayValueAsChannelArray(const usg::ShaderDesc& shaderDesc,
                                                                       const char*            propName,
                                                                       int32_t                defaultSize = 0);

  //! Convert an IntArray or Int usg::ShaderProperty (non animating, non connecting) into a
  //! DD::Image::ChannelSet.
  DDImage_API
  DD::Image::ChannelSet  GetInputIntArrayValueAsChannelSet(const usg::ShaderDesc&       shaderDesc,
                                                           const char*                  propName,
                                                           const DD::Image::ChannelSet& defChannels = DD::Image::Mask_None);

  //------------------------------------------------------------------------

  //! One of these is stored in the thread safe SlrThreadContext for every SlrShader
  //! created allowing an SlrShader to safely store data that needs to be per thread,
  //! like DD::Image::Samplers, DD::Image::Pixels, etc.
  struct SlrShaderThreadData
  {
    typedef void (*DeleteFunc)(void* self);
    void*      data = nullptr;        //!< Allocated data blob
    size_t     dataSize = 0;          //!< Size of data blob in bytes
    DeleteFunc deleteFunc = nullptr;  //!< Function to call when data blob is deleted
  };

  //--------------------------------------------------

  // Convenience for building OutputPortList functions
  #define SLRSAMPLEFUNC(A) reinterpret_cast<void*>(A)

  //--------------------------------------------------

  /*! \class DD::Image::SlrShader

    ScanlineRender-compatible shader class intended to be created
    through a plugin Description from an abstract shader description.

    The SlrSurfaceShader and SlrLightShader subclasses include the basic surface
    and lighting shader methods that are expected to be called from shading code.

  */
  class DDImage_API SlrShader : public DD::Image::PluginObject
  {
  public:
    /*! Limited data types for shader inputs.
       (we'll use Nuke nomenclature here for no good reason...this likely should change
        to use ndk Value::Type)
    */
    enum KnobType
    {
      STRING_KNOB,      //!< std::string
      //
      INT_KNOB,         //!< Also used for boolean
      FLOAT_KNOB,       //!<
      DOUBLE_KNOB,      //!<
      //
      VEC2_KNOB,        //!< fdk::Vec2f
      VEC3_KNOB,        //!< fdk::Vec3f
      VEC4_KNOB,        //!< fdk::Vec4f
      MAT4_KNOB,        //!< fdk::Mat4d
      //
      COLOR3_KNOB,      //!< fdk::Vec3f - rgb
      COLOR4_KNOB,      //!< fdk::Vec4f - rgba
      COLOR_R_KNOB,     //!< 1 float 'red'
      COLOR_G_KNOB,     //!< 1 float 'green'
      COLOR_B_KNOB,     //!< 1 float 'blue'
      COLOR_A_KNOB,     //!< 1 float 'alpha'
      //
      INTARRAY_KNOB,    //!< Array of int32s
      FLOATARRAY_KNOB,  //!< Array of floats
      VEC2ARRAY_KNOB,   //!< Array of Vec2fs
      VEC3ARRAY_KNOB,   //!< Array of Vec3fs
      VEC4ARRAY_KNOB,   //!< Array of Vec4fs
      //
      PIXEL_KNOB,       //!< Pixel (also contains a ChannelSet)
      //
      INVALID_KNOB,
      NUM_KNOB_TYPES = INVALID_KNOB
    };

    //! Return the string version of the type enum.
    static const char* typeString(KnobType type);

    //! Return the number of data elements in the type enum.
    static size_t numElements(KnobType type);

    //! Return the number of float elements in the type enum.
    static size_t numFloats(KnobType type);

    //! Returns true if the knob is a Vec2/3/4 array of floats.
    static bool isVector(KnobType type);

    //! Returns true if the knob is a Color3/4/r/g/b/a type.
    static bool isColor(KnobType type);

    //! Get the color ChannelSet that's appropriate for the KnobType. Data types return Mask_None.
    static DD::Image::ChannelSet getKnobColorChannels(KnobType type,
                                                      bool     isOutput = false);

    //-----------------------------------------------------


    /*! Nuke-specific input mapping
    */
    struct InputKnob
    {
      const char* name;       //!< Pointer to the name string - no local copy is made!
      KnobType    type;       //!< Data type of knob
      bool        isUniform;  //!< Hint that knob should not vary

      InputKnob(const char* _name,
                KnobType    _type,
                bool        is_uniform = false)
        : name((_name) ? _name : "")
        , type(_type)
        , isUniform(is_uniform)
      {
        //
      }

      friend std::ostream& operator << (std::ostream& o, const InputKnob& b)
      {
        o << b.name << "(" << ((b.isUniform) ? "uniform " : "") << typeString(b.type) << ")";
        return o;
      }

    };

    using InputKnobList = std::vector<InputKnob>;

    //-----------------------------------------------------

    /*! Nuke-specific output mapping
    */
    struct OutputPort
    {
      const char*           name;           //!< Pointer to the name string - no local copy is made!
      KnobType              type;           //!< Type of port
      void*                 samplerFunc;    //!< Sampler function, cast to correct class using 'type'.
      int32_t               index;          //!< Port position in OutputPortList
      bool                  isVarying;      //!< Is the port data constant or varying?
      DD::Image::ChannelSet channels;       //!< Port's output channels (cached result from validateShader())
      bool                  hasRgb;         //!< Does port provide Chans RGB? (cached result from validateShader())
      bool                  hasAlpha;       //!< Does port provide Chan_Alpha? (cached result from validateShader())
      bool                  hasPresence;    //!< Does port provide Chan_Coverage? (cached result from validateShader())
      DD::Image::ChannelSet requested;      //!< Intersected set of channels and all requested channels (cached result from requestResources())

      OutputPort(const char* _name,
                 KnobType    _type,
                 void*       sampler_func)
        : name(_name)
        , type(_type)
        , samplerFunc(sampler_func)
        , index(0)
        , isVarying(false)
        , hasRgb(false)
        , hasAlpha(false)
        , hasPresence(false)
        , requested(DD::Image::Mask_None)
      {
        //
      }

      //! Does the port provide the channel?
      inline bool hasChannel(DD::Image::Channel c) const { return (c != DD::Image::Chan_Black && channels.contains(c)); }

      friend std::ostream& operator << (std::ostream& o, const OutputPort& b)
      {
        o << b.index << ":" << b.name << "(" << ((b.isVarying) ? "<varying>" : "<constant>") << typeString(b.type) << ")";
        if (b.channels != DD::Image::Mask_None) {
          o << "[" << b.channels << ", req(" << b.requested << ")]";
          if (b.hasRgb) { o << "^rgb"; }
          if (b.hasAlpha) { o << "^a"; }
        }
        return o;
      }
    };

    using OutputPortList = std::vector<OutputPort>;

    //-----------------------------------------------------


    /*!
    */
    struct Connection
    {
      SlrShader*  sourceShader;         //!
      int32_t     outputPortIndex;      //!

      bool isValid() const { return (sourceShader != nullptr && outputPortIndex != -1); }
      operator bool() const { return isValid(); }

      Connection() : sourceShader(nullptr), outputPortIndex(-1) {}

      //! Retrieve the port from the sourceShader, if valid.
      OutputPort*  getOutputPort() const;

      friend std::ostream& operator << (std::ostream& o, const Connection& b)
      {
        if (b.sourceShader) {
          o << "'" << b.sourceShader->getPath() << "' port " << b.outputPortIndex;
        }
        else {
          o << "<null>";
        }
        return o;
      }
    };

    using ConnectionList = std::vector<Connection>;
    using ConnectionMap  = std::unordered_map<std::string, Connection>;


    //-----------------------------------------------------

    struct IntBinding;
    struct FloatBinding;
    struct DoubleBinding;
    struct Vec2fBinding;
    struct Vec3fBinding;
    struct Vec4fBinding;
    struct Mat4dBinding;
    struct Color4Binding;
    struct PixelBinding;
    //
    struct Dual2fBinding;
    struct Dual2dBinding;
    struct Dual2Vec2fBinding;
    struct Dual2Vec3fBinding;
    struct Dual2Vec4fBinding;

    struct ThreadShaderData;


    /*
        Output sampler handlers, typically assigned in the OutputPortList
    */
    typedef int32_t    (*IntOutputSampler)(const IntBinding&  , const SlrShadingContext*);
    typedef float      (*FloatOutputSampler)(const FloatBinding&, const SlrShadingContext*);
    typedef double     (*DoubleOutputSampler)(const DoubleBinding&, const SlrShadingContext*);
    typedef fdk::Vec2f (*Vec2fOutputSampler)(const Vec2fBinding&, const SlrShadingContext*);
    typedef fdk::Vec3f (*Vec3fOutputSampler)(const Vec3fBinding&, const SlrShadingContext*);
    typedef fdk::Vec4f (*Vec4fOutputSampler)(const Vec4fBinding&, const SlrShadingContext*);
    typedef fdk::Mat4d (*Mat4dOutputSampler)(const Mat4dBinding&, const SlrShadingContext*);
    typedef fdk::Vec4f (*Color4OutputSampler)(const Color4Binding&, const SlrShadingContext*);
    typedef void       (*PixelOutputSampler)(const PixelBinding&, const SlrShadingContext*, const DD::Image::ChannelSet& get, DD::Image::Pixel& out);
    //
    typedef void (*Dual2fOutputSampler)(const Dual2fBinding&, const SlrShadingContext*, Dual2f& out);
    typedef void (*Dual2dOutputSampler)(const Dual2dBinding&, const SlrShadingContext*, Dual2d& out);
    typedef void (*Dual2Vec2fOutputSampler)(const Dual2Vec2fBinding&, const SlrShadingContext*, Dual2Vec2f& out);
    typedef void (*Dual2Vec3fOutputSampler)(const Dual2Vec3fBinding&, const SlrShadingContext*, Dual2Vec3f& out);
    typedef void (*Dual2Vec4fOutputSampler)(const Dual2Vec4fBinding&, const SlrShadingContext*, Dual2Vec4f& out);

    /*
        Dedicated output sampler handlers which return the binding's uniform value.
        Note that for uniformPixelSampler only the enabled channels on the Pixel are cleared.
    */
    static int32_t     uniformIntSampler(const IntBinding&, const SlrShadingContext*);
    static float       uniformFloatSampler(const FloatBinding&, const SlrShadingContext*);
    static double      uniformDoubleSampler(const DoubleBinding&, const SlrShadingContext*);
    static fdk::Vec2f  uniformVec2fSampler(const Vec2fBinding&, const SlrShadingContext*);
    static fdk::Vec3f  uniformVec3fSampler(const Vec3fBinding&, const SlrShadingContext*);
    static fdk::Vec4f  uniformVec4fSampler(const Vec4fBinding&, const SlrShadingContext*);
    static fdk::Mat4d  uniformMat4dSampler(const Mat4dBinding&, const SlrShadingContext*);
    static fdk::Vec4f  uniformColor4Sampler(const Color4Binding&, const SlrShadingContext*);
    static void        uniformPixelSampler(const PixelBinding&, const SlrShadingContext*, const DD::Image::ChannelSet& get, DD::Image::Pixel& out);
    //
    static void uniformDual2fSampler(const Dual2fBinding&, const SlrShadingContext*, Dual2f& out);
    static void uniformDual2dSampler(const Dual2dBinding&, const SlrShadingContext*, Dual2d& out);
    static void uniformDual2Vec2fSampler(const Dual2Vec2fBinding&, const SlrShadingContext*, Dual2Vec2f& out);
    static void uniformDual2Vec3fSampler(const Dual2Vec3fBinding&, const SlrShadingContext*, Dual2Vec3f& out);
    static void uniformDual2Vec4fSampler(const Dual2Vec4fBinding&, const SlrShadingContext*, Dual2Vec4f& out);

    //-----------------------------------------------------

    struct InputBinding
    {
      SlrShader*  shader = nullptr;
      OutputPort* port = nullptr;

      //! Binding is connected to output sampler and valid.
      virtual bool      isBound() const = 0;

      //! Binding is connected to another shader.
      virtual bool      isConnected() const { return (isBound() && port != nullptr && shader != nullptr); }

      //! Knob type of binding
      virtual KnobType  type() const = 0;

      //! Returns true if the bound output port provides rgb color.
      inline bool hasRgb() const { return (port) ? port->hasRgb : false; }

      //! Returns true if the bound output port provides alpha.
      inline bool hasAlpha() const { return (port) ? port->hasAlpha : false; }

      //! Returns true if the bound output port provides presence (coverage.)
      inline bool hasPresence() const { return (port) ? port->hasPresence : false; }

      //! Returns true if the bound output port provides the channel.
      inline bool hasChannel(DD::Image::Channel c) const { return (port) ? port->hasChannel(c) : false; }

      //! Can the port data vary from sample to sample, or is it constant? Texture map values,
      //! for example, usually vary at each sample while a user-set multiplier control will
      //! usually be constant for all samples.
      inline bool isVarying() const { return (port && isBound()) ? port->isVarying : false; }

      //! Is port data constant? For example a fixed output parameter like image width, or height
      //! will always be constant and never vary per sample, while a color value could vary if
      //! it's coming from a texture map.
      inline bool isConstant() const { return !isVarying(); }
    };

    using InputBindingList = std::vector<InputBinding*>;


    struct PixelBinding : public InputBinding
    {
      PixelOutputSampler samplerFunc = uniformPixelSampler;

      bool isBound() const override { return (samplerFunc != uniformPixelSampler); }
      KnobType type() const override { return PIXEL_KNOB; }

      //! Gets all the channels requested of this binding, ignoring the ChannelSet in \a out.
      inline void get(const SlrShadingContext& stx, DD::Image::Pixel& out) const
        { samplerFunc(*this, &stx, port->requested, out); }

      //! Gets only the channels in ChannelSet \a get, ignoring the ChannelSet in \a out.
      inline void get(const SlrShadingContext& stx, const DD::Image::ChannelSet& get, DD::Image::Pixel& out) const
        { samplerFunc(*this, &stx, get, out); }
    };

    struct Vec4fBinding : public InputBinding
    {
      fdk::Vec4f         uniformVal;
      Vec4fOutputSampler samplerFunc = uniformVec4fSampler;

      Vec4fBinding(const fdk::Vec4f& v = fdk::Vec4f(0.0f)) : uniformVal(v) {}

      bool isBound() const override { return (samplerFunc != uniformVec4fSampler); }
      KnobType type() const override { return VEC4_KNOB; }

      inline fdk::Vec4f get(const SlrShadingContext& stx) const { return samplerFunc(*this, &stx); }
      inline fdk::Vec4f get() const { return samplerFunc(*this, nullptr); }
      inline operator fdk::Vec4f() const { return samplerFunc(*this, nullptr); }
    };

    struct Vec3fBinding : public InputBinding
    {
      fdk::Vec3f         uniformVal;
      Vec3fOutputSampler samplerFunc = uniformVec3fSampler;
      Vec4fBinding       vec4Binding;   //!< Used if bound output port is a VEC4/COLOR4 type

      Vec3fBinding(const fdk::Vec3f& v = fdk::Vec3f(0.0f)) : uniformVal(v) {}

      bool isBound() const override { return (samplerFunc != uniformVec3fSampler); }
      KnobType type() const override { return VEC3_KNOB; }

      inline fdk::Vec3f get(const SlrShadingContext& stx) const { return samplerFunc(*this, &stx); }
      inline fdk::Vec3f get() const { return samplerFunc(*this, nullptr); }
      inline operator fdk::Vec3f() const { return samplerFunc(*this, nullptr); }
    };

    struct Vec2fBinding : public InputBinding
    {
      fdk::Vec2f         uniformVal;
      Vec2fOutputSampler samplerFunc = uniformVec2fSampler;
      Vec3fBinding       vec3Binding;   //!< Used if bound output port is a VEC3/COLOR3 type
      Vec4fBinding       vec4Binding;   //!< Used if bound output port is a VEC4/COLOR4 type

      Vec2fBinding(const fdk::Vec2f& v = fdk::Vec2f(0.0f)) : uniformVal(v) {}

      bool isBound() const override { return (samplerFunc != uniformVec2fSampler); }
      KnobType type() const override { return VEC2_KNOB; }

      inline fdk::Vec2f get(const SlrShadingContext& stx) const { return samplerFunc(*this, &stx); }
      inline fdk::Vec2f get() const { return samplerFunc(*this, nullptr); }
      inline operator fdk::Vec2f() const { return samplerFunc(*this, nullptr); }
    };

    struct IntBinding : public InputBinding
    {
      int32_t          uniformVal;
      IntOutputSampler samplerFunc = uniformIntSampler;

      IntBinding(int32_t v = 0) : uniformVal(v) {}

      bool isBound() const override { return (samplerFunc != uniformIntSampler); }
      KnobType type() const override { return INT_KNOB; }

      inline int32_t get(const SlrShadingContext& stx) const { return samplerFunc(*this, &stx); }
      inline int32_t get() const { return samplerFunc(*this, nullptr); }
      inline operator int32_t() const { return samplerFunc(*this, nullptr); }
    };

    struct FloatBinding : public InputBinding
    {
      float              uniformVal;
      FloatOutputSampler samplerFunc = uniformFloatSampler;
      Vec2fBinding       vec2Binding;       //!< Used if bound output port is a VEC2_KNOB type
      Vec3fBinding       vec3Binding;       //!< Used if bound output port is a VEC3/COLOR3 type
      Vec4fBinding       vec4Binding;       //!< Used if bound output port is a VEC4/COLOR4 type
      PixelBinding       pixelBinding;      //!< Used if bound output port is a PIXEL_KNOB type
      int32_t            elementIndex = 0;  //!< Element index to sample in a larger binding's set of values
      DD::Image::ChannelSet sampleChans = DD::Image::Mask_None; //!< ChannelSet to sample in a PIXEL_KNOB's values

      FloatBinding(float v = 0.0f) : uniformVal(v) {}

      bool isBound() const override { return (samplerFunc != uniformFloatSampler); }
      KnobType type() const override { return FLOAT_KNOB; }

      inline float get(const SlrShadingContext& stx) const { return samplerFunc(*this, &stx); }
      inline float get() const { return samplerFunc(*this, nullptr); }
      inline operator float() const { return samplerFunc(*this, nullptr); }
    };

    struct DoubleBinding : public InputBinding
    {
      double              uniformVal;
      DoubleOutputSampler samplerFunc = uniformDoubleSampler;

      DoubleBinding(double v = 0.0) : uniformVal(v) {}

      bool isBound() const override { return (samplerFunc != uniformDoubleSampler); }
      KnobType type() const override { return DOUBLE_KNOB; }

      inline double get(const SlrShadingContext& stx) const { return samplerFunc(*this, &stx); }
      inline double get() const { return samplerFunc(*this, nullptr); }
      inline operator double() const { return samplerFunc(*this, nullptr); }
    };

    struct Mat4dBinding : public InputBinding
    {
      fdk::Mat4d         uniformVal;
      Mat4dOutputSampler samplerFunc = uniformMat4dSampler;

      bool isBound() const override { return (samplerFunc != uniformMat4dSampler); }
      KnobType type() const override { return MAT4_KNOB; }

      inline fdk::Mat4d get(const SlrShadingContext& stx) const { return samplerFunc(*this, &stx); }
      inline fdk::Mat4d get() const { return samplerFunc(*this, nullptr); }
      inline operator fdk::Mat4d() const { return samplerFunc(*this, nullptr); }
    };

    struct Color4Binding : public InputBinding
    {
      OutputPort*         opacityPort;      //!< Used if shared with color port
      fdk::Vec4f          uniformVal;
      Color4OutputSampler samplerFunc;      //!< Used if color & opacity are coming from same shader
      Vec3fBinding        colorBinding;     //!< Used if color & opacity are split
      FloatBinding        opacityBinding;   //!< Used if color & opacity are split

      bool isBound() const override;
      KnobType type() const override { return COLOR4_KNOB; }

      //! Returns true if the bound opacity output port provides alpha.
      inline bool hasAlpha() const { return (opacityPort) ? opacityPort->hasAlpha : false; }

      fdk::Vec4f get(const SlrShadingContext& stx) const;
      fdk::Vec4f get() const;
      operator fdk::Vec4f() const;

      Color4Binding() : opacityPort(nullptr), samplerFunc(uniformColor4Sampler) {}
      Color4Binding(const fdk::Vec4f& v)
        : opacityPort(nullptr)
        , samplerFunc(uniformColor4Sampler)
        , colorBinding(v.xyz())
        , opacityBinding(v.w) {}
    };

    struct Dual2fBinding : public FloatBinding
    {
      Dual2fOutputSampler dual2SamplerFunc = uniformDual2fSampler;

      Dual2fBinding(float v = 0.0) : FloatBinding(v) {}

      bool isBound() const override { return (dual2SamplerFunc != uniformDual2fSampler); }
      KnobType type() const override { return FLOAT_KNOB; }

      inline void get(const SlrShadingContext& stx, Dual2f& v) const { dual2SamplerFunc(*this, &stx, v); }
      inline void get(Dual2f& v) const { return dual2SamplerFunc(*this, nullptr, v); }
    };

    struct Dual2dBinding : public DoubleBinding
    {
      Dual2dOutputSampler dual2SamplerFunc = uniformDual2dSampler;

      Dual2dBinding(double v = 0.0) : DoubleBinding(v) {}

      bool isBound() const override { return (dual2SamplerFunc != uniformDual2dSampler); }
      KnobType type() const override { return DOUBLE_KNOB; }

      inline void get(const SlrShadingContext& stx, Dual2d& v) const { dual2SamplerFunc(*this, &stx, v); }
      inline void get(Dual2d& v) const { return dual2SamplerFunc(*this, nullptr, v); }
    };

    struct Dual2Vec2fBinding : public Vec2fBinding
    {
      Dual2Vec2fOutputSampler dual2SamplerFunc = uniformDual2Vec2fSampler;

      Dual2Vec2fBinding(const fdk::Vec2f& v = fdk::Vec2f(0.0f)) : Vec2fBinding(v) {}

      bool isBound() const override { return (dual2SamplerFunc != uniformDual2Vec2fSampler); }
      KnobType type() const override { return VEC2_KNOB; }

      inline void get(const SlrShadingContext& stx, Dual2Vec2f& v) const { dual2SamplerFunc(*this, &stx, v); }
      inline void get(Dual2Vec2f& v) const { return dual2SamplerFunc(*this, nullptr, v); }
    };

    struct Dual2Vec3fBinding : public Vec3fBinding
    {
      Dual2Vec3fOutputSampler dual2SamplerFunc = uniformDual2Vec3fSampler;

      Dual2Vec3fBinding(const fdk::Vec3f& v = fdk::Vec3f(0.0f)) : Vec3fBinding(v) {}

      bool isBound() const override { return (dual2SamplerFunc != uniformDual2Vec3fSampler); }
      KnobType type() const override { return VEC3_KNOB; }

      inline void get(const SlrShadingContext& stx, Dual2Vec3f& v) const { dual2SamplerFunc(*this, &stx, v); }
      inline void get(Dual2Vec3f& v) const { return dual2SamplerFunc(*this, nullptr, v); }
    };

    struct Dual2Vec4fBinding : public Vec4fBinding
    {
      Dual2Vec4fOutputSampler dual2SamplerFunc = uniformDual2Vec4fSampler;

      Dual2Vec4fBinding(const fdk::Vec4f& v = fdk::Vec4f(0.0f)) : Vec4fBinding(v) {}

      bool isBound() const override { return (dual2SamplerFunc != uniformDual2Vec4fSampler); }
      KnobType type() const override { return VEC4_KNOB; }

      inline void get(const SlrShadingContext& stx, Dual2Vec4f& v) const { dual2SamplerFunc(*this, &stx, v); }
      inline void get(Dual2Vec4f& v) const { return dual2SamplerFunc(*this, nullptr, v); }
    };

    //-----------------------------------------------------



  private:
    //! Disabled copy constructor.
    SlrShader(const SlrShader&) = delete;
    //! Disabled assignment operator.
    SlrShader& operator = (const SlrShader&) = delete;


  public:
    static constexpr std::string_view pluginCategory = "SlrShaders";    //!< Category can be used to organize plugins within a subdirectory.
    static constexpr std::string_view pluginPrefix   = "slr";           //!< The name to use as the plugin name prefix.
    static constexpr std::string_view pluginSuffix   = ""/*"Shader"*/;  //!< The name to use as the plugin name suffix.

    //-----------------------------------------------------------------
    /*! This structure creates a subclass of SlrShader, possibly from loading a plugin file
        or a compiled-in Description.
    */
    class DDImage_API Description : public DD::Image::PluginObject::Description
    {
    public:
      //! Constructor method definitions used for 'build()' methods in plugins.
      typedef SlrShader* (*ShaderBuilder)(SlrEngineContext*);
      ShaderBuilder builderFunc;  //!< Call this to construct an SlrShader object.

    public:
      //! SlrShader specific builder.
      Description(const char*   shaderClassName,
                  ShaderBuilder shaderBuilder);

      //! Prefix and suffix strings prepended and appended to pluginClass() string to
      //! construct plugin filenames.
      const std::string_view&  classPrefix() const override { return SlrShader::pluginPrefix; }
      const std::string_view&  classSuffix() const override { return SlrShader::pluginSuffix; }

      //! Name of the SlrShader class without any leading prefix or or trailing suffix.
      const char*  shaderClass() const { return pluginClass().data(); }

      //! Full name of plugin with prefix and suffix.
      const std::string&  shaderPluginName() const { return _pluginName; }
    };
    //-----------------------------------------------------------------


    using KnobNameMap = std::unordered_map<std::string, uint32_t>;


  protected:
    friend class RenderEngine;
    SlrEngineContext*      _slrtx;                  //!< Renderer owner
    int32_t                _identifierIndex;        //!< Renderer-assigned unique index Id in a linear array of shaders
    std::string            _path;                   //!< Shader path in material group
    bool                   _debug;                  //!< 
    int32_t                _renderVersion;          //!< Update version incremented in updateProperties()
    int32_t                _validateVersion;        //!< Update version incremented in validateShader()
    int32_t                _requestVersion;         //!< Request version incremented in requestResources(()
    //
    InputKnobList          _inputs;                 //!< Input knobs, copied and updated from the static list
    KnobNameMap            _inputNameMap;           //!< Name to input index map
    OutputPortList         _outputs;                //!< Output knobs, copied from the static list
    KnobNameMap            _outputNameMap;          //!< Name to output index map
    //
    ConnectionList         _inputConnections;       //!< Shaders bound to input knobs
    ShadingLayerReqSet     _geomPrimvarsReq;        //!< Names of primvars we need
    //
    ShadingLayerReqSet     _requestedGeometryPrimvars;  //!< Geometry primvar attributes being requested
    ShadingLayerReq        _parametricStLayer;          //!< Layer name to use for parametric st coordinates.
    //
    ShadingLayerReqSet     _writableShadingLayers;      //!< Set of shading layers the shader wants to read or write to
    //
    DD::Image::ChannelSet  _inputTextureColorChannels;  //!< Set of color channels output by all input texture bindings that this shader needs
    DD::Image::ChannelSet  _outputColorChannels;        //!< Set of all output color (blendable) channels this shader produces
    DD::Image::ChannelSet  _outputDataChannels;         //!< Set of all output data (non-blendable) channels this shader produces
    DD::Image::ChannelSet  _requestedChannels;          //!< Intersected set of output channels and all requested channels
    DD::Image::Format      _outputFormat;               //!< Nominal Format if the surface shader represents an image/texture
    //
    DD::Image::ChannelPack _outputColorChannelPack; //!< Automatically built from _outputColorChannels
    DD::Image::ChannelPack _outputDataChannelPack;  //!< Automatically built from _outputDataChannels

    //! Subclass portion of validateShader().
    //! Default implementation does nothing.
    virtual void  _validateShader();

    //! Subclass portion of configureOutputPort().
    //! Because the subclass is called after the base port info is configured the
    //! shader subclass can overwrite the port info completely, so be careful.
    //! Default implementation does nothing.
    virtual void  _configureOutputPort(OutputPort& port, SlrShader* connectingShader);

    //! Subclass portion of requestResources().
    //! Default implementation does nothing.
    virtual void  _requestResources(const DD::Image::ChannelSet& request_channels);

    //! Subclass portion of initializeForShadingThread().
    //!
    //! Initialize any custom shader data in preparation for doing shading on engine thread
    //! \a sttx. \a threadShaderData is a struct stored in the thread safe SlrThreadContext
    //! and solely owned by that thread, so an SlrShader should allocate a locally defined
    //! object (data blob) and configure \a threadShaderData. When the shading thread is
    //! eventually destroyed it will call the ThreadShaderData::deleteFunc for each shader.
    //!
    //! Default implementation does nothing.
    virtual void  _initializeForShadingThread(const SlrThreadContext& sttx,
                                              SlrShaderThreadData&    threadShaderData);

    //! Subclass portion of cleanupAfterShading().
    //! Default implementation does nothing.
    virtual void  _cleanupAfterShading();


  public:
    //!
    SlrShader(SlrEngineContext*     slrtx,
              const InputKnobList&  inputs,
              const OutputPortList& outputs,
              void*                 inputs_storage_data = nullptr,
              size_t                inputs_storage_size = 0);

    //! Must have a virtual destructor.
    virtual ~SlrShader();

    //! Returns the class name, must implement.
    virtual const char*  shaderClass() const = 0;

    //! Global render context, the owner of the SlrShader instance.
    //! Use this to get access to global render info.
    SlrEngineContext&  globalRenderContext() const;

    //! Renderer assigned unique index Id in a linear array of shaders. Usually used to store
    //! private shader data in the engine shading thread context.
    int32_t  identifierIndex() const { return _identifierIndex; }

    /*! Create a SlrShader instance from a shader class name, an existing shader description,
        or a ShaderDesc.
        Calling code takes ownership of returned pointer.
    */
    static SlrShader*  create(SlrEngineContext*, const SlrShader::Description& shader_description);
    static SlrShader*  create(SlrEngineContext*, const std::string_view& shader_class);
    static SlrShader*  create(SlrEngineContext*, const usg::ShaderDesc* shader_desc);

    //! Find plugins matching this class' prefix and suffix. See PluginObject::FindPlugins() for more info.
    static const fdk::StringList& FindPlugins(bool findBuiltins,
                                              bool findExternals,
                                              bool forceRefresh = false);

    //!
    virtual SlrSurfaceShader*  asSurfaceShader()  { return nullptr; }
    //!
    virtual SlrCoShader*       asCoShader()       { return nullptr; }
    //!
    virtual SlrGeometryShader* asGeometryShader() { return nullptr; }
    //!
    virtual SlrLightShader*    asLightShader()    { return nullptr; }
    //!
    virtual SlrVolumeShader*   asVolumeShader()   { return nullptr; }
    //!
    virtual SlrLensShader*     asLensShader()     { return nullptr; }


  public:
    //---------------------------------------------------------
    //---------------------------------------------------------

    //! Return the shader path if assigned. Used mainly for debugging.
    const std::string&  getPath() const { return _path; }
    //! Assign a shader path. Used mainly for debugging.
    void                setPath(const std::string& path) { _path = path; }

    //! 
    int32_t  getRenderVersion() const { return _renderVersion; }
    //!
    void     setRenderVersion(const int32_t& v) { _renderVersion = v; }

    //! Return a static list of input knobs for this shader. Base class returns an empty list.
    virtual const InputKnobList&   getInputKnobDefinitions() const;

    //! Return a static list of output knobs for this shader. Base class returns only the 'primary' output.
    virtual const OutputPortList&  getOutputPortDefinitions() const;


    //---------------------------------------------------------
    //
    //---------------------------------------------------------


    //! Returns the number of input knobs.
    uint32_t     numInputs()  const { return (uint32_t)_inputs.size();  }
    //! Returns the number of output knobs.
    uint32_t     numOutputs() const { return (uint32_t)_outputs.size(); }

    //! Returns the input index of the 'base' (or 'default' or 'background') input
    //! for the shader. This is the input that configuration info flows down, such
    //! as format info, channel sets or global trait masks.
    //!
    //! Typically only shaders that do merging or switching operations, or surface
    //! shaders need to change the default.
    //!
    //! Default returns index 0.
    virtual int32_t baseInputIndex() const { return 0; }

    //! Returns the shader, if any, on input baseInputIndex(). Convenience, same as
    //! getting the Connection for the input and returning its source shader.
    const SlrShader*  baseInputShader() const;

    //! Get the list of input connections.
    const ConnectionList&  inputConnections() const { return _inputConnections; }

    //! Returns the input connection if index is in the valid range.
    const Connection*  inputConnection(uint32_t inputIndex) const;

    //! Returns true if the InputKnob type and OutputPort type are compatible.
    static bool  isConnectionTypeCompatible(const InputKnob&  input_knob,
                                            const OutputPort& output_port);

    //!
    bool         connectInput(const std::string& input_name,
                              SlrShader*         source_shader,
                              int32_t            output_port_index);
    bool         connectInput(const std::string& input_name,
                              SlrShader*         source_shader,
                              const std::string& output_port_name);
    bool         connectInput(const std::string& input_name,
                              const Connection&  connection);
    //!
    bool         connectInputs(const ConnectionMap& connections);


    //! Return the connection for the named input, otherwise an invalid one is returned.
    const Connection&  getConnection(const std::string& input_name) const;

    /*! Convenience to get a SlrShader input.
        Same as calling 'getConnection(input_name)->source_shader'.
    */
    SlrShader*         getShaderForInput(const std::string& input_name) const;


    //---------------------------------------------------------

    //! Returns input knob or null if not available.
    InputKnob*   getInputKnob(uint32_t input) const;
    InputKnob*   getInputKnob(const std::string& input_name) const;

    //! Return a named input's index or -1 if not found.
    int32_t      getInputIndex(const std::string& input_name) const;


    //! Returns output knob or null if not available.
    OutputPort*  getOutputPort(uint32_t output) const;
    OutputPort*  getOutputPort(const std::string& output_name) const;

    //! Return a named output's index or -1 if not found.
    int32_t      getOutputIndex(const std::string& output_name) const;



    /*! Initialize an input binding by retrieving the named input property from the
        passed-in ShaderDesc and assigning the appropriate sampler function.

        If the ShaderDesc has the named input property then its default value
        is copied to the binding's 'uniformVal'. If the input property is connected
        to another shader the binding's 'samplerFunc' is set to the other shader's
        OutputPort 'samplerFunc' pointer. If the pointer is null then the uniform
        sampler for this type is assigned which will return the binding's
        'uniformVal'.

        If the ShaderDesc does not have the named input property the passed-in
        'fallback' value is copied to the binding's 'uniformVal' and the sampler
        set to the uniform sampler func for this type.
    */
    IntBinding        bindIntShaderDescInput(const usg::ShaderDesc&, const std::string& input_name, int32_t fallback = 0) const;
    DoubleBinding  bindDoubleShaderDescInput(const usg::ShaderDesc&, const std::string& input_name, double  fallback = 0.0) const;
    //
    FloatBinding    bindFloatShaderDescInput(const usg::ShaderDesc&, const std::string& input_name, float fallback = 0.0f) const;
    FloatBinding    bindFloatShaderDescInput(const usg::ShaderDesc&, const std::string& input_name, DD::Image::Channel sample_chan, float fallback = 0.0f) const;
    //
    Vec2fBinding    bindVec2fShaderDescInput(const usg::ShaderDesc&, const std::string& input_name, const fdk::Vec2f& fallback = fdk::Vec2f(0.0f)) const;
    Vec3fBinding    bindVec3fShaderDescInput(const usg::ShaderDesc&, const std::string& input_name, const fdk::Vec3f& fallback = fdk::Vec3f(0.0f)) const;
    Vec4fBinding    bindVec4fShaderDescInput(const usg::ShaderDesc&, const std::string& input_name, const fdk::Vec4f& fallback = fdk::Vec4f(0.0f)) const;
    //
    Mat4dBinding    bindMat4dShaderDescInput(const usg::ShaderDesc&, const std::string& input_name, const fdk::Mat4d& fallback = fdk::Mat4d::getIdentity()) const;
    //
    //! Specialized double bind for color and opacity coming from the same source shader, a very common case.
    Color4Binding   bindColor4ShaderDescInput(const usg::ShaderDesc& shader_desc,
                                              const std::string&     color_input_name,
                                              const std::string&     opacity_input_name,
                                              const fdk::Vec4f&      fallback = fdk::Vec4f(1.0f)) const;
    //!
    PixelBinding    bindPixelShaderDescInput(const usg::ShaderDesc&, const std::string& input_name) const;

    /*! Create a secondary Dual2 binding to a valid, existing binding. Requires the name of the
        Dual2 output port on the source shader. Typically if an output is named 'result' then
        the Dual2 version of that output is named 'resultDual2'.
    */
    Dual2fBinding     bindDual2fInput(const FloatBinding&, const std::string& dual2_output_name) const;
    Dual2dBinding     bindDual2dInput(const DoubleBinding&, const std::string& dual2_output_name) const;
    Dual2Vec2fBinding bindDual2Vec2fInput(const Vec2fBinding& in, const std::string& dual2_output_name) const;
    Dual2Vec3fBinding bindDual2Vec3fInput(const Vec3fBinding& in, const std::string& dual2_output_name) const;
    Dual2Vec4fBinding bindDual2Vec4fInput(const Vec4fBinding& in, const std::string& dual2_output_name) const;


    /*! Update an input binding's sampler function if there's a valid connection on the
        named input. Returns true if there's a valid connection to another shader and
        the binding is bound to it.

        Retrieves and assigns the correct value sampler to use depending if the InputKnob
        is connected to another shader's OutputPort. If the input is not connected or the
        connected OutputPort's 'samplerFunc' pointer is null then the uniform sampler
        is assigned which returns the binding's 'uniformVal' var, and false is returned
        from this method.

        The binding's 'uniformVal' can be directly set or normally it's initialized using
        the 'bind<type>ShaderDescInput()' method which accepts an optional fallback value
        if the ShaderDesc does not have that property.
    */
    bool     connectIntInputBinding(IntBinding&,    const std::string& input_name) const;
    bool   connectFloatInputBinding(FloatBinding&,  const std::string& input_name, DD::Image::Channel sample_chan = DD::Image::Chan_Black) const;
    bool  connectDoubleInputBinding(DoubleBinding&, const std::string& input_name) const;
    bool   connectVec2fInputBinding(Vec2fBinding&,  const std::string& input_name) const;
    bool   connectVec3fInputBinding(Vec3fBinding&,  const std::string& input_name) const;
    bool   connectVec4fInputBinding(Vec4fBinding&,  const std::string& input_name) const;
    bool   connectMat4dInputBinding(Mat4dBinding&,  const std::string& input_name) const;
    bool   connectPixelInputBinding(PixelBinding&,  const std::string& input_name) const;


    /*! Return the channels output by the output port.

        By default the port's type is used to determine the channels. Only types that are
        explicitly color types return a valid channel set. So COLOR3_KNOB returns Mask_RGB
        while COLOR4_KNOB returns Mask_RGBA, COLOR_R_KNOB returns Mask_Red, COLOR_A_KNOB
        returns Mask_Alpha, etc.

        Single value types like INT, FLOAT and DOUBLE, and vector/matrix types all
        return Mask_None as they are general data, and not explicitly 'colors'.

        PIXEL is handled special - all the SlrShader's output channels are returned,
        including any data aovs. A shader subclass should override this method to correctly
        define the set if this is not wanted.

        Output types which don't naturally translate to a fixed set of Channels return
        Mask_None, such as STRING, MAT4, INTARRAY, FLOATARRAY, DOUBLEARRAY, etc.
    */
    virtual DD::Image::ChannelSet  getOutputPortChannels(const OutputPort& output_port) const;


  public:
    //---------------------------------------------------------
    //---------------------------------------------------------


    /*! Copy/update property values from a usg::ShaderDesc shader schema source.
        Must implement.
    */
    virtual void  updateProperties(const usg::ShaderDesc& shader_desc) = 0;

    /*! Initialize any vars prior to rendering. This is called single-threaded
        by the Renderer during Iop::_validate().

        A typical SlrShader will take input variables and do any costly pre-calcs
        here to save on per-shade cost. Don't bother storing anything that may
        change on a per-sample basis!

        Base class calls validateShader() on the set of input shaders and
        combines the output channel sets, and primvar and shading layer
        requests from all inputs, and then it calls the virtual _validateShader()
        for the subclass to do any additional work.
    */
    void  validateShader();

    /*! Allows custom output port configurations to be done on a shader subclass, for example
        setting whether a port has certain channels active or the value can vary.

        Note - this is called from the -downstream- \a connectingShader validateShader(), NOT
        -this- shader's validateShader(). This shader will have already been validated its
        state is up to date.

        Base class gets the port's channel set from getOutputPortChannels() and updates the
        port's base state (hasAlpha, hasRGB, etc) then calls _configureOutputPort() to
        let subclass set any further port values, like isVarying. Because the subclass is
        called last it can overwrite the port info completely, so be careful.
    */
    void  configureOutputPort(OutputPort& port, SlrShader* connectingShader);

    /*! Request the resources this shader will want to use during shading. For
        example the texture-Iop samplers used when input DD::Image::Iop sources
        need to be directly sampled, and the Channel indices used for sampling
        primvar attributes.

        This is called single-threaded by the Renderer during Iop::_request(),
        in preparation for the threaded Iop engine() calls when shading is done.

        The shader should request an Iop sampler by calling
        globalRenderContext()->requestTextureSampler() and passing the input Iop*
        and channels that will be read from it. Since there can be multiple shaders
        accessing the same source Iop* with different channel sets, all the shader
        requests are collated together before the DD::Image::Sampler objects are
        created by the Renderer, so the created Sampler will contain the union of
        all requested channels.

        To complete the request process the shader should retrieve the created
        DD::Image::Samplers in _initializeForShadingThread() using
        globalRenderContext()->getTextureSampler(Iop*), and store them locally
        so they're available during the threaded shading methods.

        Base class calls requestResources() on the set of input shaders, then
        calls the virtual _requestResources() for the subclass to do its work.
    */
    void  requestResources(const DD::Image::ChannelSet& request_channels);

    //---------------------------------------------------------

    /*! Initialize any custom shader data in preparation for doing shading on engine thread
        \a sttx. \a customThreadData is a pointer stored in the thread safe SlrThreadContext
        and is owned solely by that thread, so an SlrShader should allocate a locally
        defined struct and set \a threadShaderData to it. When the shading thread is
        eventually destroyed it will call the ThreadShaderData::deleteFunc for each shader.

        At this point any Iop texture samplers requested in requestResources() can
        be created by the Renderer and the shader can make local copies of the
        DD::Image::Sampler pointers by querying slrtx.getTextureSampler(iop).

        All needed input primvar attributes called out in _validateShader() (added to the
        _requestedGeometryPrimvars set) are now assigned Channel indices in shading channel
        manager and can be retrieved via globalRenderContext().retrievePrimvarLayer(<primvar_name>)
        method and stored locally on the shader for speedy data access in the shading
        calls. Note that <primvar_name> is the name of the primitive attribute -without-
        the leading 'primvars:' namespace.

        Base class calls initializeForShadingThread() on the set of input shaders then
        calls the virtual _initializeForShadingThread() for the subclass to do its work.
    */
    void  initializeForShadingThread(const SlrThreadContext& sttx);

    //! 
//    void  

    /*! Called when the render is complete and the shader can delete any temp
        allocations done in initializeForShadingThread().

        The Renderer calls this on Iop::_close() which clears out render prims,
        texture samplers, etc. The Renderer will do another
        validate/request/initialize sequence before the shaders are called
        again to shade anything.
 
        Base class calls cleanupAfterShading() on the set of input shaders then
        calls the virtual _cleanupAfterShading() for the subclass to do its work.
    */
    void  cleanupAfterShading();


    //---------------------------------------------------------
    // Resources
    //---------------------------------------------------------

    /*! Return the opacity traits for the renderable influenced by this shader. This
        method will be called for each renderable Gprim a surface shader is assigned to,
        and the surface shader can then optionally query its input shaders for their
        traits.

        By default this returns the opaqueSolid() OpacityTraitsMask.

        If a shader requires renderable specific primvar values to determine opacity
        then it can call SlrEngineContext::gprimProvidesPrimvar() to find if a primvar
        is available for that renderable and its surface interpolation type.
    */
    virtual ndk::OpacityTraitsMask  getRenderableOpacityTraits(const SlrPrimInfo& primInfo) const { return ndk::opaqueSolid(); }

    //! Shader turns on cutout mode, which is passed down to the surface shader and
    //! applied by the integrator.
    //! When this is true the integrator should not fill the color channelsfor this surface
    //! and leave them empty (zeroed).
    //! Default is false.
    virtual bool  cutoutModeEnabled() const { return false; }

    //! Shader turns on cutout data aovs mode, which is passed down to the surface shader
    //! and applied by the integrator.
    //! When this is true the integrator should not fill the data avos for this surface
    //! and leave them empty (zeroed).
    //! Default is false.
    virtual bool  cutoutModeRemovesDataAovsEnabled() const { return false; }

    /*! Return the set of named primvars this shader wants to read from the geometry.

        Note that even though the shader has requested a primvar that does not mean it will
        be available on the GPrim the shader is bound to, and only primvar names that are
        available will be added to the shading layer set.

        Retrieval of the shading layer channels should be done in requestResources()
        via globalRenderContext().retrievePrimvarLayer(<primvar_name>). Note that
        <primvar_name> is the name of the primitive attribute -without- the leading
        'primvars:' namespace.

        If this shader wants to read or write to an arbitrary shading layer then request
        those by implementing requestedWritableShadingLayers().

        Default implementation returns the _geomPrimvarsReq primvar request set built in
        _validateShader().
    */
    virtual const ShadingLayerReqSet&  requestedGeometryPrimvars() const;

    /*! Return the primary 'st' primvar name for this shader that is used to do parametric
        surface evaluation. Note that this is usually the same primvar that's used for
        texture coordinates, but there can be different texture primvars used in each
        texture sampler, so it's not obvious what is the 'primary' parametric primvar.

        Texture shaders should set this to the user's desired texture primvar if there's
        a valid texture binding. Then a surface shader will typically then get the primvar
        name from its diffuse/albedo input, before falling back to other inputs like emission.

        Default implementation returns an empty value.
    */
    virtual ShadingLayerReq  parametricStPrimvar() const { return ShadingLayerReq(); }

    /*! Return the set of arbitrary shading layers this shader wants to read or write to.
        This set should be constructed in _validateShader(), it is cleared at the start
        of each validate cycle.

        Each layer/channel name follows Nuke channel notation with optional layer index suffix
        '<layer>.<channel>:<index>'. For example to create a new layer with xyz channels you
        would add strings 'MyShadingLayer.x:0', 'MyShadingLayer.y:1', and 'MyShadingLayer.z:2.
        If the layer index suffix is omitted then channels named 'x', 'y', 'z' will be assigned
        layer index 0, 1, and 2. The layer channels orders are important so that values are packed
        in the channel array correctly. You don't want the value of channel index 'MyShadingLayer.z'
        being read out into MyShadingVec3.x.

        Important - these layers are limited to four(4) channels in size! ie Vec4s - and any
        named channels past the 4th with the same layer name will be ignored. For example a shader
        wanting to pass a Mat4 via shading layers will need to make four separate Vec4 layers. Note
        this limitation is just for the shading channels passed via SlrShadingContext, shader input
        bindings are not so limited.

        Also note that these are -arbitrary- channels much like DD::Image::Channels, but are local
        to the shading system and do not affect Nuke's global channel system. A shader accesses
        these vars on the SlrShadingContext that is passed up the shading graph and can modify
        this context before passing it on by making an editable SlrShadingContext copy via 
        SlrShadingContext::createEditableShadingContextCopy().

        Default implementation returns an empty set.
    */
    virtual const ShadingLayerReqSet&  requestedWritableShadingLayers() const;

    //--------------------------------

    /*! Return the total set of primvars this shader is requesting from the geometry,
        including primvar requests from all shader inputs.

        This set is built in validateShader() from all the shader's inputs just before
        this shader's _validateShader() method is called, so all inputs will have this
        value up to date.
    */
    const ShadingLayerReqSet&      inputPrimvars() const { return _requestedGeometryPrimvars; }

    /*! Return the total set of writable shading layers this shader is requesting, including
        shading layer requests from all shader inputs.

        This set is built in validateShader() from all the shader's inputs just before
        this shader's _validateShader() method is called, so all inputs will have this
        value up to date.
    */
    const ShadingLayerReqSet&      writableShadingLayers() const { return _writableShadingLayers; }

    /*! Return the color channels for all the texture sources this shader intends to use.

        This set is built in validateShader() from all the shader's color inputs (COLOR3_KNOB, etc)
        just before this shader's _validateShader() method is called, so all inputs will have this
        value up to date and can be extended/changed in _validateShader().
    */
    const DD::Image::ChannelSet&   inputTextureColorChannels() const { return _inputTextureColorChannels; }

    /*! Return the color channels (including alphas and masks) output by this shader.
        This set is built in validateShader() from all the shader's color (non-data) inputs.
        ie COLOR3_KNOB, COLOR_R_KNOB, etc.
    */
    const DD::Image::ChannelSet&   outputColorChannels() const { return _outputColorChannels; }
    //! Fast-access ChannelPack version of outputColorChannels() for use in varying shader code.
    const DD::Image::ChannelPack&  outputColorChannelPack() const { return _outputColorChannelPack; }

    /*! Return the data (non-color) channels output by this shader such as normals, IDs, etc.
        This set is built in validateShader() from all the shader's data (non-color) inputs.
        ie VEC2_KNOB, FLOAT_KNOB, etc.
    */
    const DD::Image::ChannelSet&   outputDataChannels() const { return _outputDataChannels; }
    //! Fast-access ChannelPack version of outputDataChannels() for use in varying shader code.
    const DD::Image::ChannelPack&  outputDataChannelPack() const { return _outputDataChannelPack; }

    //! Return the nominal Format if the surface shader represents an image/texture.
    const DD::Image::Format&       outputFormat() const { return _outputFormat; }

    //! Intersected set of shader's output channels and all requested channels from possibly multiple
    //! calls to requestResources().
    const DD::Image::ChannelSet&   requestedChannels() const { return _requestedChannels; }


  public:
    //!
    void print(const char*   prefix,
               std::ostream& o) const;
    friend std::ostream& operator << (std::ostream& o, const SlrShader& b) { b.print("", o); return o; }


  }; // SlrShader

  using SlrShaderList = std::vector<SlrShader*>;
  using SlrShaderMap  = std::unordered_map<std::string, SlrShader*>;

  DDImage_API std::ostream& operator << (std::ostream& o, const SlrShader::InputBinding& b);


  //-----------------------------------------------------------------------------
  //-----------------------------------------------------------------------------


  /*! A list of SlrShader pointers and the group's output.

      The pointers are *not* managed by this class but there's convenience
      methods to help delete them and clear out the group.
  */
  template <class ShaderT>
  struct SlrShaderGroup
  {
    SlrShaderMap shaders;       //!< Shaders with name mapping for connections
    ShaderT*     output;        //!< Shader graph entry point
    //
    fdk::Hash  topologyHash;    //!< Does the shader graph need to be rebuilt?
    fdk::Hash  propertyHash;    //!< Do the shaders need parameter updating?


    SlrShaderGroup();

    size_t  size() const { return shaders.size(); }
    bool    empty() const { return shaders.empty(); }
    bool    isValid() const { return (output && !empty()); }

    //! Return the named shader, or null if not in group.
    SlrShader*  getShaderNode(const std::string& shader_node_name) const;

    //! Convenience function to delete shader pointers and empty the group.
    void        deleteShaders();

  };


  // Predeclare the supported shader group specializations:
  using SlrCoShaderGroup       = SlrShaderGroup<SlrCoShader>;
  using SlrSurfaceShaderGroup  = SlrShaderGroup<SlrSurfaceShader>;
  using SlrGeometryShaderGroup = SlrShaderGroup<SlrGeometryShader>;
  using SlrLightShaderGroup    = SlrShaderGroup<SlrLightShader>;
  using SlrVolumeShaderGroup   = SlrShaderGroup<SlrVolumeShader>;
  using SlrLensShaderGroup     = SlrShaderGroup<SlrLensShader>;


  //------------------------------------------------------------------------


  /*! Extension interface that adds cutout knob controls and the mappings
      to the shader parameters.
      "cutout_mode_enable" Int
      "cutout_mode_removes_data_aovs" Int
  */
  class DDImage_API SlrShaderCutoutControlsI
  {
  public:
    bool  cutoutModeControl() const { return (_cutoutModeBinding.get() > 0); }
    bool  cutoutModeRemovesDataAovsControl() const { return (_cutoutModeRemovesDataAovsBinding.get() > 0); }

    //! Retrieve shader cutout properties from \a shaderDesc and store in local vars.
    //! Call this from the ownerSlrShader::updateProperties() method.
    virtual void  cutoutModesRetrieveProperties(SlrShader* ownerSlrShader, const usg::ShaderDesc& shaderDesc);


    //! Update the local cutout state. Call this from the ownerSlrShader::_validateShader() method.
    //! If \a ownerSlrShader has a valid input on baseInputIndex() the opacity traits from that
    //! input are automatically copied and the local shader properties then applied as overrides.
    //! Note that the local properties will only -enable- the modes, never disable them if the
    //! input has them enabled.
    virtual void  cutoutModesValidate(SlrShader* ownerSlrShader);


  protected:
    SlrShader::IntBinding _cutoutModeBinding;                 //!< Surface acts as a cutout/holdout/matte
    SlrShader::IntBinding _cutoutModeRemovesDataAovsBinding;  //!< If cutout surface enabled, does depth & aovs get cut as well

    bool _cutoutModeEnabled = false;                //!< If true enable cutout mode
    bool _cutoutModeRemovesDataAovsEnabled = false; //!< If cutout mode is on, also cutout data aovs (depth, normals, etc)
  };


//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------


template <class ShaderT>
inline
SlrShaderGroup<ShaderT>::SlrShaderGroup() :
  output(nullptr)
{
  //
}

template <class ShaderT>
inline SlrShader*
SlrShaderGroup<ShaderT>::getShaderNode(const std::string& shader_node_name) const
{
  const typename SlrShaderMap::const_iterator it = shaders.find(shader_node_name);
  return (it != shaders.end()) ? it->second : nullptr;
}

template <class ShaderT>
inline void
SlrShaderGroup<ShaderT>::deleteShaders()
{
  for (typename std::unordered_map<std::string, SlrShader*>::iterator it=shaders.begin(); it != shaders.end(); ++it)
  {
    delete it->second;
  }
  shaders.clear();
  output = nullptr;
  topologyHash.reset();
  propertyHash.reset();
}

//----------------------------------------------------

inline bool
SlrShader::Color4Binding::isBound() const
{
  if (shader) {
    return (samplerFunc != uniformColor4Sampler); // color4 takes precedence
  }
  return (colorBinding.isBound() && opacityBinding.isBound());
}

inline fdk::Vec4f
SlrShader::Color4Binding::get(const SlrShadingContext& stx) const {
  if (shader) {
    return samplerFunc(*this, &stx); // color4 takes precedence
  }
  return fdk::Vec4f(colorBinding.samplerFunc(colorBinding, &stx),
                    opacityBinding.samplerFunc(opacityBinding, &stx));
}

inline fdk::Vec4f
SlrShader::Color4Binding::get() const {
  if (shader) {
    return samplerFunc(*this, nullptr); // color4 takes precedence
  }
  return fdk::Vec4f(colorBinding.samplerFunc(colorBinding, nullptr),
                    opacityBinding.samplerFunc(opacityBinding, nullptr));
}

inline SlrShader::Color4Binding::operator fdk::Vec4f() const
{
  return this->get();
}

} // namespace slr


#endif
