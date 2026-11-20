//
// Copyright (c) 2021 The Foundry Visionmongers Ltd. All Rights Reserved.
//

#ifndef DDIMAGE_SOURCEGEOMOP_H
#define DDIMAGE_SOURCEGEOMOP_H

#include "DDImage/GeomOp.h"
#include "DDImage/GeomOpKnobs.h"
#include "DDImage/GeomOpMixIns.h"
#include "DDImage/KnobMixIns.h"

namespace DD
{
  namespace Image
  {

    //! Subclass of GeomOp that supports a ShaderOp input and local transform
    //! for ease of object setup.

    //! Connection to legacy 'Material' class is not allowed, although
    //! connection to Iops for simple textures is.

    //! Do not use this class if there is not at least 1 input for the merge
    //! source (ie input 0)! If your class does not need a merge input or material
    //! input then subclass directly from GeomOp instead.
    class DDImage_API SourceGeomOp : public GeomOp,
                                     public TransformGeomOpI,
                                     public MaterialBindingOpI
    {
    protected:
      std::string _primPath;                  //!< The root path for created prims

      //! Possible types to create for missing parents
      enum class ParentType {
        Scope,
        Xform,
      };
      ParentType _missingParentType = ParentType::Xform;             //! The type for missing parent prims we create
      bool _createMissingParents = true;                             //! Whether to create the missing parent prims or not
      bool _selectable = true;
      
      SnapKnobs _snapKnobs;

    protected:
      //! The plugin author must at least implement the \a createPrims() method.
      class DDImage_API SourceEngine : public GeomOpEngine
      {
      protected:

        SourceEngine(GeomOpNode* parent);

        //!
        SourceGeomOp* sourceGeomOp() const { return reinterpret_cast<SourceGeomOp*>(firstOp()); }

        //! Initializes the scene state then calls the subclass createPrims().
        void  processScenegraph(usg::GeomSceneContext& context) override;
        

        //! Subclasses implement this to create primitives.
        //! \a context is the scene context
        //! \a path is the suggested path for the new prims.
        virtual void createPrims(usg::GeomSceneContext& context,
                                 const usg::Path&   path) = 0;


        //! Creates and assigns the Material to the given prims
        void assignMaterial(usg::GeomSceneContext& context,const usg::PathArray& primPaths);


      protected:
        // Targets are per-engine so they represent layer/stage state
        usg::GeomStateTarget  materialTopologyTarget; //!< 'material-topology' - affects the define-geometry target
        usg::GeomStateTarget  materialPropertyTarget; //!< 'material-property' - affects the edit-stage target
        usg::GeomStateTarget  modifyXformTarget;      //!< 'modify-xform' - affects the modify-values target
        usg::Token _parentPrimType;                   //!< The type of prim to create for the main prim's parent (default Xform)
        usg::Path  _evaluatedPrimPath;                //!< Evaluated output path of prim in stage
        TransformGeomOpI::SubEngine _transformSubEngine;
        MaterialBindingOpI::SubEngine _materialBindingSubEngine;
      };

    public:
      //! Constructs a SourceGeomOp with a single texture input.
      SourceGeomOp(Node*               node,
                   GeomOpEngineBuilder engineBuilder);

      const char* Class() const override;
      const char* node_help() const override;


      //------------------------------------------------------
      // Nuke Node methods
      //------------------------------------------------------

      //! Returns the node input to use for the merge geometry source. Always defaults to input 0.
      int          mergeInput() const override { return 0; }

      //! Returns the node input to use for the material binding.
      //! Default base class material input is 1.
      virtual int  materialInput() const { return 1; }

      //! Default is 2: merge input and material input.

      //! Do not use this class if the subclass will not support merge input 0!
      int          minimum_inputs() const override { return 2/*merge + material*/; }

      //! Default is 2: merge input and material input.

      //! Do not use this class if the subclass will not support merge input 0!
      int          maximum_inputs() const override { return 2/*merge + material*/; }

      //! Allow ShaderOp or Iop *only* for eMaterialInput.
      //! Connection to legacy 'Material' class is not allowed!
      bool         test_input(int node_input,
                              Op* op) const override;
      //! Disconnected eMaterialInput is assigned a black default constant ShaderOp,
      //! matching legacy behavior.
      Op*          default_input(int node_input) const override;

      //! 'img' for input eMaterialInput.
      const char*  input_label(int   node_input,
                               char* buffer) const override;

      //! build_handles calls buildXformHandles with the (global) local matrix
      //! so nodes up the tree are displayed correctly.
      void         build_handles(ViewerContext* ctx) override;

      //! Updates the general info from the material input. Otherwise the default behaviour
      //! from GeomOp is used.
      void         updateGeneralInfo() override;
      

      //------------------------------------------------------
      // Knobs
      //------------------------------------------------------

      //! Calls the GeomOp base class with \a is_source_geo=true so that
      //! the display knobs are correct for a Source object, then calls
      //! addSourceKnobs() to let a subclass add it's own knobs.
      void knobs(Knob_Callback) override;

      //! Overload of \a knobs() which allows the primPath to be not created
      void knobs(Knob_Callback f, bool addPathKnob);

      //! Subclass specifies knobs that creates source geometry.
      //! Base class does nothing.
      virtual void addSourceKnobs(Knob_Callback f) {}

      //! Adds knobs to do with display e.g. whether the geometry should be selectable.
      virtual void addDisplayOptionsKnobs(Knob_Callback f);

      //!
      int knob_changed(Knob* k) override;

      //!
      void appendGeomState(DD::Image::Hash& op_hash) override;

      //! Whether the geometry the node is managing should be pickable in the Viewer.
      bool selectable() const override;

      //------------------------------------------------------


    protected:
      //! Make an Axis_Knob called "transform" which will drive the local transform. Calling this
      //! will cause a parent Xform prim containing the tranform before createPrims is called.
      //! Plugins can implement their own implementation of this to support custom transform knobs.
      virtual Knob* makeTransformKnob(Knob_Callback f);

      virtual Knob* getTransformKnob() const;

      //! Return the parent transformation for the handles.

      //! The parent transformation is a part of the transformation of the target prim that
      //! in the transformation order comes before the actual transformation produced by
      //! the Axis_Knob. Depending on the value of the transformation operation order knob,
      //! when Prepend, the parent transformation is the concatenated transformation of the
      //! parents of the target prim, or when Append, it's the concatenated transformation
      //! of the parents and the local transformation of the target prim.

      //! This is called in build_handles() when the handles are building.
      fdk::Mat4d handlesParentTransform(DD::Image::ViewerContext* ctx) override;

    };


  } // namespace Image
} // namespace DD

#endif
