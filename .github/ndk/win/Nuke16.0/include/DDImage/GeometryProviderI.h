// Copyright (c) 2025 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDIMAGE_GEOMETRYPROVIDERI_H
#define DDIMAGE_GEOMETRYPROVIDERI_H

#include "DDImage/DDImage_API.h"

#include "usg/geom/Layer.h"
#include "usg/geom/Stage.h"
#include "usg/base/ArgSet.h"


namespace DD {
  namespace Image {
    class Op;
    using OpSet = std::set<Op*>;


    /*! Interface class to attach to another class, usually an Op, that can provide
        geometry in the form of usg::Layers or a usg::Stage. Subclasses can decide to
        provide both a Layer and a Stage, or just a Layer. If providing both then the
        Layer would commonly be the root layer set in the provided Stage.

        Op examples utilizing this interface are GeomOp, AxisOp (CameraOp, LightOp) and
        ScanlineRender2. 

        The returned Layer and Stage are usually local caches on the attached object as
        downstream consumers typically do not have persistent caches in Nuke due to the
        procedural graph mechanisms. A good example of this is the Viewer which contains
        a Stage reference but relies on each Node to store the geometry data in local
        Layers and provide a Stage when asked.

        However the object could also pass the interface methods up the graph to an input
        to provide geometry if the object itself is not geometry aware. A good use of
        this would be an Iop with a geometry input that wants the Viewer to access the
        geometry passing through the Iop. ScanlineRender2 does this for example.

        Note that code accessing GeometryProvider methods should be aware that it may
        not always be on an Op and should be able to handle null being returned from
        getGeometryProviderOp().
    */
    class DDImage_API GeometryProviderI
    {
    public:
      //------------------------------------------------------
      // Build a filled in usg::Stage:

      //! Build the passed-in stage. If the OpSet is empty or the Ops are all invalid
      //! the stage is emptied.
      //!
      //! All Ops in the set must be from the same Node, otherwise only the first Op
      //! in the set will be used and all others from different Nodes will be ignored.
      //! 
      //! These Ops are typically the inputs to a Viewer, renderer Iop, or an Op that
      //! wants to sample geometry. Only Ops that can generate geometry (specifically
      //! a Layer or Stage) are supported, which includes GeomOps or any Op with the
      //! GeometryProviderI interface attached.
      //! 
      //! The outputContext() of each Op is used to define the requested times and
      //! views to be processed for the usg::Stage. A GeometryProvider should try to
      //! fulfill the requested Op times at a minimum, but is free to add additional
      //! time samples.
      //!
      //! If \a additionalTimes is provided those are added to the requested times set.
      static void  buildStage(usg::StageRef&           stage,
                              const usg::ArgSet&       requestArgs,
                              const OpSet&             ops,
                              const fdk::TimeValueSet* additionalTimes = nullptr);

      //! Same as buildStage(OpSet) but for a single Op that can cast to GeomOp or
      //! GeometryProviderI. If Op won't cast then \a stage is cleared.
      static void  buildStage(usg::StageRef&           stage,
                              const usg::ArgSet&       requestArgs,
                              Op*                      geometryProviderOp,
                              const fdk::TimeValueSet* additionalTimes = nullptr);


    public:
      GeometryProviderI() = default;
      virtual ~GeometryProviderI() = default;

      //! Allow subclasses to gain access to sibling functions. Should return 'this'.
      //! Must implement.
      virtual GeometryProviderI*  asGeometryProvider() = 0;


      //--------------------------------------------
      // Op attachement:

      //! If the interface is attached to an Op return it. Should return 'this' if attached
      //! to an Op, otherwise null.
      //! Must implement.
      virtual Op*  getGeometryProviderOp() = 0;

      //! Same as getGeometryProviderOp() but const.
      Op*  getGeometryProviderOp() const { return const_cast<GeometryProviderI*>(this)->getGeometryProviderOp(); }


      //--------------------------------------------
      // Geometry state:

      //! Compose state of output geometry.
      //! Subclass should return a hash which represents the composition state (define geometry)
      //! of the stage it could produce. Multiple calls to this method should return exactly
      //! the same hash as long as a built stage does not need to be recomposed.
      //!
      //! Must implement.
      virtual fdk::Hash  geometryComposeState() const = 0;

      //! Edit state of output layer(s).
      //! Subclass should return a hash which represents the edit state (modify values)
      //! of the layers it will produce. Multiple calls to this method should return exactly the
      //! same hash as long as the resulting Layer attribute can be appended to. When this hash
      //! changes the contents of all attributes in the layer are likely invalid and any time
      //! samples cleared out.
      //!
      //! Must implement.
      virtual fdk::Hash  geometryEditState() const = 0;

      //! Does the geometry state have contributions from an animating source?
      //! A subclass should either return true if it doesn't know for certain, or determine the
      //! animation state from local parameters and input info.
      //!
      //! This is a hint to downstream nodes that the per-frame evaluated hash state of an Op
      //! will likely vary, it does not indicate that the actual geometry data changes over time.
      //!
      //! Must implement.
      virtual bool  geometryStateVaries() const = 0;


      //--------------------------------------------
      // Layer output:

      //! Build a usg::Layer, which is owned by the GeometryProviderI subclass.
      //! \a sampleTimes is for optional samples that the caller might need explicit time
      //! samples for, but this behavior is dependent on the subclass implementation.
      //!
      //! If \a appendTo is false (the default) then the subclass is assumed to be initializing
      //! its Layer and defining prims and attributes.
      //! If \a appendTo is true then the subclass should only add additional time samples to
      //! the previously-defined attributes and avoid changing the existence of prims or
      //! attributes as this would cause a Stage using the Layer to recompose.
      //!
      //! Must implement.
      virtual usg::LayerRef  buildGeometryLayer(bool                     appendTo = false,
                                                const fdk::TimeValueSet& sampleTimes = fdk::TimeValueSet()) = 0;

      //! Provide a fully populated usg::Layer. \a sampleTimes is for optional sample times
      //! that a consumer might need explicit time samples for, but this behavior is dependent
      //! on the subclass implementation of buildGeometryLayer().
      //!
      //! Default implementation calls buildGeometryLayer() with a local LayerRef and returns it.
      //! This is virtual to allow a subclass to do something more clever.
      virtual usg::LayerRef  getGeometryLayer(const fdk::TimeValueSet& sampleTimes = fdk::TimeValueSet());


      //--------------------------------------------
      // Stage building:

      //! Return true if the Op this is attached to can build a usg::Stage via buildGeometryStage().
      //!
      //! Default is not to provide Stage building.
      virtual bool  canProvideGeometryStage() const { return false; }

      //! Build the provided usg::Stage. \a sampleTimes is for optional sample times that the caller
      //! might want explicit time samples for. However whether those additional times are filled
      //! in is depedent on the subclass implementation of buildGeometryStage().
      //!
      //! Default implementation creates the stage from the layer built from calling
      //! buildGeometryLayer(). It compares the geometryComposeState() against the provided
      //! stage's composeState and if the same the stage won't be created then buildGeometryLayer()
      //! will be called in appendTo=true mode so the layer attribute contents can be modified
      //! without stage recomposition. If newly created the stage is set to LoadAll mode and its
      //! composeState set to the current geometryComposeState().
      virtual void  buildGeometryStage(usg::StageRef&           stage,
                                       const usg::ArgSet&       requestArgs,
                                       const fdk::TimeValueSet& sampleTimes = fdk::TimeValueSet());

      //! Provide a fully composed usg::Stage. A subclass implementing this 
      //!
      //! \a sampleTimes is for the sample times that the caller may want explicit time samples
      //! for. However whether those additional times are filled in is depedent on the subclass
      //! implementation of buildGeometryStage().
      //!
      //! If the stage consumer wants to call the stage building repeatedly with different sample
      //! times then buildGeometryStage() should be used instead.
      //!
      //! Default implementation calls buildGeometryStage() with a local StageRef and returns it.
      //! This is virtual to allow a subclass to do something more clever.
      virtual usg::StageRef  getGeometryStage(const fdk::TimeValueSet& sampleTimes = fdk::TimeValueSet());

    };

  }
}

#endif
