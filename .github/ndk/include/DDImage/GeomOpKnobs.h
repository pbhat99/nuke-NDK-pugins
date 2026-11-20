// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef DDIMAGE_GEOMOPKNOBS_H
#define DDIMAGE_GEOMOPKNOBS_H

#include <string_view>

#include "DDImage/GeomOp.h"

#include "usg/geom/XformablePrim.h"

namespace DD
{
  namespace Image
  {
    //! A class which provides common UI for Imageable and Model prims.
    //! Create one of these and call the knobs method to make the knobs.
    //! Then call the apply method to author the attributes.
    class DDImage_API ImageableAndModelKnobs {
    public:
      ImageableAndModelKnobs() {}
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
      int _kind = 0;
      int _purpose = 0;
      int _visibility = 0;
    };

    //-----------------------------------------------------------------

    //! A class which provides common UI for display color and opacity.
    //! Create one of these and call the knobs method to make the knobs.
    //! Then call the apply method to author the attributes.
    class DDImage_API DisplayKnobs
    {
    public:
      DisplayKnobs() {}
      DisplayKnobs(const fdk::Vec3f& displayColor, float displayOpacity = 1.0f);

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
      fdk::Vec3f _displayColor{1.0f};
      float _displayOpacity = 1.0f;
    };

    //-----------------------------------------------------------------

    //! A class which provides common UI for Mesh prims.
    //! Create one of these and call the knobs method to make the knobs.
    //! Then call the apply method to create/set the attributes.
    //!
    //! Note that by default these knobs do not have authoring mode enabled since they
    //! are not inherited attributes. Use enableAuthoring() to control this.
    class DDImage_API MeshKnobs {
    public:
      MeshKnobs() {}
      MeshKnobs(int subdivisionScheme, int interpolateBoundary = 0, bool doubleSided = false);

      void knobs(Knob_Callback f, GeomOp* op);

      //! Just call knobs() for the constituent knobs, don't create closed group
      void knobsWithoutGroup(Knob_Callback f, GeomOp* op);

      //! Enable or disable authoring mode.
      void enableAuthoring() { _authoringEnabled = true; }
      void disableAuthoring() { _authoringEnabled = false; }
 
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

    protected:
      int _subdivisionScheme = 0;       //!< From UsdGeomMesh class
      int _interpolateBoundary = 0;     //!< From UsdGeomMesh class
      bool _doubleSided = false;        //!< From UsdGeomGprim class
      bool _authoringEnabled = false;   //!< Do knobs show authoring controls
    };

    //-----------------------------------------------------------------

    //! A class which provides common UI for transforming prims.
    //! Create one of these and call the knobs method to make the knobs.
    //! Then call the apply method to author the attributes.
    class DDImage_API TransformKnobs {
    public:
      TransformKnobs(const std::string& name);

      void knobs(Knob_Callback f, GeomOp* op);

      Knob* transformKnob() const { return _transformKnob; }
      const fdk::Mat4d& transform() const { return _localTransform; }

      class DDImage_API Binding {
      public:
        Binding(GeomOpEngine& engine);

        //! Write value to target prim.
        void apply(usg::XformablePrim& prim, const usg::GeomSceneContext& context, const std::string& opName = "local_axis");

        bool isAnimated() const { return _transformKnob.isAnimated(); }

      protected:
        const GeomOpEngine::KnobBinding _transformKnob;
      };

    protected:
      Knob* _transformKnob;
      fdk::Mat4d _localTransform;            //!< Local matrix that Axis_Knob fills in
      std::string _transformKnobName;
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

        std::string_view getMaskKnobName() const { return _maskKnobName; }

      protected:
        GeomOpEngine* _engine;
        const std::string_view _maskKnobName;
      };

    public:
      MaskKnobs(std::string_view filterMask = "/*", std::string_view knobName = "mask_patterns");

      void knobs(Knob_Callback f, GeomOp* op);

      const std::string& filterSource() const { return _filterMask; }

      std::string_view getMaskKnobName() const { return _maskKnobName; }
      Knob* maskKnob() const { return _maskKnob; }

      Binding getBinding(GeomOpEngine* engine) const;

    protected:
      const std::string_view _maskKnobName;
      std::string _filterMask;     //!< Mask patterns
      Knob* _maskKnob = nullptr;
    };

  }
}

#endif
