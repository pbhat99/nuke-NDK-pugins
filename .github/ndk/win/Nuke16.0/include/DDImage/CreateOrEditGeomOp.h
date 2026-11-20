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

      CreateOrEditGeomOp(Node* node, GeomOpEngine::Builder builder,
        const std::string& primPath = "{nodename}",
        const std::string& createParentsKnobNamePrefix = "");

      void knobs(DD::Image::Knob_Callback f) override;
      int knob_changed(Knob* k) override;

      const char* modeKnobName() const;

      const usg::PathArray& filteredPrims() const;

      class DDImage_API Engine : public GeomOpEngine
      {
      public:
        //! If filter is not nullptr it will be called to narrow the set of filtered prims
        Engine(Op* parent, FilterPrimsGeomEngineI* filter = nullptr)
        : GeomOpEngine(parent)
        , _modifySubEngine(this, filter)
        {
        }

        //! Get current set of prims to create or edit.
        const usg::PathArray& filteredPrims() const;
        
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
    };
  }
}

#endif
