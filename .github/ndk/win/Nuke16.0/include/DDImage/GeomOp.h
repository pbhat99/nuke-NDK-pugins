//------------------------------------------------------------------------------
// DDImage/GeomOp.h
//
// Copyright (c) 2021 The Foundry Visionmongers Ltd.  All Rights Reserved.
//------------------------------------------------------------------------------

#ifndef DDIMAGE_GEOMOP_H
#define DDIMAGE_GEOMOP_H

#include "DDImage/GeometryProviderI.h"

#include "DDImage/Authoring.h"
#include "DDImage/Op.h"
#include "DDImage/Knobs.h"
#include "DDImage/GeneralInfo.h"
#include "DDImage/ViewerContext.h"
#include "DDImage/Execute.h"

#include "usg/base/ArgSet.h"
#include "usg/base/ArrayTypes.h"
#include "usg/base/Value.h" // for Value::Type
#include "usg/geom/Stage.h"
#include "usg/geom/GeomSceneOutput.h" // convenience for GeomOp plugins

#include "usg/engine/GeomEngine.h"
#include "usg/engine/GeomStateTarget.h"
#include "usg/engine/GeomSceneContext.h" // convenience for GeomOp plugins

#include <functional>
#include <vector>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <type_traits> // for is_base_of


namespace usg {
  class ShaderDescGroup;
  using ShaderDescGroupPtrMap = std::unordered_map<std::string, ShaderDescGroup*>;
}


namespace DD {
  namespace Image {

    //--------------------------------------------------------------------------------

    class GeomOp;
    using GeomOpList            = std::vector<GeomOp*>;
    using GeomOpSet             = std::unordered_set<GeomOp*>;
    using GeomStateTargetIdxMap = std::unordered_map<usg::Token, uint32_t>;

    //--------------------------------------------------------------------------------

    /*! For passing knob value snapshots between GeomOp and GeomOpEngine.

        A GeomOpEngine subclass defines a subclassed local version of this
        struct filled with the variables that Knobs will store into
        at different frames.

        TODO ndk14: Each GeomKnobSample needs to remember what view it's configured
        for so it can pass that on to the Knob functions without additional junk.
    */
    struct DDImage_API GeomKnobSample
    {
      fdk::TimeValue time;

      GeomKnobSample(fdk::TimeValue t = fdk::defaultTimeValue()) : time(t) {}

      //! Does this sample's time represent an explicit time sample?
      virtual bool  isAnimated() const { return !fdk::isDefaultTimeValue(time); }

      // Equality only tests time (for sorting.)
      bool operator == (const GeomKnobSample& b) const { return (time == b.time); }
      bool operator != (const GeomKnobSample& b) const { return (time != b.time); }
      // Greater/less-than only tests time (for sorting.)
      bool operator <  (const GeomKnobSample& b) const { return (time < b.time); }
      bool operator >  (const GeomKnobSample& b) const { return (time > b.time); }

    };


    //--------------------------------------------------------------------------------


    /*! Scenegraph processing engine node that can build a usg::Stage.
        One of these is created and shared between all GeomOps generated from the same
        Nuke Node.
    */
    class DDImage_API GeomOpEngine : public usg::GeomEngine
    {
    public:
      //! 'parent' is automatically provided when the first GeomOpEngine request is made.
      using Builder = std::function<GeomOpEngine*(Op*)>;

      //! Combination of input Op and its OutputContext when cached in map.
      using OpInput            = std::pair<Op*, OutputContext>;
      using OpInputTimeMap     = std::map<double, OpInput>;
      using OpInputTimeMapList = std::vector<OpInputTimeMap>;
      using OpInputTimeWarpMap = std::map<double, double>;

      //----------------------------------------------------------------------

      /*! Nuke Node input binding. Note that this is *NOT* an Op input!

          If \a _upstreamEngine is not null the InputBinding is connected to a GeomOp with
          a GeomOpEngine. \a _upstreamFirstOp will be a GeomOp, and \a _upstreamEngine can
          be called directly.

          Otherwise \a _upstreamFirstOp will be some other Op type and can be type tested.

          In both cases \a _upstreamOpsPerView will contain the list of Ops at the
          requested OutputContexts.
      */
      class InputBinding
      {
      public:
        //! Default ctor makes an invalid binding.
        InputBinding();

        //! Node input number
        uint32_t  nodeInput() const { return _nodeInput; }

        //------------------------------------------------------------

        //! Return true if the InputBinding has a valid input Op assignment.
        bool  isValid() const { return (_upstreamFirstOp != nullptr); }

        /*! Returns a GeomOpEngine pointer if the connected input object is another
            GeomOpEngine (a GeomEngine).
        */
        GeomOpEngine*  geomEngine() const { return _upstreamEngine; }

        /*! If the input is an Op type, return its Op::firstOp(), which may not be at any
            specific desired OutputContext. To get a specific frame/view Op variant use
            the opAt() and opAtContext() methods.
        */
        Op*  baseOp() const { return _upstreamFirstOp; }

        //------------------------------------------------------------

        /*! Possibly remap the member GeomOp frame to a time-warped input Op frame.
            Also rounds the frame to three decimal places (ie .001, a thousandth of a frame)
        */
        double inputFrameForOpTime(double member_geop_frame) const;

        /*! Returns the matching Op input for a specific frame and view, if one exists.
            The frame number is rounded to three decimal places (ie .001, a thousandth of a frame)
            to make it more likely that a match will occur, and possibly remapped.
            If view is non-valid then the 'main' view (view index 1) is used instead.
        */
        Op*  opAt(double frame, int32_t view = -1) const;

        /*! Returns the matching Op input for the provided OutputContext (frame & view)
            if one exists.
            The OutputContext's frame number is rounded to three decimal places
            (ie .001, a thousandth of a frame) to make it more likely that a match
            will occur.
        */
        Op*  opAtContext(const OutputContext& ctx) const;

        //------------------------------------------------------------

        //! Starting point of Node input index in Op input count.
        uint32_t  opIndexStart() const { return _opInputStart; }

        //! Op split input count for this Nuke Node input
        uint32_t  opSplitCount() const { return _opSplitCount; }

        /*! Returns the time/InputOp map for a specific view. If the view doesn't
            exists the map will be empty.
        */
        OpInputTimeMap&  opsForView(int32_t view);


      private:
        // Allow access to GeomOpEngine to build InputBindings, but restrict
        // GeomOpEngine subclasses from messing with the assigned internals.
        friend class GeomOpEngine;
        friend DDImage_API std::ostream& operator << (std::ostream&, const InputBinding&);

        uint32_t            _nodeInput;             //!< The index of the Nuke Node input
        GeomOpEngine*       _upstreamEngine;        //!< Upstream GeomOpEngine, if there is one.
        Op*                 _upstreamFirstOp;       //!< Upstream Node's firstOp
        uint32_t            _opInputStart;          //!< Starting point of Node input index in Op input count
        uint32_t            _opSplitCount;          //!< Op split input count for this Nuke Node input
        OpInputTimeMapList  _upstreamOpsPerView;    //!< All upstream Ops for this Nuke Node input
        OpInputTimeWarpMap  _inputTimeMap;          //!< For remapping time-warped Op inputs

      };
      using InputBindingList = std::vector<InputBinding>;

      //----------------------------------------------------------------------

      //! A class which gives a read handle onto a Knob, allowing GeomEngines to fetch knob values
      //! and query animation state. This should be used in preference to raw knob pointers which are not
      //! safe to use from GeomEngine.
      class KnobBinding {
      public:
        KnobBinding() {}

        KnobBinding(Knob* knob) : _knob(knob) {}

        KnobBinding(const KnobBinding& other) = default;

        //! Returns true if the reference to the knob is valid
        operator bool() const { return _knob; }

        //! Returns the bound knob, if there is one.
        Knob* knob() const { return _knob; }

        //! Returns true if the knob is animated
        bool isAnimated() const { return _knob->isAnimated(); }

        //! Returns true if the knob is not animated
        bool isUniform() const { return !_knob->isAnimated(); }

        //! Returns the authoring mode of the knob
        const Authoring::Mode getAuthoringMode() const { return _knob->getAuthoring()->getMode(); }

        //! Returns the value of the knob at the given OutputContext
        template<class T>
        T get(const OutputContext* ctx) const { return _knob->get<T>(ctx); }

        //! Returns the value of the knob at the given time and view
        template<class T>
        T get(const fdk::TimeValue time = fdk::defaultTimeValue(), int view = -1) const;

      private:
        Knob* _knob;
      };

      //----------------------------------------------------------------------


    protected:
      // Parent Op is only used to retrieve the Op::firstOp() from parent GeomOp Node class.
      GeomOpEngine(Op* parent);


    public:
      virtual ~GeomOpEngine();

      /*! Default returns nodeName() from \a _firstOp. Since the name of the Nuke
          Node can change dynamically we don't save this string locally.
      */
      std::string  name() const override;

      //----------------------------------------------------------------------

      /*! Only use this to access values that *ALL* GeomOp instances would share,
          ie non-animating, non-viewsplit values, or to access Nuke Node methods.
      */
      Op*          firstOp() const { return _firstOp; }

      //! Add to the node's GeomOp member set.
      void         addMemberGeomOp(GeomOp* geop);
      //! Remove from the node's GeomOp member set.
      void         removeMemberGeomOp(GeomOp* geop);

      /*! Read access to all member GeomOps assigned to this engine. Note that
          some GeomOps may be disabled by Nuke and should not be assumed to be
          usable just because they're in this set. If the GeomOp's OutputContext
          view is < 0 (usually -1) then it is disabled and not usable.
      */
      const GeomOpSet&  memberGeomOps() const { return _memberGeomOps; }

      /*! Retrieve a valid GeomOp instance sharing this engine. This iterates over
          the \a _memberGeomOps set skipping disabled GeoOps (ones with
          outputContext().view() = -1,) so it's not the quickest routine.

          No mutex lock is performed on set read - it's assumed that adding GeomOp
          instances is done on the main thread calls in GeomOp engineBuilder ctors().

          Only use these GeomOps to access values that *ALL* instances would share,
          usually non-animating, non-viewsplit values, or non-Knob related methods.

          The frame number is rounded to three decimal places (ie .001, a thousandth of a frame)
          to make it more likely that a match will occur.
          If view is non-valid then the 'main' view (view index 1) is used instead.
      */
      GeomOp*      geomOpAt(double  frame,
                            int32_t view = -1) const;
      //! Same as geomOpAt() but takes OutputContext.
      GeomOp*      geomOpAtContext(const OutputContext& context) const;

      //! Find all the GeomOps matching the views and times.
      GeomOpSet    geomOpsFor(const std::set<int32_t>& views,
                              const fdk::TimeValueSet& times) const;

      //----------------------------------------------------------------------

      //! Return a read handle to the named Knob from the engine.
      KnobBinding knob(const char* knob_name) const { return KnobBinding(_firstOp->knob(knob_name)); }

      //! Convenience method to get a value from a KnobBinding, if it's valid.
      template<typename T>
      T     getValue(const KnobBinding& knob) const;

      //! Convenience method to get a value from a named Knob, if it exists.
      template<typename T>
      T     getValue(const char* knobName) const;

      //! Convenience method to get a value from a named Knob, if it exists, otherwise return defaultValue.
      template<typename T>
      T     getValue(const T&    defaultValue,
                     const char* knobName,
                     double      frame,
                     int         view = -1) const;
      //! getValue() variant for uniform (non-animating) Knobs, or if the default uiContext() is desired.
      template<typename T>
      T     getValue(const T&    defaultValue,
                     const char* knobName) const;

      template<typename T>
      void  getValue(T&          v,
                     const char* knobName,
                     double      frame,
                     int         view = -1) const;
      //! getValue(&T) variant for uniform (non-animating) Knobs, or if the default uiContext() is desired.
      template<typename T>
      void  getValue(T&          v,
                     const char* knobName) const;

      //! Convenience method to get a value from a KnobBinding, if it's valid, otherwise return defaultValue.
      template<typename T>
      T     getValue(const T&           defaultValue,
                     const KnobBinding& knob,
                     double             frame,
                     int                view = -1) const;
      //! getValue() variant for uniform (non-animating) Knobs, or if the default uiContext() is desired.
      template<typename T>
      T     getValue(const T&           defaultValue,
                     const KnobBinding& knob) const;


      //----------------------------------------------------------------------

      /*! Returns an Op for input \a node_input at a specific \a frame and \a OutputContext
          (for the view and other context vars,) generating the Op if it's not already cached.

          This method does not limit access to input Ops only via the limited number of shared
          GeomOp instances so the engine is free to access inputs at arbitrary times, bearing
          in mind the possible performance costs of doing so. If an Op does not already exist
          Op::node_input() is called with EXECUTABLE_INPUT mode to create it or if \a execute
          is supplied then generateOp() is called on this to ensure existing ops are not recycled.
          For complex/large Op graphs this can be costly as the Op's inputs may also need to be 
          generated.

          Note - the frame number in the \a ctx arg is -ignored- and the \a frame arg is always
          used. The frame number is rounded to three decimal places (ie .001, a thousandth
          of a frame) to make it more likely that a subsequent retrieval match will occur.
          If frame is the 'default' frame value this returns the first Op in the list, usually
          the earliest in time.

          Not thread safe - assuming this is called on main thread.
      */
      Op*     inputOpAt(uint32_t             node_input,
                        double               frame,
                        const OutputContext& ctx, 
                        Execute* execute = nullptr);
      /*! Same as inputOpAt() but for a single view. If view is -1 (invalid) then the 'main'
          view (view index 1) is used instead.

          Not thread safe - assuming this is called on main thread.
      */
      Op*     inputOpAt(uint32_t node_input,
                        double   frame,
                        int32_t  view = -1,
                        Execute* execute = nullptr);

      /*! Return the number of Node InputBinding connections. Note that these
          are NOT Op inputs. The total number of Op inputs this Node produces
          can be found by totalling the op_split_count of all InputBindings,
          which is what nInputOps() returns.
      */
      size_t  nInputs() const override { return _inputs.size(); }

      //! Returns the sum of all InputBinding op_split_counts.
      size_t  nInputOps() const;


      /*! Returns the InputBinding for the node_input.
          If node_input is out of range an invalid InputBinding is returned.
      */
      const InputBinding&  inputBinding(uint32_t node_input) const;

      /*! Returns the GeomOpEngine pointer from the node_input InputBinding.
          This will be null if the input is not a GeomOp Node or an empty GeomOpEngine
          (a NullGeomOp) if the input is disconnected.
      */
      GeomEngine*  inputEngine(uint32_t node_input) const override { return inputBinding(node_input).geomEngine(); }
      GeomEngine*  inputEngine0()                   const override { return inputBinding(0).geomEngine(); }
      GeomEngine*  inputEngine1()                   const override { return inputBinding(1).geomEngine(); }


      /*! Returns the Op::firstOp() pointer from the node_input InputBinding.
          Warning, the firstOp may not be valid for a specific OutputContext that
          the engine is processing. It is ok to use for generic Node or Knob queries
          and Node input info - see the Op::firstOp() method for more info.

          Use inputOpAt() and inputOpAtContext() to get a specific valid input Op.
      */
      Op*  inputBaseOp(uint32_t node_input) { return inputBinding(node_input).baseOp(); }


      //! Prints the contents of the current InputBindings.
      void  printInputBindings(std::ostream&,
                               const char* prefix ="") const;

      //----------------------------------------------------------------------

      //! This just sets the nuke:node data on the edit layer so we can go back from the Layer to the Node that created it.
      void  processScenegraph(usg::GeomSceneContext& context) override;

      /*! Update the state of the GeomOpEngine in preparation for calling buildScenegraph()
          and processScenegraph().

          The GeomOpSet is the set of desired GeomOps that should be used to update the
          engine's state, and \a active_times will be filled in for each valid GeomOp that
          gets used to construct the state.

          This method then calls the base class GeomEngine::validateGeomState() and
          any local GeomState changes will be reflected in the engine's dirty status.
      */
      void  updateGeomEngineState(const usg::ArgSet& requestArgs,
                                  const GeomOpSet&   geops,
                                  fdk::TimeValueSet& activeTimes);

      /*! The 'request' hash is used to track the evaluation or configuration state
          of the GeomEngine by the GeomOp network.

          The request hash is built in buildGeometryStage(GeomOpSet) from the set of
          GeomOps passed to it. If those GeomOps are the same and their hashes haven't
          changed, then the state of the GeomEngine and its input network is assumed
          to be unchanged.
      */
      const fdk::Hash&  lastRequestHash() const { return _lastRequestHash; }
      //! Assign the value of the request hash, usually done in buildGeometryStage(GeomOpSet).
      void              setRequestHash(const fdk::Hash& hash) { _lastRequestHash = hash; }

      /*! Converts the input Op(s) to surface shader and material Prims, defined in the
          edit Layer anchored under \a material_library_path.

          When \a create_mode is false this method will update any existing overrides for the
          shader and material prims that were created during the \a create_mode=true phase.
          Overrides are only written if the shader/material parameters are not at their
          inital default.

          If the supplied Ops are ShaderOps (have a MaterialOpI interface) then the
          shaders are created through that interface. If the input is an Iop then it's
          treated as a texture source to a default constant (albedo only) shader. Legacy
          Material ops are NOT supported and will be ignored.

          The \a material_group_index is used to track the 'output' ShaderDesc from each material
          graph in the \a _shaderDescGroups map, but it doesn't need to match an actual
          GeomOpEngine input number. Typically there will be only one input material graph so
          material_group_index will usually be 0.

          Returns the path to the created or updated MaterialPrim, if successful.
      */
      virtual usg::Path importShaderOp(bool                    create_mode,
                                       usg::GeomSceneContext&  scene_context,
                                       const std::vector<Op*>& source_ops,
                                       uint32_t                material_group_index,
                                       const usg::Path&        material_library_path);

      /*! Assign the material to a list of Prims. Companion method to importShaderOp() usually
          called after it to assign the created material to a list of previously-created Prims.

          When \a create_mode is true each Prim is retrieved from the engine's editLayer() and
          a Relationship created on the Prim.
          When \a create_mode is false each Prim is retrieved from the composed stage flowing
          through this node, and a Relationship matching \a material_path is searched for and
          updated.
      */
      virtual void      assignMaterialBinding(usg::GeomSceneContext& context,
                                              bool                   create_mode,
                                              const usg::Path&       material_path,
                                              const usg::PathArray&  primPaths);

    protected:
      friend class GeomOp;

      Op*         _firstOp;           //!< Op that's guaranteed to exist and offer access to Nuke Node
      GeomOpSet   _memberGeomOps;     //!< All GeomOps sharing this engine - NOT the current active engine set!
      fdk::Hash   _lastVersionsHash;  //!< The last combined GeomOp versions hashes
      fdk::Hash   _lastRequestHash;   //!< The last combined GeomOp hashes
      //
      usg::ShaderDescGroupPtrMap _shaderDescGroups; //!< Groups of locally-created ShaderDescs

      //---------------------------------------------------------------------------------


      //! Sets the owner node data in the edit layer to point to the owning node
      void setOwnerNode();
      
    private:
      InputBindingList  _inputs;        //!< Input bindings
      int32_t           _treeVersion;   //!< Current global Nuke treeVersion

      //! Gather GeomOps that map to the same GeomOpEngine by Node input index vs. Op input index.
      void updateInputBindings();


    }; // GeomOpEngine


    //--------------------------------------------------------------------------------


    /*! \class DD::Image::GeomOp

        Base class for an Op that processes a usg::Stage containing scenegraph and
        3D geometry data.

        This Op subclass acts somewhat different from other Nuke Ops as it actively
        avoids processing in a time/view sampled mode. Since stage scenegraph prims
        and attributes contain their own time sampled values and don't support Nuke's
        concept of 'views', Nuke's default behavior of splitting Ops into explicit
        time/view instances means there is no obvious 'primary' or principal GeomOp
        to process the scenegraph with.
        For example if a TimeWarp node asks Nuke for five split input GeomOps at
        different times Nuke will create those GeomOps and sample their Knobs at each
        unique time. Normally each of those GeomOps are not aware of each other and
        would need to individually process the scenegraph to produce their output,
        possibly doing 4x more work than neccessary.

        To avoid this all GeomOps Nuke creates from the same Node share a single
        GeomOpEngine subclass which is what does the actual scenegraph processing. If
        any of the GeomOps are asked to build a stage the same GeomOpEngine instance
        is used, the same stage object processes, and the results cached so the next
        GeomOp will produce the same result.

        The scene data is created by calling the static GeometryProviderI::buildStage()
        method with the set of GeomOps constituting the 'request set' which is typically
        the set of input GeomOps feeding into the consumer node - ie. a viewer, renderer, etc.
        Combining the OutputContexts of all GeomOps in the set defines the frames & views
        that the stage should be explicitly evaluated for in the GeomOpEngines, however
        the GeomOpEngine is not restricted to only processing those times & views.
        For example if a renderer intends to output a single linear motionblur
        step it would request two GeomOps, one at shutter-open and one at
        shutter-close, but each GeomOpEngine is free to create no requested samples,
        all the requested samples, more samples than was requested, etc.

        The primary advantage of this 'timeless' scheme is that Prims passing through the
        GeomOpEngines are not stripped of their existing time samples or restricted to just
        the requested set. So if an animated mesh is imported and then transformed, the
        GeomOpEngine applying the transform does not automatically strip the imported Prims
        of their animation data which is then made available to the downstream consumer
        regardless of the explicitly requested time samples.

    */
    class DDImage_API GeomOp : public Op
                             , public GeneralInfoProvider
                             , public GeometryProviderI
    {
    protected:
      /*! Base class constructor requires a GeomOpEngine builder method, only
          subclasses can create an instance.
          The Engine builder method is usually declared in a GeomOp subclass like so:

            class MyGeomOp : public GeomOp
            {
              class MyEngine : public Engine
              { 
                MyEngine(Op* parent) : Engine(parent) {}

                void processScenegraph(usg::GeomSceneContext& context) override
                {
                  do something cool
                }
              };
              static Engine* engineBuilder(Op* parent) { return new MyEngine(parent); }

              MyGeomOp(Node* node) : GeomOp(node, engineBuilder) {}
            };
      */
      GeomOp(Node*                 node,
             GeomOpEngine::Builder engineBuilder);

      //! Ctor for MultiTypeNode support only! Do not implement for Engine-using subclasses.
      GeomOp(Node*);

      //! Return hints to control when and how this op will be evaluated by the top-down system.
      OpHints opHints() const override;

      //--------------------------------------------------------------------------------

      //! A helper function you can use as the \a engineBuilder argument to the GeomOp constructor to reduce clutter.
      template<class Engine>
      static GeomOpEngine::Builder BuildEngine()
      {
        return [](Op* parent) { return new Engine(parent); };
      }


    public:
      virtual ~GeomOp();

      //!
      const GeneralInfo& generalInfo() const override { return info_; }

      //! Cast to a GeomOp. This is much cheaper and safer than using dynamic_cast.
      const GeomOp*  geomOp() const override { return this; }
            GeomOp*  geomOp()       override { return this; }


      //!
      GeomOpEngine*  engineNode() const { return _engineNode; }


      //----------------------------------------------------------------------------
      // GeometryProviderI handling
      //----------------------------------------------------------------------------

      //! GeometryProviderI:  Return the Op the interface is attached to.
      Op*  getGeometryProviderOp() override { return this; }

      //! GeometryProviderI: Allow subclasses to gain access to sibling functions.
      GeometryProviderI*  asGeometryProvider() override { return this; }

      //! Cast to a GeometryProviderI interface. This is much cheaper and safer than using dynamic_cast.
            GeometryProviderI* geometryProvider()       override { return asGeometryProvider(); }
      const GeometryProviderI* geometryProvider() const override { return const_cast<GeomOp*>(this)->asGeometryProvider(); }

      //! Returns the compose state of the GeomOpEngine.
      fdk::Hash  geometryComposeState() const override;

      //! Returns the edit state of the GeomOpEngine.
      fdk::Hash  geometryEditState() const override;

      //! Returns the animating state of the GeomOpEngine.
      bool  geometryStateVaries() const override;

      //! Base class returns an empty layer.
      usg::LayerRef buildGeometryLayer(bool, const fdk::TimeValueSet&) override { return usg::LayerRef(); }

      //! Provide a fully constructed usg::Stage via getGeometryStage().
      bool canProvideGeometryStage() const override { return true; }

      //! Build the provided usg::Stage which is owned by the caller.
      //! Base class calls buildGeometryStage(GeomOpSet).
      void buildGeometryStage(usg::StageRef&           stage,
                              const usg::ArgSet&       requestArgs,
                              const fdk::TimeValueSet& sampleTimes = fdk::TimeValueSet()) override;

      //! Specialized GeomOp version with a set of GeomOps for the GeomOpEngine to validate.
      void buildGeometryStage(usg::StageRef&           stage,
                              const usg::ArgSet&       requestArgs,
                              const GeomOpSet&         geomops,
                              const fdk::TimeValueSet& processTimes);


      //------------------------------------------------------
      // Nuke Node methods
      //------------------------------------------------------

      //! Draw a round sided box.
      const char*  node_shape() const override;
      unsigned     node_color() const override;

      //! Minimum number of Node inputs for this class - GeomOp base class has only one input.
      int          minimum_inputs() const override { return 1; }

      //! Maximum number of Node inputs for this class.
      int          maximum_inputs() const override { return 1; }

      //! Return a default NullGeomOp op for Node input0.
      Op*          default_input(int node_input) const override;

      //! Allows only GeomOp inputs by default.
      bool         test_input(int node_input,
                              Op* op) const override;

      //! Returns the node input to use for the merge geometry source. Always defaults to input 0.
      virtual int mergeInput() const;

      //! Returns the merge input label for use in Op::input_label()
      virtual const char* mergeInputLabel() const;

      //! For use in Op::test_input(). Allows connections to GeomOp and GeometryProviderI ops.
      virtual bool testMergeInput(Op* op) const;

      //------------------------------------------------------
      // Knobs
      //------------------------------------------------------

      /*! Correlate a Knob value with a GeomOpEngine manipulation, specified by the
          GeomStateTarget \a target_id identifier.

          This method associates the Knob with a named GeomStateTarget owned by the
          GeomOpEngine which then affects the engine's local GeomState. The current
          edit state and hash of the knob, calculated during Knob::store(), is appended to
          the version state and hash of the named target.

          Call this in the knobs() method right after defining a knob which is intended to
          affect the geometry output in some way.

          For example in the knobs() method of a GeomOp subclass that wants a 'scale'
          parameter to modify existing Prim attributes:
            XYZ_knob(f, &params.scale.x, "scale");
              AppendKnobToTarget(f, usg::EngineTokens.target.modifyValues);

          The GeomOpEngine class has built-in targets which trigger GeomState flags:
            'define-geometry' (usg::EngineTokens.target.defineGeometry)
            'modify-values'   (usg::EngineTokens.target.modifyValues)
            'edit-stage'      (usg::EngineTokens.target.editStage)
            'affect-stereo'   (usg::EngineTokens.target.affectStereo)
          See usg::GeomStateTarget for more info.

          You must do this after the knob to be bound's declaration, as the Knob_Callback
          var passed to the declaration method (Int_knob(f), XYZ_knob(f), etc) contains
          the results from the most recent knob store(), like the frame, view and edit
          state of the knob. This info is used during binding to track the animation state
          of the knob.
      */
      virtual void  AppendKnobToTarget(const Knob_Closure& f,
                                       const usg::Token&   target_id);

      /*! Like AppendKnobToTarget() but for knobs with no current Op OutputContext (ie no frame
          or view.) Intended for global non-animating Node knobs like 'name' and 'tile_color'.
      */
      virtual void  AppendKnobToTarget(const char*       knob_name,
                                       const usg::Token& target_id);


      /*! Use this method if the Knob's value will affect the 3D scenegraph topology by
          adding/removing Prims or Attributes, or by changing their names or paths in the
          scenegraph.

          Convenience method that calls AppendKnobToTarget() with the 'define-geometry' target.

          Important note - knob changes affecting the GeomState::defineGeometryState will
          cause Stage recomposition whenever the Knob's value changes. Stage recomposition
          can be expensive (slow) so ONLY bind this knob to GeomState::defineGeometryState
          when stage recomposition is absolutely required.
          If the knob is just changing the contents of an existing attribute rather than
          explicitly creating that attribute, use KnobModifiesAttribValues() instead.
      */
      void  KnobDefinesGeometry(const Knob_Closure& f);

      /*! Use this method if the Knob's value is changing the contents of an existing
          attribute. The most common type of geometry manipulation.

          Convenience method that calls AppendKnobToTarget() with the 'modify-values' target
          and appendToDefineGeometryTarget() with the Knob's authoring mode.
      */
      void  KnobModifiesAttribValues(const Knob_Closure& f);

      /*! Use this method if the Knob's value is performing non-specific changes to the engine's
          editLayer().

          Convenience method that calls AppendKnobToTarget() with the 'edit-layer' target.
      */
      void  KnobEditsLayer(const Knob_Closure& f);

      /*! Use this method if the Knob's value is performing non-specific changes to the stage
          passing through, or generated by, the engine.

          Convenience method that calls AppendKnobToTarget() with the 'edit-stage' target.
      */
      void  KnobEditStage(const Knob_Closure& f);

      //----------------------------------------------

      /*! Convenience to get a Knob pointer and retrieve its value in one step.
          See Knob::get() for more info.
          Same as calling:
            T value; Knob* k = knob("foo"); if (k) value = k->get<T>(context);
      */
      template <typename T>
      T     getKnobValue(const char*          knob_name,
                         const OutputContext& context,
                         const T&             default_value) const;

      /*! Convenience to get a Knob pointer and set its value in one step.
          See Knob::set() for more info.
          Same as calling:
            Knob* k = knob("foo"); if (k) k->set(value, time, element_offset, view);
          Return true on success.
      */
      template <typename T>
      bool  setKnobValue(const char* knob_name,
                         const T&    value,
                         double      time,
                         int32_t     element_offset = 0,
                         int32_t     view = -1);

      /*! Convenience to get a Knob pointer and set its values in one step.
          See Knob::set() for more info.
          Same as calling:
            Knob* k = knob("foo"); if (k) k->set(clear_keys, values, times, element_offset, view);
          Return true on success.
      */
      template <typename T>
      bool  setKnobValues(const char*                knob_name,
                          bool                       clear_keys,
                          const std::vector<T>&      values,
                          const std::vector<double>& times,
                          int32_t                    element_offset = 0,
                          int32_t                    view = -1);

      //------------------------------------------------------
      // Nuke Node input handling
      //------------------------------------------------------

      //!
      const OutputContext&  inputContext(int            node_input,
                                         int            offset,
                                         OutputContext& scratch_ctx) const override;
      //!
      const OutputContext*  inputUIContext(int            node_input,
                                           OutputContext& scratch_ctx) const override;

      //------------------------------------------------------------------
      // GeomOp-typed Op inputs
      //
      // Avoid using direct Op queries unless you need to peek at the raw
      // Op connections before validateGeomState() has been called.
      // After validateGeomState() has been called use the GeomOpEngine
      // input methods instead as these contain the gathered up frames and
      // views for the current graph state.
      //------------------------------------------------------------------

      GeomOp*  input0() const { return op_cast<GeomOp*>(Op::input0()); }
      GeomOp*  input1() const { return op_cast<GeomOp*>(Op::input1()); }

      //----------------------------------------------

      /*! Whether the geometry the node is managing should be selectable in the Viewer.
          Base class returns true.
      */
      virtual bool   selectable()  const;

      virtual bool   selection3DChanged(ViewerContext* ctx)   { return false; }
      virtual bool   selection3DCleared()                     { return false; }
      virtual bool   selection3DMouseDown(ViewerContext* ctx) { return false; }
      virtual bool   selection3DMouseUp(ViewerContext* ctx)   { return false; }
      virtual bool   selection3DMouseDrag(ViewerContext* ctx) { return false; }


      //------------------------------------------------------
      //
      //------------------------------------------------------

      //!
      void           build_handles(ViewerContext* ctx) override;

      /*! Build_handles with the context model matrix multiplied by
          the (global) local matrix so nodes up the tree are
          displayed correctly. 
      */
      virtual void   buildXformHandles(ViewerContext*    ctx,
                                       const fdk::Mat4d& localMatrix);


      //----------------------------------------------

      const usg::GeomState&  inputVersions()  const { return _inputVersions; }
      const usg::GeomState&  localVersions()  const { return _localVersions; }
      const usg::GeomState&  outputVersions() const { return _outputVersions; }

      const usg::GeomState&  inputGeomState()  const { return _inputGeomState; }
      const usg::GeomState&  localGeomState()  const { return _localGeomState; }
      const usg::GeomState&  outputGeomState() const { return _outputGeomState; }

      /*! Clear the GeomOp's local GeomStateTarget hashes which get built in knobs()
          and appendGeomState().

          Warning - do not call this method unless you know exactly what you're doing
          otherwise unexpected behavior of the geometry system may result.

          This is called by the non-virtual portion of Op::knobStorePrep().
      */
      void  resetGeomState();

      /*! Make changes to the GeomStateTargets registered to this GeomEngine which
          contribute to the dirtiness(out-of-dateness) state of the layers and stage
          this node affects or processes.

          Warning - do not call this method unless you know exactly what you're doing
          otherwise unexpected behavior of the geometry system may result.

          This is called by the non-virtual portion of Op::knobStorePost().

          Each GeomStateTarget affects a specific aspect of the 3D scene and there are
          several pre-defined ones such as 'define-geometry' and 'modify-values' which
          in turn affect Hash values in the GeomState stored in the GeomEngine.

          Knob values are typically used to build a Hash which is then appended to the
          GeomStateTargets the knob wants to influence, usually at the Op's current
          OutputContext. This should be done in the knobs() method.

          For arbitrary hash contributions to GeomStateTargets, such as input hashes,
          subclasses should implement the virtual appendGeomState() method and append
          to the local GeomStateTargets via appendToTarget().
      */
      void  buildGeomState(DD::Image::Hash& op_hash);

      /*! Append a value state hash and edit-version state hash to GeomStateTarget \a target_id
          at the GeomOp's current OutputContext (time & view.)

          If \a is_animating_source is true then the GeomStateTarget is marked as animating
          regardless of whether the hashes are actually changing over time. This is useful
          as a hint when creating geometry attribute time samples.

          Not thread safe - assuming this is called on main thread.
      */
      void  appendToTarget(const usg::Token& target_id,
                           const fdk::Hash&  hash,
                           const fdk::Hash&  version,
                           bool              is_animating_source = false);
      void  appendToTarget(const usg::Token& target_id,
                           const Hash&       hash,
                           const Hash&       version,
                           bool              is_animating_source = false);

      //! Variant that appends op->hash() and uses op->varyingOutputHash() for 'is hash animating' state.
      void  appendToTarget(const usg::Token& target_id, Op* op);

      // Convenience implementations that affect the 'define-geometry' GeomStateTarget:
      void  appendToDefineGeometryTarget(const fdk::Hash& hash, const fdk::Hash& version, bool is_animating_source = false);
      void  appendToDefineGeometryTarget(const Hash&      hash, const Hash&      version, bool is_animating_source = false);
      void  appendToDefineGeometryTarget(Op* op);

      // Convenience implementations that affect the 'modify-values' GeomStateTarget:
      void  appendToModifyValuesTarget(const fdk::Hash& hash, const fdk::Hash& version, bool is_animating_source = false);
      void  appendToModifyValuesTarget(const Hash&      hash, const Hash&      version, bool is_animating_source = false);
      void  appendToModifyValuesTarget(Op* op);


      // Convenience implementations that affect the 'edit-stage' GeomStateTarget:
      void  appendToEditStageTarget(const fdk::Hash& hash, const fdk::Hash& version, bool is_animating_source = false);
      void  appendToEditStageTarget(const Hash&      hash, const Hash&      version, bool is_animating_source = false);
      void  appendToEditStageTarget(Op* op);


      //! The set of knob-change targets, copied from the shared GeomOpEngine.
      const usg::GeomStateTargetArray& localTargets() const { return _localTargets; }


    protected:

      //! Called by Nuke immediately AFTER Op::knobs().
      void          setOutputContext(const OutputContext& context) override;

      //!
      int           knob_changed(Knob* k) override;


      /*! Append arbitrary hash contributions to this GeoOp's GeomStateTargets which
          affect the local GeomState. These hash contribution sources are normally from
          non-Knob sources like inputs or some other Op value like frame or view.
          \a op_hash is provided as a convenience to allow the GeomOp to affect the Op's
          output hash at the same time without needing to reimplement the hash calcs in
          Op::append(), which will be called soon afterwards.

          This is called after GeomOp::knobs() so the targets will already include any
          Knob contributions.
          It's called from within GeomOp::buildGeomState() just prior to the construction
          of the GeoOp's \a _inputGeomState, \a _localGeomState, and \a _outputGeomState
          private vars.

          A GeomOp subclass should affect the assigned GeomStateTargets via appendToTarget()
          which indirectly contribute to the \a _localGeomState hash when the target graph
          is queried for its concatenated GeomStateTarget::hash().

          Default implementation handles input0 connections to scene prim exporters.
      */
      virtual void  appendGeomState(DD::Image::Hash& op_hash);


      //! GeomOp subclass-dependent portion of validate().
      //! This calls validate on the inputs and updates GeneralInfo.
      void          _validate(bool for_real) override;

      //! Default implemenation copies the GeneralInfo from input0.
      virtual void  updateGeneralInfo();

      /*! Return the parent transformation for the handles.

          This is called in build_handles() when the handles are building.
      */
      virtual fdk::Mat4d handlesParentTransform(DD::Image::ViewerContext* ctx) { return fdk::Mat4d::getIdentity(); }

      //! Called directly by both build_handles and buildXformHandles 
      //! so can share common code.
      virtual void buildXformHandles(ViewerContext* ctx,
                              const fdk::Mat4d* localMatrix);

      //----------------------------------------------


    protected:
      GeomOpEngine*  _engineNode = nullptr;   //!< GeomEngine graph processor shared between all Op instances of this GeomOp subclass
      GeneralInfo    info_;                   //!< Global intended frame range (must remain 'info_' for MultiTypeNode support!)

      //----------------------------------------------


    private:
      struct TargetHash
      {
        fdk::Hash hash;                     //!< Hash being built for the GeomStateTarget
        fdk::Hash versionHash;              //!< Version state of hash
        bool      animatingSource = false;  //!< If any of the hashes are from an animating source like a Knob
      };
      using TargetHashMap = std::unordered_map<usg::Token, TargetHash>;

      TargetHashMap             _localTargetHashes; //!< 
      usg::GeomStateTargetArray _localTargets;      //!< What the GeomOp/GeomEngine intends to affect
      GeomStateTargetIdxMap     _localTargetsMap;   //!<
      //
      usg::GeomState  _inputVersions;     //!< Input edit versions build from input GeomOps
      usg::GeomState  _localVersions;     //!< Hashes of Knob::editVersion()s contributing to _localGeomState
      usg::GeomState  _outputVersions;    //!< Output edit versions build from _inputVersions + _localVersions
      //
      usg::GeomState  _inputGeomState;    //!< Input GeomState built from input GeomOps.
      usg::GeomState  _localGeomState;    //!< Local GeomState built from Knob hashes, input hashes, etc.
      usg::GeomState  _outputGeomState;   //!< Output GeomState built from _inputGeomState + _localGeomState

    }; // GeomOp


    //--------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------


    /*! This is a stub GeomOp, intended to be the dead-end in a geometry graph.
    */
    class NullGeomOp : public GeomOp
    {
    public:

      /*!
      */
      class NullEngine : public GeomOpEngine
      {
      public:
        NullEngine(Op* parent) : GeomOpEngine(parent) {}

      protected:
        /*virtual*/ std::string name() const override { return std::string("NullEngine"); }

        //! No work to do, do nothing
        void  processScenegraph(usg::GeomSceneContext& context) override {}
      };


    public:
      static GeomOpEngine* engineBuilder(Op* parent);

      NullGeomOp(Node*                 node,
                 GeomOpEngine::Builder engineBuilder) : GeomOp(node, engineBuilder) { inputs(0); }
      NullGeomOp(Node* node) : GeomOp(node) { inputs(0); }

      int          minimum_inputs() const override { return 0; }
      int          maximum_inputs() const override { return 0; }

      const char*  Class()     const override { return "NullGeomOp"; }
      const char*  node_help() const override { return ""; }

    };


    //--------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------

    template<class T>
    inline T GeomOpEngine::KnobBinding::get(const fdk::TimeValue time, int view) const
    {
      static fdk::TimeValue safeDefaultTime(0);

      // Guard against NaN time values. These will produce random result and destroy the animation cache.
      const fdk::TimeValue& safeTime = fdk::isDefaultTimeValue(time) ? 0 : time;
      
      OutputContext ctx = _knob->uiContext();
      ctx.setFrame(safeTime);
      ctx.setView(view);
      return _knob->get<T>(&ctx);
    }

    //-----------------------------------------

    template<typename T>
    inline T
    GeomOpEngine::getValue(const T&           defaultValue,
                           const KnobBinding& knob,
                           double             frame,
                           int                view) const
    {
      if (knob) {
        return knob.get<T>(frame, view);
      }
      return defaultValue;
    }

    template<typename T>
    inline T
    GeomOpEngine::getValue(const T&    defaultValue,
                           const char* knobName,
                           double      frame,
                           int         view) const { return getValue<T>(defaultValue, _firstOp->knob(knobName), frame, view); }

    template<typename T>
    inline T
    GeomOpEngine::getValue(const T&           defaultValue,
                           const KnobBinding& knob) const { return (knob) ? knob.get<T>() : defaultValue; }

    template<typename T>
    inline T
    GeomOpEngine::getValue(const T&    defaultValue,
                           const char* knobName) const { return getValue<T>(defaultValue, _firstOp->knob(knobName)); }

    template<typename T>
    inline T
    GeomOpEngine::getValue(const KnobBinding& knob) const { return getValue<T>(T(), knob); }

    template<typename T>
    inline T
    GeomOpEngine::getValue(const char* knobName) const { return getValue<T>(_firstOp->knob(knobName)); }

    //-----------------------------------------

    template<typename T>
    inline void
    GeomOpEngine::getValue(T&          v,
                           const char* knobName,
                           double      frame,
                           int         view) const { getValue<T>(v, _firstOp->knob(knobName), frame, view); }

    template<typename T>
    inline void
    GeomOpEngine::getValue(T&           v,
                           const char*  knobName) const { getValue<T>(v, _firstOp->knob(knobName)); }

    //--------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------

    template <typename T>
    inline T GeomOp::getKnobValue(const char*          knob_name,
                                  const OutputContext& context,
                                  const T&             default_value) const
    {
      Knob* k = knob(knob_name);
      if (k) {
        return k->get<T>(&context);
      }
      return default_value;
    }

    template <typename T>
    inline bool GeomOp::setKnobValue(const char* knob_name,
                                     const T&    value,
                                     double      time,
                                     int32_t     element_offset,
                                     int32_t     view)
    {
      Knob* k = knob(knob_name);
      if (k) {
        return k->set(value, time, element_offset, view);
      }
      return false;
    }

    template <typename T>
    inline bool GeomOp::setKnobValues(const char*                knob_name,
                                      bool                       clear_keys,
                                      const std::vector<T>&      values,
                                      const std::vector<double>& times,
                                      int32_t                    element_offset,
                                      int32_t                    view)
    {
      Knob* k = knob(knob_name);
      if (k && times.size() == values.size()) {
        return k->set(clear_keys, values, times, element_offset, view);
      }
      return false;
    }

    inline void GeomOp::KnobDefinesGeometry(const Knob_Closure& f)
      { AppendKnobToTarget(f, usg::EngineTokens.target.defineGeometry); }
    //
    inline void GeomOp::KnobModifiesAttribValues(const Knob_Closure& f)
    {
      if (f.storeKnobs()) {
        // Knob authoring mode value affects define-geometry:
        if (f.lastStoredKnob() && f.lastStoredKnob()->getAuthoring()) {
          Hash author_hash;
          author_hash.append(static_cast<int>(f.lastStoredKnob()->getAuthoring()->getMode()));
          appendToDefineGeometryTarget(author_hash, f.lastStoredKnob()->editState());
        }
      }
      // Knob hash affects modify-values:
      AppendKnobToTarget(f, usg::EngineTokens.target.modifyValues);
    }
    //
    inline void GeomOp::KnobEditStage(const Knob_Closure& f)
      { AppendKnobToTarget(f, usg::EngineTokens.target.editStage); }

    inline void GeomOp::appendToDefineGeometryTarget(const fdk::Hash& value_hash, const fdk::Hash& version, bool is_animating_source)
      { appendToTarget(usg::EngineTokens.target.defineGeometry, value_hash, version, is_animating_source); }
    inline void GeomOp::appendToDefineGeometryTarget(const Hash& value_hash, const Hash& version, bool is_animating_source)
      { appendToTarget(usg::EngineTokens.target.defineGeometry, value_hash, version, is_animating_source); }
    inline void GeomOp::appendToDefineGeometryTarget(Op* op) {appendToTarget(usg::EngineTokens.target.defineGeometry, op);}
    //
    inline void GeomOp::appendToModifyValuesTarget(const fdk::Hash& value_hash, const fdk::Hash& version, bool is_animating_source)
      { appendToTarget(usg::EngineTokens.target.modifyValues, value_hash, version, is_animating_source); }
    inline void GeomOp::appendToModifyValuesTarget(const Hash& value_hash, const Hash& version, bool is_animating_source)
      { appendToTarget(usg::EngineTokens.target.modifyValues, value_hash, version, is_animating_source); }
    inline void GeomOp::appendToModifyValuesTarget(Op* op) {appendToTarget(usg::EngineTokens.target.modifyValues, op); }
    //
    inline void GeomOp::appendToEditStageTarget(const fdk::Hash& value_hash, const fdk::Hash& version, bool is_animating_source)
      { appendToTarget(usg::EngineTokens.target.editStage, value_hash, version, is_animating_source); }
    inline void GeomOp::appendToEditStageTarget(const Hash& value_hash, const Hash& version, bool is_animating_source)
      { appendToTarget(usg::EngineTokens.target.editStage, value_hash, version, is_animating_source); }
    inline void GeomOp::appendToEditStageTarget(Op* op) { appendToTarget(usg::EngineTokens.target.editStage, op); }

    //--------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------

    //! Print out InputBinding components to a stream.
    DDImage_API  std::ostream& operator << (std::ostream&, const GeomOpEngine::InputBinding&);


}} // namespace DD::Image

#endif
