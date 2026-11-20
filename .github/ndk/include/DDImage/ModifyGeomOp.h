//------------------------------------------------------------------------------
// ModifyGeomOp.h
//
// Copyright (c) 2021 The Foundry Visionmongers Ltd.  All Rights Reserved.
//------------------------------------------------------------------------------

#ifndef DDIMAGE_MODIFYGEOMOP_H
#define DDIMAGE_MODIFYGEOMOP_H

#include "DDImage/GeomOp.h"
#include "DDImage/GeomOpMixIns.h"

// TODO: is this still needed?
#ifdef _WIN32
  #pragma warning(disable:4251)
#endif

namespace DD
{
  namespace Image
  {


    /*!
     */
    class DDImage_API ModifyGeomOp : public GeomOp,
                                     public ModifyPathsGeomOpI
    {
    protected:
      /*! 
          The plugin author should at least implement the \a processPrim() method.
       */
      class DDImage_API ModifyEngine : public GeomOpEngine,
                                       public FilterPrimsGeomEngineI
      {
      public:
        usg::PathArray filteredPrims();

      protected:
        ModifyPathsGeomOpI::SubEngine _modifySubEngine;

        //! Prim filtering will be done in subclass's secondaryFilterPrimPaths()
        //! unless filter argument is a non-null pointer.
        ModifyEngine(GeomOpNode* parent, FilterPrimsGeomEngineI* filter = nullptr)
        : GeomOpEngine(parent)
        , _modifySubEngine(this, this)
        {
          if (filter) {
            _modifySubEngine.setFilter(filter);
          }
        }

        /*! Call this to re-evaluate the filtered paths defined by the mask expression
            and any additional filter logic a subclass may want to apply in
            filteredPrimPaths() and secondaryFilterPrimPaths().
            Returns true if the path set changed.
        */
        bool  updateFilteredPrimPaths(GeomEngine*          inputEngine,
                                      const usg::StageRef& stage);

        /*! Change the hash of the current filter. Subclasses can implement this to force the
            prim filter methods to be re-run, usually because the subclass has knobs that can
            affect the result of the filtering.
            Default implementation does nothing.
        */
        virtual void  appendFilterHash(fdk::Hash& filterHash) const {}

        /*! Calls buildScenegraph() on input 0 then the subclass processPrimSelection() method.
        */
        void  processScenegraph(usg::GeomSceneContext& context) override;


      protected:
        /*! Get the list of filtered prim paths from the provided stage using the
            current scenegraph filter. ModifyEngine calls this in updateFilteredPrimPaths()
            when the mesh expression or input stage compose state changes, and saves
            the resulting path list to _filtered_prims.

            Subclasses can override this method to extend/replace the filtering logic
            of the builtin PathExpression knob, or implement secondaryFilterPrimPaths()
            to do Prim type testing, etc.
        */
        virtual usg::PathArray filteredPrimPaths(const usg::StageRef& srcStage);

        /*  Apply an additional filtering pass to the prim paths returned by
            filteredPrimPaths(), returning true if a change was made.

            Unculled paths should be copied to \a outPrimPaths and true returned to
            indicate the filter should use & cache \a outPrimPaths.

            If nothing was culled the return false and the original \a inPrimPaths
            will be used and cached.

            If everything was culled outPrimPaths should be left empty and true returned.

            Default implementation does nothing and returns false.
        */
        bool secondaryFilterPrimPaths(const usg::StageRef&  srcStage,
          const usg::PathArray& inPrimPaths,
          usg::PathArray&       outPrimPaths) override { return false; }


        /*! Loops through the selected prim paths calling processPrim().

            This method is called from processScenegraph() only if the input or local
            state is dirty.

            This is virtual to allow a subclass that needs the entire prim
            selection a chance to do something before the per-prim loop is run.
            For example finding the total bbox of the selected prims.

            So a subclass would implement this method, do something with the prim
            paths first, then call ModifyGeomOp::processPrimSelection() to have
            the prim loop run.
            Or it can just do all the work in here and not bother calling the base
            class implementation.
        */
        virtual void  processPrimSelection(usg::GeomSceneContext& context,
                                           const usg::StageRef&   srcStage,
                                           const usg::PathArray&  primPaths);

        /*! Per-prim processing method.

            This method is called from a prim path loop in processPrimSelection()
            only if the input or local state is dirty.

            If your subclass needs to work with the entire selection set then
            just implement processPrimSelection().
        */
        virtual void  processPrim(usg::GeomSceneContext& context,
                                  const usg::StageRef&   srcStage,
                                  const usg::Path&       primPath) {}

      };


      //-------------------------------------------------------------------------


      ModifyGeomOp(Node*               node,
                   GeomOpEngineBuilder engineBuilder,
                   std::string_view filterMask = "{default}",
                   std::string_view maskKnobName = "mask_patterns",
                   std::string_view injectKnobName = "inject_mask"
                   ); // --- IGNORE ---

      /*! Adds standard scenegraph filter knobs.
          Subclasses can override this to extend the knob set.
      */
      virtual void    addFilterKnobs(Knob_Callback f);


    public:
      //! Calls addPrimFilterKnobs().
      void            knobs(Knob_Callback f) override;
      //! Handles changes to the filter knobs.
      int             knob_changed(Knob*) override;

    };


  }
}

#endif
