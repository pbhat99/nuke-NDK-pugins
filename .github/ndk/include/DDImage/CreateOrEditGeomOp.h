// Copyright (c) 2021 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef DDIMAGE_CREATEOREDITGEOMOP_H
#define DDIMAGE_CREATEOREDITGEOMOP_H

#include "DDImage/GeomOp.h"
#include "DDImage/GeomOpKnobs.h"
#include "DDImage/GeomOpMixIns.h"

namespace DD
{
  namespace Image
  {
    //! Subclass of GeomOp that supports editing existing or creating new prims
    //! depending on the current mode, as set by a knob.
    class DDImage_API CreateOrEditGeomOp : public GeomOp,
                                           public CreateParentsGeomOpI,
                                           public ModifyPathsGeomOpI
    {
    public:
      enum class Mode {
        Create,
        Edit,
      };

      CreateOrEditGeomOp(Node* node, GeomOpEngineBuilder builder,
        Mode defaultMode = Mode::Create,
        const std::string& primPath = "{nodename}",
        const std::string& createParentsKnobNamePrefix = "",
        std::string_view editFilterMask = "{default}", 
        std::string_view editMaskKnobName = "mask_patterns");

      void knobs(DD::Image::Knob_Callback f) override;
      int knob_changed(Knob* k) override;

      const char* modeKnobName() const;

      virtual void addDisplayOptionsKnobs(DD::Image::Knob_Callback);
      bool selectable() const override;

      const usg::PathArray& filteredPrims() const;

      class DDImage_API Engine : public GeomOpEngine
      {
      public:
        //! If filter is not nullptr it will be called to narrow the set of filtered prims
        Engine(GeomOpNode* parent, FilterPrimsGeomEngineI* filter = nullptr)
        : GeomOpEngine(parent)
        , _modifySubEngine(this, filter)
        {
        }

        //! Get current set of prims to create or edit.
        const usg::PathArray& filteredPrims() const;

        //! Returns true if filteredPrims updated.
        //! Applies additional filtering if constructor was passed a filter object.
        bool updateFilteredPrimPaths(usg::GeomEngine* inputEngine,
          const usg::StageRef& inputStage);
        
      protected:
        ModifyPathsGeomOpI::SubEngine _modifySubEngine;
        usg::Path _evaluatedPrimPath;

        //! Override this to process the paths to create or edit (will be called repeatedly if there are several paths).
        virtual void processEdits(usg::GeomSceneContext& context, const usg::Path& path, Mode mode) = 0;

        //! Calls subclass processEdits().
        void processScenegraph(usg::GeomSceneContext& context) override;

        //! Returns current mode as set by the knob.
        CreateOrEditGeomOp::Mode mode() const;
      };

    protected:
      int _mode = static_cast<int>(Mode::Create);
      bool _selectable = true;
    };
  }
}

#endif
