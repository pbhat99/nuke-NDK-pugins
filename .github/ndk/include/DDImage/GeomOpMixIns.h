// Copyright (c) 2024 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef DDIMAGE_GEOMOPMIXINS_H
#define DDIMAGE_GEOMOPMIXINS_H

#include <string_view>

#include "DDImage/GeomOpKnobs.h"

namespace DD
{
  namespace Image
  {
    class GeomOpEngine;

    //! Implements the transform behaviour for GeomOps.
    //! GeomOp plugins that wish to have this behaviour should inherit
    //! from this class and call TransformGeomOpI::knobs from its knobs
    //! method to set up the required knobs.
    class DDImage_API TransformGeomOpI
    {
    public:
      TransformGeomOpI(const std::string& name = "transform");
      TransformGeomOpI(const TransformKnobs::KnobParams& params);

      int axisInput() const;

      fdk::Mat4d handlesParentTransform(ViewerContext* ctx, const GeomOp* op, const std::string& primPath) const;

      //! Sets up an axis knob in \a op.
      void knobs(Knob_Callback f, GeomOp* op);

      const TransformKnobs* transformKnobs() const { return &_transformKnobs; }

      //! Implements the usg::GeomEngine transform behaviour
      //! In the GeomOp plugin's engine, instantiate this subclass
      //! and call the apply method to apply the correct xform to
      //! the prim.
      class DDImage_API SubEngine
      {
      public:
        SubEngine(GeomOpEngine* engine);

        void apply(usg::GeomSceneContext& context, usg::XformablePrim& xform,
          const std::string& opName = "local_axis");

      private:
        GeomOpEngine* _engine;
        usg::GeomStateTarget _modifyXformTarget;      //!< 'modify-xform' - affects the modify-values target
        TransformKnobs* _transformKnobs{nullptr};
      };

    protected:
      TransformKnobs _transformKnobs;
    };

    //! Implements the parent path creation behavior for GeomOps.
    //! GeomOp plugins that wish to have this behaviour should inherit
    //! from this class and call knobs() from its knobs
    //! method to set up the required knobs.
    class DDImage_API CreateParentsGeomOpI
    {
    public:
      CreateParentsGeomOpI(const std::string& primPath = "{nodename}",
        const std::string& knobNamePrefix = "");

    protected:
      //! Implements the usg::GeomEngine parent path creation behaviour
      //! In the GeomOp plugin's engine, instantiate this subclass
      //! and call the apply method to create the missing parents.
      class DDImage_API SubEngine
      {
      public:
        void apply(usg::GeomSceneContext& context,
                    usg::StageRef& inputStage,
                    GeomOpEngine* engine,
                    usg::Path& evaluatedPrimPath);
      private:
        PathKnobs* _pathKnobs{nullptr};
      };

      //! Sets up a path knob in \a op.
      void knobs(Knob_Callback f, GeomOp* op);

      int knob_changed(Knob* k, GeomOp* op);

      PathKnobs _pathKnobs;
    };


    //! Implements the behaviour of binding a material to prims.
    //! GeomOp plugins that wish to have this behaviour should inherit
    //! from this class and call its methods in the appropriate places
    //! to use its functionality.
    class DDImage_API MaterialBindingOpI
    {
    public:
      //! Returns the node input to use for the material binding.
      int  materialInput() const;

      //! Returns the label to be used for the material input.
      const char* materialInputLabel() const;

      //! Test wether \a op can be used as a material
      //! Call this method from the plugin's test_input method.
      bool testMaterialInput(Op* op) const;

      //! Computes the Geom State based on the material input and appends
      //! to \a opHash.
      //! Call this method from the plugin's appendGeomState method.
      void appendMaterialGeomState(GeomOp* op, DD::Image::Hash& opHash);

      //! Implements the usg::GeomEngine material binding behaviour
      //! In the GeomOp plugin's engine, instantiate this subclass
      //! and call the apply method to apply the material binding to
      //! the prims.
      class DDImage_API SubEngine
      {
      public:
        SubEngine(GeomOpEngine* engine);

        //! Applies the material bindings to the prims located at \a primPaths
        void apply(usg::GeomSceneContext context, const usg::PathArray& primPaths);

      private:
        GeomOpEngine* _engine;
        usg::GeomStateTarget  materialTopologyTarget; //!< 'material-topology' - affects the define-geometry target
        usg::GeomStateTarget  materialPropertyTarget; //!< 'material-property' - affects the edit-stage target
      };
    };

    //! For use with ModifyPathsGeomOpI::SubEngine
    class DDImage_API FilterPrimsGeomEngineI
    {
    public:
        /*  Apply an additional filtering pass to the prim paths returned by
            ModifyPathsGeomOpI::updateFilteredPrimPaths(), returning true if a change was made.

            Unculled paths should be copied to \a outPrimPaths and true returned to
            indicate the filter should use & cache \a outPrimPaths.

            If nothing was culled then return false and the original \a inPrimPaths
            will be used and cached.

            If everything was culled outPrimPaths should be left empty and true returned.
        */
        virtual bool secondaryFilterPrimPaths(const usg::StageRef&  srcStage,
                                              const usg::PathArray& inPrimPaths,
                                              usg::PathArray&       outPrimPaths) = 0;
    };

    //! Allows only point based prims
    class DDImage_API PointBasedPrimFilterGeomEngineI : public FilterPrimsGeomEngineI
    {
    public:
        bool secondaryFilterPrimPaths(const usg::StageRef&  srcStage,
                                      const usg::PathArray& inPrimPaths,
                                      usg::PathArray&       outPrimPaths) override;
    };

    //! Allows only mesh prims
    class DDImage_API MeshPrimFilterGeomEngineI : public FilterPrimsGeomEngineI
    {
    public:
        bool secondaryFilterPrimPaths(const usg::StageRef&  srcStage,
                                      const usg::PathArray& inPrimPaths,
                                      usg::PathArray&       outPrimPaths) override;
    };

    //! Implements the path filtering behavior for GeomOps.
    //! GeomOp plugins that wish to have this behaviour should inherit
    //! from this class and call knobs() from its knobs
    //! method to set up the required knobs.
    class DDImage_API ModifyPathsGeomOpI
    {
    public:
      ModifyPathsGeomOpI(std::string_view filterMask = "{default}", 
                         std::string_view knobName = "mask_patterns",
                         std::string_view injectKnobName= "inject_mask");

    protected:
      //! Implements the usg::GeomEngine path filtering behavior.
      //! In the GeomOp plugin's engine, instantiate this subclass
      //! and call the updateFilteredPrimPaths method to update the
      //! filteredPrims member with the list of masked paths.
      class DDImage_API SubEngine
      {
      public:
        //! The corresponding op has to inherit ModifyPathsGeomOpI!
        //! Applies additional filtering using the filter object, if given.
        SubEngine(GeomOpEngine* eng, FilterPrimsGeomEngineI* filter = nullptr);

        //! Returns true if filteredPrims updated.
        //! Applies additional filtering if constructor was passed a filter object.
        bool updateFilteredPrimPaths(usg::GeomEngine* inputEngine,
          const usg::StageRef& inputStage);

        MaskKnobs* getMaskKnobs();

        void setFilter(FilterPrimsGeomEngineI* filter);
        FilterPrimsGeomEngineI* getFilter() const;

        const usg::PathArray& filteredPrims() const { return _filteredPrims; }

        void updateDownstreamInjectedPaths();

      private:
        MaskKnobs* _maskKnobs{nullptr};
        FilterPrimsGeomEngineI* _filter{nullptr};
        fdk::Hash _lastFilterHash;
        GeomOpEngine* _engine;
        usg::PathArray _filteredPrims;
      };

      //! Sets up path mask knobs in \a op.
      void knobs(Knob_Callback f, GeomOp* op);

      int knob_changed(Knob* k, GeomOp* op);

      MaskKnobs _maskKnobs;
    };

  }
}

#endif
