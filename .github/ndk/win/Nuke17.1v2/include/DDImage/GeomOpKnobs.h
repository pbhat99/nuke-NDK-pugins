// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef DDIMAGE_GEOMOPKNOBS_H
#define DDIMAGE_GEOMOPKNOBS_H

#include <string_view>

#include "DDImage/GeomOp.h"
#include "DDImage/ArrayKnobI.h" // for ValueProvider
#include "DDImage/StringKnobI.h"

#include "usg/geom/XformablePrim.h"

namespace DD
{
  namespace Image
  {
    
    //-----------------------------------------------------------------

    //! Helper class for a Knob-stored value with authoring options.
    template <typename T>
    class AuthoredValue : public Authoring
                        , public ValueProvider
    {
    public:
      T value;          //!< Current authored value
      T schemaValue;    //!< Fallback value if not authored

      AuthoredValue(const AuthoringSettings& settings = AuthoringSetNoChangeDefaults()); // defaults to 0 or ""
      AuthoredValue(const T& val, const AuthoringSettings& settings = AuthoringSetNoChangeDefaults()); // schemaVal is same as val
      AuthoredValue(const T& val, const T& schemaVal, const AuthoringSettings& settings = AuthoringSetNoChangeDefaults());
      AuthoredValue(const T& val, const T& schemaVal, const char* attribName, Mode mode = Mode::NoChange, Mode defaultMode = Mode::NoChange);
      AuthoredValue(const T& val, const T& schemaVal, const usg::Token& attribName, Mode mode = Mode::NoChange, Mode defaultMode = Mode::NoChange);

      //! Important - must call this to attach to Knob during knobs() call.
      //! Note that since this only assigns during makeKnobs() only the Node firstOp's copy of
      //! this class will have the authoring and value provider assigned.
      //! Authoring mode will default to 'NoChange' and the value provider enabled.
      void InitGeomAuthorMode(Knob_Callback f);

      //---------------------------------------------------

      //! Enabled when in NoChange mode (no authoring enabled) so that Knob
      //! is disabled for editing and displays the schema default value.
      bool provideValuesEnabled(const Knob* knob, const OutputContext&) const override;
      int  providerSourceType(const Knob* knob, const OutputContext&) const override { return 1/*static-value*/; }
      bool isDefault(const Knob* knob, const OutputContext&) const override { return false; }
      bool isAnimated(const Knob* knob, const OutputContext&) const override { return false; }

      //! Returns the schemaValue.
      std::vector<double> provideValues(const ArrayKnobI* arrayKnob, const OutputContext& oc) const override;

      //! Returns the schemaValue.
      void provideStringValue(std::string& value, const StringKnobI* knob, const OutputContext& oc) const override;


    protected:
      int32_t    _nElements;    //!< Derived when InitGeomAuthorMode() is called
      usg::Token _attribName;   //!< Authored attribute name
    };

    //-----------------------------------------------------------------

    //! A class which provides common UI for Imageable and Model prims.
    //! Create one of these and call the knobs method to make the knobs.
    //! Then call the apply method to author the attributes.
    class DDImage_API ImageableAndModelKnobs {
    public:
      ImageableAndModelKnobs();
      ImageableAndModelKnobs(int purpose, int visibility = 0, int kind = 0);

      void knobs(Knob_Callback f, GeomOp* op);

      //! Just call knobs() for the constituent knobs, don't create closed group
      void knobsWithoutGroup(Knob_Callback f, GeomOp* op);

      class DDImage_API Binding {
      public:
        Binding(GeomOpEngine& engine);

        //! Write value to target prim.
        void apply(usg::Prim& prim, const usg::GeomSceneContext& context);

      protected:
        const GeomOpEngine::KnobBinding _purposeKnob;
        const GeomOpEngine::KnobBinding _visibilityKnob;
        const GeomOpEngine::KnobBinding _kindKnob;
      };

    protected:
      AuthoredValue<int> _purpose;
      AuthoredValue<int> _visibility;
      AuthoredValue<int> _kind;
    };

    //-----------------------------------------------------------------

    //! A class which provides common UI for color and opacity.
    //! Create one of these and call the knobs method to make the knobs.
    //! Then call the apply method to author the attributes.
    class DDImage_API ColorKnobs
    {
    public:
      ColorKnobs();
      ColorKnobs(const fdk::Vec3f& displayColor, float displayOpacity = 1.0f);

      void knobs(Knob_Callback f, GeomOp* op);

      //! Just call knobs() for the constituent knobs, don't create closed group
      void knobsWithoutGroup(Knob_Callback f, GeomOp* op);

      class DDImage_API Binding {
      public:
        Binding(GeomOpEngine& engine);

        //! Write value to target prim.
        void apply(usg::Prim& prim, const usg::GeomSceneContext& context);

      protected:
        const GeomOpEngine::KnobBinding _displayColorKnob;
        const GeomOpEngine::KnobBinding _displayOpacityKnob;
      };

    protected:
      AuthoredValue<fdk::Vec3f> _displayColor;
      AuthoredValue<float>      _displayOpacity;
    };

    //-----------------------------------------------------------------

    //! A class which provides common UI for Mesh prims.
    //! Create one of these and call the knobs method to make the knobs.
    //! Then call the apply method to create/set the attributes.
    class DDImage_API MeshKnobs {
    public:
      enum class SubdivisionScheme {
        CatmullClark,
        Loop,
        Bilinear,
        None
      };

      struct KnobParams
      {
        int subdivisionScheme = 3; //! Index into subdivision schemes vector - this MUST be a valid index
        int interpolateBoundary = 0; //! 0=none, 1=edgeOnly, 2=edgeAndCorner 
        bool doubleSided = false; //! Double sided shading
        //! Subdivision schemes available in the subdivision scheme knob.
        std::vector<SubdivisionScheme> subdivisionSchemes = {
          SubdivisionScheme::CatmullClark, SubdivisionScheme::Loop, SubdivisionScheme::Bilinear, SubdivisionScheme::None};
      };

    public:
      MeshKnobs();
      MeshKnobs(int subdivisionScheme, int interpolateBoundary = 0, bool doubleSided = false);
      explicit MeshKnobs(KnobParams params);

      void knobs(Knob_Callback f, GeomOp* op);

      //! Just call knobs() for the constituent knobs, don't create closed group
      void knobsWithoutGroup(Knob_Callback f, GeomOp* op);
 
      class DDImage_API Binding {
      public:
        Binding(GeomOpEngine& engine);

        //! Write value to target prim.
        void apply(usg::Prim& prim, const fdk::TimeValue& time = fdk::defaultTimeValue());

      protected:
        const GeomOpEngine::KnobBinding _subdivisionSchemeKnob;
        const GeomOpEngine::KnobBinding _interpolateBoundaryKnob;
        const GeomOpEngine::KnobBinding _doubleSidedKnob;
      };

    private:
      std::vector<const char*> getSubdivisionSchemeMenuItems() const;
      std::string getSubdivisionSchemeTooltip() const;
      

      bool _authoringEnabled;
      AuthoredValue<int> _subdivisionScheme;    //!< From UsdGeomMesh class
      AuthoredValue<int> _interpolateBoundary;  //!< From UsdGeomMesh class
      AuthoredValue<int> _doubleSided;          //!< From UsdGeomGprim class
      std::vector<SubdivisionScheme> _subdivisionSchemes;   //!< Allowed Subdivision schemes
    };

    //! Safe Subdivision schemes for triangular meshes
    const std::vector<MeshKnobs::SubdivisionScheme> kTriMeshSubdivisionSchemes = 
    {
      MeshKnobs::SubdivisionScheme::Loop,
      MeshKnobs::SubdivisionScheme::Bilinear,
      MeshKnobs::SubdivisionScheme::None
    };

    //! Safe Subdivision schemes for quad meshes
    const std::vector<MeshKnobs::SubdivisionScheme> kQuadMeshSubdivisionSchemes = 
    {
      MeshKnobs::SubdivisionScheme::CatmullClark,
      MeshKnobs::SubdivisionScheme::Bilinear,
      MeshKnobs::SubdivisionScheme::None
    };

    //! Safe Subdivision schemes for any mesh type
    const std::vector<MeshKnobs::SubdivisionScheme> kAnyMeshSubdivisionSchemes = 
    {
      MeshKnobs::SubdivisionScheme::Bilinear,
      MeshKnobs::SubdivisionScheme::None
    };

    //! All Subdivision schemes
    const std::vector<MeshKnobs::SubdivisionScheme> kAllSubdivisionSchemes = 
    {
      MeshKnobs::SubdivisionScheme::CatmullClark,
      MeshKnobs::SubdivisionScheme::Loop,
      MeshKnobs::SubdivisionScheme::Bilinear,
      MeshKnobs::SubdivisionScheme::None
    };

    //-----------------------------------------------------------------

    //! A class which provides common UI for transforming prims.
    //! Create one of these and call the knobs method to make the knobs.
    //! Then call the apply method to author the attributes.
    class DDImage_API TransformKnobs {
    public:
      enum class XformOpOrder : int32_t
      {
        NoChange,
        Append,
        Prepend,
        Replace
      };

      struct KnobParams
      {
        std::string transformKnobName = "transform";
        bool hasOpOrderKnob = false;
        std::string opOrderKnobName = "xform_op_order";
        //! Users may offer only a subset of these
        std::vector<XformOpOrder> opOrderOptions = { XformOpOrder::NoChange, XformOpOrder::Append, XformOpOrder::Prepend, XformOpOrder::Replace };
        //! Used as storage and the default value for the op order knob. Index matches the given opOrderOptions
        int xformOpOrder = 0;
        std::string opOrderTooltip = "Specifies the point at which this transform is applied to the prims (selected by the mask). "
          "USD prims support multiple transforms.  This setting determines whether the transform "
          "should be applied before (Prepend) or after (Append) the upstream transforms, "
          "it replaces the existing transform (Replace) or does not change the current transform "
          "at all (No Change).";
      };

      static const KnobParams kDefaultKnobParams;

    protected:
      class DDImage_API Binding {
      public:
        Binding(GeomOpEngine* engine, Knob* transformKnob, Knob* opOrderKnob);

        //! Write value to target prim. If we have no op order knob, replace the transform.
        void apply(usg::XformablePrim& prim, const usg::GeomSceneContext& context,
          const std::string& opName = "local_axis") const;

        bool isAnimated() const { return _transformKnob->isAnimated(); }

      protected:
        GeomOpEngine* _engine;
        const Knob* _transformKnob;
        const Knob* _opOrderKnob;
      };

    public:
      TransformKnobs(const std::string& transformKnobName = "transform");

      TransformKnobs(const KnobParams& params);

      void knobs(Knob_Callback f, GeomOp* op);

      Knob* transformKnob() const { return _transformKnob; }
      const std::string& transformKnobName() const { return _knobParams.transformKnobName; }

      Knob* opOrderKnob() const { return _opOrderKnob; }
      const std::string& opOrderKnobName() const { return _knobParams.opOrderKnobName; }

      const fdk::Mat4d& transform() const { return _localTransform; }

      Binding getBinding(GeomOpEngine* engine) const;

      XformOpOrder getOpOrderFromKnobValue(int knobValue) const;

    protected:
      Knob* _transformKnob = nullptr;
      Knob* _opOrderKnob = nullptr;
      fdk::Mat4d _localTransform;            //!< Local matrix that Axis_Knob fills in
      KnobParams _knobParams;
    };

    //-----------------------------------------------------------------

    //! A class which provides common UI for creating prims.
    //! Create one of these and call the knobs method to make the knobs.
    //! Then call the apply method to create the missing parents.
    class DDImage_API PathKnobs
    {
    protected:
      class DDImage_API Binding
      {
      public:
        Binding(GeomOpEngine* engine,
                std::string_view primPathKnobName,
                std::string_view createMissingParentsKnobName,
                std::string_view parentPrimTypeKnobName);

        void apply(usg::StageRef& inputStage,
                   usg::LayerRef& editLayer,
                   const usg::GeomSceneContext& context);

        const usg::Path& evaluatedPrimPath() const { return _evaluatedPrimPath; }

      protected:
        GeomOpEngine* _engine;
        GeomOpEngine::KnobBinding _primPathKnob;
        GeomOpEngine::KnobBinding _createParentsKnob;
        GeomOpEngine::KnobBinding _parentTypeKnob;
        usg::Path _evaluatedPrimPath;
        GeomOp* _geomOp;
      };

    public:
      PathKnobs(const std::string& primPath = "{nodename}",
        const std::string& knobNamePrefix = "");

      void knobs(Knob_Callback f, GeomOp* op);

      const std::string& primPath() const { return _primPath; }

      const std::string& pathKnobName() const { return _pathKnobName; }
      const std::string& createMissingParentsKnobName() const { return _createMissingParentsKnobName; }
      const std::string& missingParentTypeKnobName() const { return _missingParentTypeKnobName; }

      //! Only valid after knobs() has been called.
      DD::Image::Knob* pathKnob() const { return _pathKnob; }

      //! Only valid after knobs() has been called.
      DD::Image::Knob* createMissingParentsKnob() const { return _createMissingParentsKnob; }

      //! Only valid after knobs() has been called.
      DD::Image::Knob* missingParentTypeKnob() const { return _missingParentTypeKnob; }

      Binding getBinding(GeomOpEngine* engine) const;

    protected:
      //! Possible types to create for missing parents
      enum class ParentType {
        Scope,
        Xform,
      };

      std::string _primPath;                              //!< The root path for created prims

      std::string _pathKnobName;
      std::string _createMissingParentsKnobName;
      std::string _missingParentTypeKnobName;

      DD::Image::Knob* _pathKnob = nullptr;
      DD::Image::Knob* _createMissingParentsKnob = nullptr;
      DD::Image::Knob* _missingParentTypeKnob = nullptr;

      bool _createMissingParents = true;                  //! Whether to create the missing parent prims or not
      ParentType _missingParentType = ParentType::Xform;  //! The type for missing parent prims we create
    };

    //-----------------------------------------------------------------

    //! A class which provides common UI for modifying prims.
    //! Create one of these and call the knobs method to make the knobs.
    //! In the engine, get a binding to retrieve the prim paths
    //! in the input stage matching the mask set by the knob.
    class DDImage_API MaskKnobs
    {
    protected:
      class DDImage_API Binding
      {
      public:
        Binding(GeomOpEngine* engine, std::string_view knobName);

        std::string filterSource() const;
        usg::PathArray filteredPrimPaths(const usg::StageRef& srcStage);
        void filteredPrimPaths(const usg::StageRef& srcStage, usg::PathArray& paths);

        std::string_view getMaskKnobName() const { return _maskKnobName; }

      protected:
        GeomOpEngine* _engine;
        const std::string_view _maskKnobName;
      };

    public:
      MaskKnobs(std::string_view filterMask = "{default}", 
                std::string_view maskKnobName = "mask_patterns",
                std::string_view injectKnobName = "inject_mask");

      void knobs(Knob_Callback f, GeomOp* op);

      const std::string& filterSource() const { return _filterMask; }

      std::string_view getMaskKnobName() const { return _maskKnobName; }
      Knob* maskKnob() const { return _maskKnob; }

      std::string_view getInjectKnobName() const { return _injectKnobName; }

      Binding getBinding(GeomOpEngine* engine) const;

    protected:
      const std::string_view _maskKnobName;
      const std::string_view _injectKnobName;
      std::string _filterMask;     //!< Mask patterns
      Knob* _maskKnob = nullptr;
      bool _injectMask = false;

    };

    //-----------------------------------------------------------------

    //! A class which provides common UI for ScanlineRender options on prims.
    //! Create one of these and call the knobs method to make the knobs.
    //! Then call the apply method to author the attributes.
    class DDImage_API SlrRenderOptionsKnobs {
    public:
      SlrRenderOptionsKnobs();
      SlrRenderOptionsKnobs(bool visibleToCamera, bool castShadow, bool receiveShadow);

      void knobs(Knob_Callback f, GeomOp* op, bool groupOpenByDefault = false);

      //! Just call knobs() for the constituent knobs, don't create closed group
      void knobsWithoutGroup(Knob_Callback f, GeomOp* op);

      class DDImage_API Binding {
      public:
        Binding(GeomOpEngine& engine);

        //! Write value to target prim.
        void apply(usg::Prim& prim, const usg::GeomSceneContext& context);

      protected:
        const GeomOpEngine::KnobBinding _visibleToCameraKnob;
        const GeomOpEngine::KnobBinding _castShadowKnob;
        const GeomOpEngine::KnobBinding _receiveShadowKnob;
      };

    protected:
      AuthoredValue<bool> _visibleToCamera;
      AuthoredValue<bool> _castShadow;
      AuthoredValue<bool> _receiveShadow;
    };


//--------------------------------------------------------------------
//--------------------------------------------------------------------

template<typename T>
inline AuthoredValue<T>::AuthoredValue(const AuthoringSettings& settings)
  : Authoring(settings)
  , value(0)
  , schemaValue(0)
  , _nElements(0)
{
}

template<> inline AuthoredValue<const char*>::AuthoredValue(const AuthoringSettings& settings)
 : Authoring(settings) , value("") , schemaValue("") , _nElements(0) {}
template<> inline AuthoredValue<std::string>::AuthoredValue(const AuthoringSettings& settings)
 : Authoring(settings) , value("") , schemaValue("") , _nElements(0) {}

template<typename T>
inline AuthoredValue<T>::AuthoredValue(const T& val, const AuthoringSettings& settings)
  : Authoring(settings)
  , value(val)
  , schemaValue(val)
  , _nElements(0)
{
}

template<typename T>
inline AuthoredValue<T>::AuthoredValue(const T& val, const T& schemaVal, const AuthoringSettings& settings)
  : Authoring(settings)
  , value(val)
  , schemaValue(schemaVal)
  , _nElements(0)
{
}

template<typename T>
inline AuthoredValue<T>::AuthoredValue(const T& val, const T& schemaVal, const char* attribName, Mode mode, Mode defaultMode)
  : Authoring(AuthoringSetNoChangeDefaults(), mode, defaultMode)
  , value(val)
  , schemaValue(schemaVal)
  , _nElements(0)
  , _attribName((attribName) ? attribName : "")
{
}

template<typename T>
inline AuthoredValue<T>::AuthoredValue(const T& val, const T& schemaVal, const usg::Token& attribName, Mode mode, Mode defaultMode)
  : Authoring(AuthoringSetNoChangeDefaults(), mode, defaultMode)
  , value(val)
  , schemaValue(schemaVal)
  , _nElements(0)
  , _attribName(attribName)
{
}

template <typename T>
void AuthoredValue<T>::InitGeomAuthorMode(Knob_Callback f)
{
  if (f.makeKnobs()) {
    if (Knob* k = f.getLastMadeKnob()) {
      _nElements = 0; // default for non-ArrayKnob
      k->setAuthoring(this);
      if (ArrayKnobI* arrayKnob = k->arrayKnob()) {
        arrayKnob->setValueProvider(this, true/*enabled*/);
        _nElements = k->numElements();
      }
      if (StringKnobI* stringKnob = k->stringKnob()) {
        stringKnob->setValueProvider(this, true/*enabled*/);
      }
    }
  }
}

template <typename T>
bool AuthoredValue<T>::provideValuesEnabled(const Knob* knob, const OutputContext& oc) const
{
  return (_mode == Mode::NoChange);
}

template <typename T>
void AuthoredValue<T>::provideStringValue(std::string& value, const StringKnobI* knob, const OutputContext& oc) const {}

template<> inline void AuthoredValue<const char*>::provideStringValue(std::string& value, const StringKnobI* knob, const OutputContext& oc) const 
{ 
  if (schemaValue) {
    value = schemaValue;
  } 
  else {
    value = "";
  }
}
template<> inline void AuthoredValue<std::string>::provideStringValue(std::string& value, const StringKnobI* knob, const OutputContext& oc) const { value = schemaValue; }

// TODO ndk17: switch these template specializations to a method that can handle extracting the values from
//             compound and single (pod) types:
template <typename T>
std::vector<double> AuthoredValue<T>::provideValues(const ArrayKnobI* arrayKnob, const OutputContext&) const { return std::vector<double>(1, static_cast<double>(schemaValue)); }

template <typename T>
void CopyArray(double* dst, const T* src, size_t n) { for (size_t i = 0; i < n; ++i) { *dst++ = static_cast<double>(*src++); } }

template <> inline std::vector<double> AuthoredValue<fdk::Vec2f>::provideValues(const ArrayKnobI* arrayKnob, const OutputContext& /*context*/) const { std::vector<double> vals(arrayKnob->array_size()); CopyArray(vals.data(), schemaValue.array(), vals.size()); return vals; }
template <> inline std::vector<double> AuthoredValue<fdk::Vec2i>::provideValues(const ArrayKnobI* arrayKnob, const OutputContext& /*context*/) const { std::vector<double> vals(arrayKnob->array_size()); CopyArray(vals.data(), schemaValue.array(), vals.size()); return vals; }
template <> inline std::vector<double> AuthoredValue<fdk::Vec3f>::provideValues(const ArrayKnobI* arrayKnob, const OutputContext& /*context*/) const { std::vector<double> vals(arrayKnob->array_size()); CopyArray(vals.data(), schemaValue.array(), vals.size()); return vals; }
template <> inline std::vector<double> AuthoredValue<fdk::Vec4f>::provideValues(const ArrayKnobI* arrayKnob, const OutputContext& /*context*/) const { std::vector<double> vals(arrayKnob->array_size()); CopyArray(vals.data(), schemaValue.array(), vals.size()); return vals; }
template <> inline std::vector<double> AuthoredValue<fdk::Vec3d>::provideValues(const ArrayKnobI* arrayKnob, const OutputContext& /*context*/) const { std::vector<double> vals(arrayKnob->array_size()); CopyArray(vals.data(), schemaValue.array(), vals.size()); return vals; }
template <> inline std::vector<double> AuthoredValue<fdk::Vec4d>::provideValues(const ArrayKnobI* arrayKnob, const OutputContext& /*context*/) const { std::vector<double> vals(arrayKnob->array_size()); CopyArray(vals.data(), schemaValue.array(), vals.size()); return vals; }
template <> inline std::vector<double> AuthoredValue<fdk::Mat4f>::provideValues(const ArrayKnobI* arrayKnob, const OutputContext& /*context*/) const { std::vector<double> vals(arrayKnob->array_size()); CopyArray(vals.data(), schemaValue.array(), vals.size()); return vals; }
template <> inline std::vector<double> AuthoredValue<fdk::Box2f>::provideValues(const ArrayKnobI* arrayKnob, const OutputContext& /*context*/) const { std::vector<double> vals(arrayKnob->array_size()); CopyArray(vals.data(), schemaValue.array(), vals.size()); return vals; }
template <> inline std::vector<double> AuthoredValue<fdk::Box3f>::provideValues(const ArrayKnobI* arrayKnob, const OutputContext& /*context*/) const { std::vector<double> vals(arrayKnob->array_size()); CopyArray(vals.data(), schemaValue.array(), vals.size()); return vals; }

// null versions:
template <> inline std::vector<double> AuthoredValue<const char*>::provideValues(const ArrayKnobI*, const OutputContext&) const { return std::vector<double>(); }
template <> inline std::vector<double> AuthoredValue<std::string>::provideValues(const ArrayKnobI*, const OutputContext&) const { return std::vector<double>(); }

  }
}

#endif
