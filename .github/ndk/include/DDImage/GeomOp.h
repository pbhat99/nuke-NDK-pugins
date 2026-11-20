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
#include <atomic>
#include <unordered_map>
#include <type_traits> // for is_base_of
#include <array>
#include <memory>


namespace usg {
  class ShaderDescGroup;
  using ShaderDescGroupPtrMap = std::unordered_map<std::string, ShaderDescGroup*>;
}
namespace ndk {
  class GeomOpNode;
  class NodeEvalContext;
}

namespace DD {
  namespace Image {

    class GeomOp;
    class GeomOpEngine;
    using GeomOpNode = ndk::GeomOpNode;
    using GeomOpList = std::vector<GeomOp*>;
    using GeomOpEngineList = std::vector<GeomOpEngine*>;
    using GeomStateTargetIdxMap = std::unordered_map<usg::Token, uint32_t>;

    //--------------------------------------------------------------------------------

    //! GeomOpEngine builder function type passed into GeomOp constructor, used by
    //! GeomOpNode to construct engines.
    //! 'parent' is automatically provided when the first GeomOpEngine request is made.
    using GeomOpEngineBuilder = std::function<GeomOpEngine*(ndk::GeomOpNode* parent)>;

    //--------------------------------------------------------------------------------


    /*! Scenegraph processing engine node that can build a usg::Stage.
        One of these is created and shared between all GeomOps generated from the same
        Nuke Node.
    */
    class DDImage_API GeomOpEngine : public usg::GeomEngine
    {
    public:
      //! A class which gives a read handle onto a Knob, allowing GeomEngines to fetch knob values
      //! and query animation state.
      class KnobBinding
      {
      public:
        KnobBinding(const GeomOpEngine& engine, const char* knobName) : _engine(&engine), _knob(engine.nodeKnob(knobName)) {}
        KnobBinding() = default;
        KnobBinding(const KnobBinding& other) = default;

        //! Returns true if the reference to the knob is valid
        operator bool() const { return (_knob != nullptr); }

        //! Returns the bound knob, if there is one.
        Knob* knob() const { return _knob; }

        //! Returns true if the knob is animated
        bool isAnimated() const { return _knob->isAnimated(); }

        //! Returns true if the knob is not animated
        bool isUniform() const { return !_knob->isAnimated(); }

        //! Returns the authoring mode of the knob
        const Authoring::Mode getAuthoringMode() const { return _knob->getAuthoring()->getMode(); }

        //! Builds a context for this KnobBinding with the GraphScope from the engine
        //! and the time/view args.
        OutputContext getContext(const fdk::TimeValue& time = fdk::defaultTimeValue(), int view = -1) const;

        //! Return the value of the knob at a 'default' time of 0.0, since Nuke Knobs do not
        //! support a separate 'no-time' value. This is primarily for non-animating Knob values
        //! like strings. Uses the GeomOpEngine's GraphScope context.
        template<class T>
        T get(int view = -1) const;

        //! Returns the value of the knob at the given time and optional view, and the GraphScope
        //! of the GeomOpEngine owner.
        template<class T>
        T get(const fdk::TimeValue& time, int view = -1) const;

        //! Returns the text string from the knob, possibly evaluated, at the engine's
        //! GraphScope context.
        const char* getText() const;

      private:
        const GeomOpEngine* _engine = nullptr;
        Knob* _knob = nullptr;
      };

      //----------------------------------------------------------------------


    protected:
      GeomOpEngine(GeomOpNode* parent);


    public:
      virtual ~GeomOpEngine();

      //! Returns nodeName() from \a _geomNode. Since the name of the Nuke Node can change
      //! dynamically we don't save this string locally.
      std::string  name() const override;

      //! Return true if the owning node context has been aborted.
      bool aborted() const override;

      //! NodeEvalContext the GeomEngine was created for. This is the context used to track
      //! engine storage in the geomNode.
      const ndk::NodeEvalContext&  engineGraphContext() const;

      //----------------------------------------------------------------------

      //! Use this to access values that *ALL* GeomOp instances would share,
      //! ie non-animating, non-viewsplit values, or to access Nuke Node methods.
      Op*  firstOp() const;

      //! Return the raw Knob* from the parent Node.
      Knob*  nodeKnob(const char* knobName) const;

      //! GraphScope context the GeomEngine is configured for. This must be used when
      //! retrieving KnobBinding values to access the correct Knob state for this engine.
      //! Note - this can be different than the engineGraphContext()!
      ndk::GraphScopeId  graphScopeId() const { return _graphScopeId; }

      //! Version number of engine, incremented when state changes.
      uint64_t  version() const { return _version; }

      //----------------------------------------------------------------------

      //! Return a read handle to the named Knob from the engine.
      KnobBinding  knob(const char* knobName) const { return KnobBinding(*this, knobName); }

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

      //! Returns a generic Op location for input \a nodeInput.
      //! Warning, the returned location should only be used for Op typing, Node/Knob queries
      //! or Node input info. Similar to using Op::nodeInputConnection() with ANY_CONTEXT.
      //!
      //! Use inputConnectionAt() to get a input Op connection at a specific context.
      OpGraphLocation  inputConnection(uint32_t nodeInput);

      //! Returns an Op connection for input \a nodeInput at a specific \a frame and \a OutputContext.
      //! GraphScope in inputCtx is used for the evaluation context.
      //!
      //! Note that the frame number in \a inputCtx arg is always replaced by the \a frame arg.
      Op::Input  inputConnectionAt(uint32_t             nodeInput,
                                   double               frame,
                                   const OutputContext& inputCtx, 
                                   Execute*             execute = nullptr);
      //! Same as inputConnectionAt() but for a single view. If view is -1 (invalid) then the 'main'
      //! view (view index 1) is used instead.
      //! GraphScope in engine is used for the evaluation context.
      Op::Input  inputConnectionAt(uint32_t nodeInput,
                                   double   frame,
                                   int32_t  view = -1,
                                   Execute* execute = nullptr);

      //! Returns an Op for input \a nodeInput at a specific \a frame and \a OutputContext.
      //! GraphScope in inputCtx is used for the evaluation context.
      //!
      //! Note that the frame number in \a inputCtx arg is always replaced by the \a frame arg.
      Op*     inputOpAt(uint32_t             nodeInput,
                        double               frame,
                        const OutputContext& inputCtx, 
                        Execute*             execute = nullptr);
      //! Same as inputOpAt() but for a single view. If view is -1 (invalid) then the 'main'
      //! view (view index 1) is used instead.
      //! GraphScope in engine is used for the evaluation context.
      Op*     inputOpAt(uint32_t nodeInput,
                        double   frame,
                        int32_t  view = -1,
                        Execute* execute = nullptr);

      //! Returns the Op::firstOp() pointer from the nodeInput Input.
      //! Warning, the firstOp may not be valid for a specific OutputContext that
      //! the engine is processing. It is ok to use for generic Node or Knob queries
      //! and Node input info - see the Op::firstOp() method for more info.
      //!
      //! Use inputOpAt() and inputOpAtContext() to get a specific valid input Op.
      Op*     inputOp(uint32_t nodeInput) const;

      //! Return the number of Node input connections. Note that these are NOT Op inputs.
      size_t  nInputs() const override { return _inputs.size(); }

      //! Returns the GeomOpEngine pointer from \a nodeInput, if input is another GeomEngine.
      //! Returns null if the input is not a GeomOp Node or is an 'empty' GeomOpEngine
      //! (a NullGeomOp) if the input is disconnected.
      GeomEngine*  inputEngine(uint32_t nodeInput) const override;
      GeomEngine*  inputEngine0()                  const override;
      GeomEngine*  inputEngine1()                  const override;

      //! Return a list of input GeomOpEngines this engine is connected to.
      GeomOpEngineList  inputGeomOpEngines() const;

      //! Prints the contents of the current input.
      void  printInputs(std::ostream&, const char* prefix ="") const;

      //----------------------------------------------------------------------

      //! Fast check if the engine needs to rebuild its states. This is cheap to call
      //! for repeat visits. Returns false if no change.
      //!
      //! If the fast check determines the engine needs to update then the engine's
      //! \a _version number will be incremented. The changed version indicates the need
      //! to call the more expensive \a _updateState() which we want to defer until the
      //! engine is asked to provide state hashes or output data, and only pay the cost
      //! once.
      //! If \a forceUpdate is true then \a _version will always be incremented.
      bool validateState(bool forceUpdate = false);

      //! Default engine implemenation which sets the nuke:node data on the edit layer so we
      //! can go back from the Layer to the Node that created it.
      void  processScenegraph(usg::GeomSceneContext& context) override;

      /*! Update the state of the GeomOpEngine to the provided OutputContext's GraphScope.
          If configuring from a single GeomOp pass in that GeomOp's outputContext().
          This can be used for configuring a standalone GeomOpEngine.
      */
      void  updateStateToContext(const OutputContext& context);

      /*! The 'request' hash is used to track the evaluation or configuration state of
          the GeomEngine by the GeomOp network, it does not indicate the geometry state
          of the engine.

          The request hash is built in GeomOp::buildGeometryStage(), passed to _buildStage()
          and if unchanged then the state of the GeomEngine and its input network is assumed
          to be unchanged.
      */
      const fdk::Hash&  lastRequestHash() const { return _lastRequestHash; }


      //----------------------------------------------------------------------


      /*! Converts the input Op(s) to surface shader and material Prims, defined in the
          edit Layer anchored under \a materialLibraryPath.

          When \a createMode is false this method will update any existing overrides for the
          shader and material prims that were created during the \a create_mode=true phase.
          Overrides are only written if the shader/material parameters are not at their
          inital default.

          \a sourceOps is the list of input Ops to be imported and the OutputContext they are
          being referenced at. If \a sampleOps is empty nothing will be done and an empty
          path returned. Important note - the OutputContext's time is when time samples are
          being written into attributes vs. the input Op::outputContext() time which may be
          different due to time modifier nodes like FrameHold and TimeOffset.

          If the supplied Ops are ShaderOps (have a MaterialOpI interface) then the
          shaders are created through that interface. If the input is an Iop then it is
          treated as a texture source to a default constant (albedo only) shader. Legacy
          Material ops are NOT supported and will be ignored.

          The \a materialGroupIndex is used to track the 'output' ShaderDesc from each material
          graph in the \a _shaderDescGroups map, but it doesn't need to match an actual
          GeomOpEngine input number. Typically there will be only one input material graph so
          materialGroupIndex will usually be 0.

          Returns the path to the created or updated MaterialPrim, if successful.
      */
      virtual usg::Path importShaderOp(bool                                  createMode,
                                       usg::GeomSceneContext&                context,
                                       const Op::InputConnectionContextList& sourceOps,
                                       uint32_t                              materialGroupIndex,
                                       const usg::Path&                      materialLibraryPath);

      /*! Assign the material to a list of Prims. Companion method to importShaderOp() usually
          called after it to assign the created material to a list of previously-created Prims.

          When \a create_mode is true each Prim is retrieved from the engine's editLayer() and
          a Relationship created on the Prim.
          When \a create_mode is false each Prim is retrieved from the composed stage flowing
          through this node, and a Relationship matching \a material_path is searched for and
          updated.
      */
      virtual void      assignMaterialBinding(usg::GeomSceneContext& context,
                                              bool                   createMode,
                                              const usg::Path&       materialPath,
                                              const usg::PathArray&  primPaths);


      //----------------------------------------------------------------------


      //! Enumeration that defines the propogation and precedence for the injected prim paths.
      //! If the next engine has multiple inputs that engine must decide how to combine/use the input injections.
      enum class InjectionType : int32_t
      {
        DownstreamEngineDefault,  //! Inject into downstream engines and use these as the injected paths for the engine if no override.
        NextEngineOverride,       //! Inject into the next downstream engine only and use these as the injected paths for that engine.
        NumElements
      };

      using InjectedPaths = std::array<std::shared_ptr<usg::PathArray>, static_cast<int32_t>(InjectionType::NumElements)>;

      //! Returns the prim paths for a particular injection type that should be injected into the downstream engine.
      std::shared_ptr<usg::PathArray> getPathsToInjectDownstream(InjectionType type) const;

      //! Sets the prim paths for a particular injection type that should be injected into the downstream engine.
      void setPathsToInjectDownstream(InjectionType type, std::shared_ptr<usg::PathArray> paths);


      //----------------------------------------------
      // For internal use only
      //----------------------------------------------

      //! GeomOpNode owner.
      ndk::GeomOpNode*  geomNode() const { return _geomNode; }


    protected:
      friend class GeomOp;
      friend class GeomOpNodeImpl;
      GeomOpNode* _geomNode; //!< Owner node
      //----------------------------------------------------------
      // TODO ndk17: remove when ndk::NodeEvalContext becomes public:
      struct NodeEvalContext {
        fdk::Hash graphScopeId;
        fdk::Hash variantId;
        void* owner = nullptr;
        std::atomic<bool>* aborted = nullptr;

        virtual ~NodeEvalContext() = default;
      };
      //----------------------------------------------------------
      /*ndk::*/NodeEvalContext _engineGraphContext; //!< Engine graph creation context
      ndk::GraphScopeId        _graphScopeId;       //!< GraphScope context the GeomEngine is configured for
      //
      usg::ShaderDescGroupPtrMap _shaderDescGroups; //!< Groups of locally-created ShaderDescs

      InjectedPaths _pathsToInjectDownstreamByType; //!< Array of prim path arrays to be injected downstream where the array is indexed by the injection type.

      //----------------------------------------------------------------------

      struct Input
      {
        uint32_t      nodeInput = 0;              //!< The index of the Nuke Node input
        GeomOpEngine* upstreamEngine = nullptr;   //!< Upstream GeomOpEngine, if there is one.
        Op*           upstreamFirstOp = nullptr;  //!< Upstream Node's firstOp
      };
      using InputList = std::vector<Input>;

      void _printInput(std::ostream& o, const GeomOpEngine::Input& b) const;

      //---------------------------------------------------------------------------------

      void _updateState(const ndk::GraphScopeId& nodeGraphScopeId,
                        fdk::TimeValueSet&       sampleTimes,
                        std::set<int32_t>&       sampleViews);

      //! Sets the owner node data in the edit layer to point to the owning node
      void setOwnerNode();

      //! Updates the paths that should be injected downstream from this engine by checking
      //! the injected paths from the previous engine.
      //! The default implementation just ensures the injected paths from input 0 with a
      //! DownstreamEngineDefault type are added to the _pathsToInjectDownstreamByType array.
      //! Called by processScenegraph()
      virtual void _updatePathsToInjectFromInput();
      

    private:
      InputList  _inputs;             //!< Input connections
      fdk::Hash  _inputsHash;         //!< Changes when input topology changes, to update inputs
      fdk::Hash  _lastRequestHash;    //!< The last combined GeomOp hashes
      int32_t    _lastTreeVersion;    //!< Current global Nuke treeVersion, to indicate possible edit changes
      std::atomic<uint64_t> _version; //!< Current engine version indicating need to run _updateState()
      uint64_t   _lastVersion;        //!< Engine version after _updateState() has been run
      bool       _isVaryingModifier;  //!< Engine has animating modifications

      const Input& _getInput(uint32_t nodeInput) const;

      void _buildStage(usg::StageRef&           stage,
                       const fdk::Hash&         newRequestHash,
                       const usg::ArgSet&       requestArgs,
                       const ndk::GraphScopeId& nodeGraphScopeId,
                       fdk::TimeValueSet&       sampleTimes,
                       std::set<int32_t>&       sampleViews);

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
    public:
      /*! \class DD::Image::GeomOp::Description

          Specialized Op description class which requests the constructed GeomOp to be owned
          by a GeomOpNode rather than a generic OpNode, which allows the GeomOpNode to
          share GeomEngine(s) and the Stage stored inside it with all sibling GeomOps.
      */
      class DDImage_API Description : public Op::Description
      {
      public:
        //! Specialized Description which requests a GeomOpNode owner.
        //! 
        Description(const char* opClass, OpConstructor opCtor) :
          Op::Description(opClass, opCtor, "GeomNode"/*ndk::GeomOpNode::descriptionName()*/) {}
      };

      //--------------------------------------------------------------------------------


    protected:
      /*! Base class constructor requires a GeomOpEngine builder method, only
          subclasses can create an instance.
          The Engine builder method is usually declared in a GeomOp subclass like so:

            class MyGeomOp : public GeomOp
            {
              static const Description description;
              const char* Class() const override { return description.name; }
              const char* node_help() const override { return "Does something cool"; }

              class MyEngine : public Engine
              { 
                MyEngine(GeomOpNode* parent) : Engine(parent) {}

                void processScenegraph(usg::GeomSceneContext& context) override
                {
                  do something cool
                }
              };

              MyGeomOp(Node* node) : GeomOp(node, BuildEngine<MyEngine>()) {}
            };

            static Op* build(Node* node) { return new MyGeomOp(node); }
            const GeomOp::Description MyGeomOp::description("MyGeom", build);
      */
      GeomOp(Node*               node,
             GeomOpEngineBuilder engineBuilder);

      //! Ctor for MultiTypeNode support only! Do not implement for Engine-using subclasses.
      GeomOp(Node*);

      //! GeomOps are owned by the 'GeomNode' Node identifier.
      const char* nodeClass() const override { return "GeomNode"/*ndk::GeomOpNode::descriptionName()*/; }

      //! GeomOps are managed by the 'GeomOpNode' Node type.
      const char* nodeManagerClass() const override { return "GeomOpNode"/*ndk::GeomOpNode::opManagerName()*/; }

      //! Return hints to control when and how this op will be evaluated by the top-down system.
      OpHints opHints() const override;

      //--------------------------------------------------------------------------------

      //! A helper function to use as the \a engineBuilder argument to the GeomOp constructor.
      template<class Engine>
      static GeomOpEngineBuilder BuildEngine()
      {
        return [](GeomOpNode* parent) { return new Engine(parent); };
      }


    public:
      virtual ~GeomOp();

      //!
      const GeneralInfo& generalInfo() const override { return info_; }

      //! Cast to a GeomOp. This is much cheaper and safer than using dynamic_cast.
      const GeomOp*  geomOp() const override { return this; }
            GeomOp*  geomOp()       override { return this; }

      //! GeomEngine shared graph processor for Op's OutputContext GraphScope, owned by GeomOpNode.
      //! This is the geometry engine for the default node context.
      GeomOpEngine*  geomEngine() const;

#if kDDImageVersionMajorNum <= 17
      mFnDeprecatedInNuke17("engineNode() has been deprecated, use geomEngine() instead.")
      GeomOpEngine*  engineNode() const { return geomEngine(); }
#else
#error "Remove the above code block in Nuke 18 as they are deprecated in Nuke 17."
#endif


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

      //! Returns the compose state of the GeomOpEngine specified by engineGraphContext.
      fdk::Hash  geometryComposeState(const ndk::NodeEvalContext& engineGraphContext) override;

      //! Same but uses the default engine's engineGraphContext() for engine retrieval.
      fdk::Hash  geometryComposeState() override;

      //! Returns the edit state of the GeomOpEngine specified by engineGraphContext.
      fdk::Hash  geometryEditVersionState(const ndk::NodeEvalContext& engineGraphContext) override;

      //! Same but uses the default engine's engineGraphContext() for engine retrieval.
      fdk::Hash  geometryEditVersionState() override;

      //! Returns the animating state of the GeomOpEngine specified by engineGraphContext.
      bool  geometryStateVaries(const ndk::NodeEvalContext& engineGraphContext) override;

      //! Same but uses the default engine's engineGraphContext() for engine retrieval.
      bool  geometryStateVaries() override;

      //! Base class returns an empty layer.
      usg::LayerRef buildGeometryLayerForContext(const ndk::NodeEvalContext& engineGraphContext,
                                                 bool                        appendTo,
                                                 const fdk::TimeValueSet&    sampleTimes) override;

      //! Provide a fully constructed usg::Stage via getGeometryStage().
      bool canProvideGeometryStage() const override { return true; }

      //! Build the provided usg::Stage which is owned by the caller. This will forward
      //! to the GeomOp's firstOp() since we need to use a 'timeless' methodology, appending
      //! this GeomOp's OutputContext frame to \a sampleTimes.
      void buildGeometryStageForContext(usg::StageRef&              stage,
                                        const usg::ArgSet&          requestArgs,
                                        const ndk::NodeEvalContext& engineGraphContext,
                                        const fdk::TimeValueSet&    sampleTimes = fdk::TimeValueSet()) override;


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
      Op*          default_input(int nodeInput) const override;

      //! Allows only GeomOp inputs by default.
      bool         test_input(int node_input, Op* op) const override;

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
                                       const usg::Token&   target_id,
                                       bool                targetCanAnimate = true);

      /*! Like AppendKnobToTarget() but for knobs with no current Op OutputContext (ie no frame
          or view.) Intended for global non-animating Node knobs like 'name' and 'tile_color'.
      */
      virtual void  AppendKnobToTarget(const char*       knobName,
                                       const usg::Token& target_id,
                                       bool              targetCanAnimate = true);


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
      T     getKnobValue(const char*          knobName,
                         const OutputContext& context,
                         const T&             default_value) const;

      /*! Convenience to get a Knob pointer and set its value in one step.
          See Knob::set() for more info.
          Same as calling:
            Knob* k = knob("foo"); if (k) k->set(value, time, element_offset, view);
          Return true on success.
      */
      template <typename T>
      bool  setKnobValue(const char* knobName,
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
      bool  setKnobValues(const char*                knobName,
                          bool                       clear_keys,
                          const std::vector<T>&      values,
                          const std::vector<double>& times,
                          int32_t                    element_offset = 0,
                          int32_t                    view = -1);

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

      /*! Returns whether this node acts as a sublayer boundary for GeoExport.

          When true, GeoExport stops traversal at this node and omits all
          upstream layers from the export. The base implementation returns
          false; subclasses (e.g. GeoBreakLayer, GeoImport) override as
          appropriate.
      */
      virtual bool   isLayerBreak() const { return false; }

      /*! Whether the geometry the node is managing should be pickable in the Viewer.
          Base class returns true.
      */
      virtual bool   selectable()  const;

      virtual bool   selection3DChanged(ViewerContext* ctx)   { return false; }
      virtual bool   selection3DCleared()                     { return false; }
      virtual bool   selection3DMouseDown(ViewerContext* ctx) { return false; }
      virtual bool   selection3DMouseUp(ViewerContext* ctx)   { return false; }
      virtual bool   selection3DMouseDrag(ViewerContext* ctx) { return false; }

      //------------------------------------------------------

      //! Handles updates from node selection changes.
      void           selectionChanged() const;

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

      const usg::GeomState&  inputGeomState()  const { return _inputGeomState; }
      const usg::GeomState&  localGeomState()  const { return _localGeomState; }
      const usg::GeomState&  outputGeomState() const { return _outputGeomState; }

      /*! Clear the GeomOp's local GeomStateTarget hashes which get built in knobs()
          and appendGeomState(). This is called by GeomOp::knobStorePrep().

          Warning - do not call this method unless you know exactly what you're doing
          otherwise unexpected behavior of the geometry system may result.
      */
      void  resetGeomState();

      /*! Make changes to the GeomStateTargets registered to this GeomEngine which
          contribute to the dirtiness(out-of-dateness) state of the layers and stage
          this node affects or processes. This is called by GeomOp::knobStorePost().

          Warning - do not call this method unless you know exactly what you're doing
          otherwise unexpected behavior of the geometry system may result.

          Each GeomStateTarget affects a specific aspect of the 3D scene and there are
          several pre-defined ones such as 'define-geometry' and 'modify-values' which
          in turn affect Hash values in the GeomState stored in the GeomEngine.

          Knob::editVersionHash() values are typically appended to the GeomStateTargets
          the knob wants to influence. This should be done in the knobs() method.

          For arbitrary hash contributions to GeomStateTargets, such as input
          Op::editVersionHash() values, subclasses should implement the virtual
          appendGeomState() method and append to the local GeomStateTargets via
          appendToTarget().
      */
      void  buildGeomState(DD::Image::Hash& newOpHash, DD::Image::Hash& newOpEditVersionHash);


      /*! Append an 'edit version' state hash to GeomStateTarget \a target_id. If \a isAnimatingSource
          is true then the GeomStateTarget is marked as animating regardless of whether the data
          actually changed over time.

          An edit version is an overall change in the data that may affect the target, not whether
          the data for a particular frame or view changes, but whether the -global- contents of the
          data is changed. For a Knob this is usually due to a user editing the Knob's value, and
          for an Op/Node it's the edit versions of all its Knobs combined.

          Not thread safe - assuming this is called on main thread.
      */
      void  appendToTarget(const usg::Token& target_id, bool targetCanAnimate, const fdk::Hash& editVersion, bool isAnimatingSource = false);
      void  appendToTarget(const usg::Token& target_id, bool targetCanAnimate, const Hash&      editVersion, bool isAnimatingSource = false);

      //! Convenience variant that appends knob->editVersionHash() and uses knob->isAnimated()
      //! for 'isAnimatingSource'.
      void  appendToTarget(const usg::Token& target_id, bool targetCanAnimate, Knob* knob);

      //! Convenience variant that appends the OpState from opConnection, and optionally opConnection.topologyHash().
      //! If targetCanAnimate == true then OpState.opHashAnimates is used for the target's
      //! 'isAnimatingSource'.
      void  appendToTarget(const usg::Token& target_id, bool targetCanAnimate, const Op::Input& opConnection, bool includeTopologyHash = true);

      // Convenience variants that affect the 'define-geometry' GeomStateTarget:
      void  appendToDefineGeometryTarget(const fdk::Hash& editVersion);
      void  appendToDefineGeometryTarget(const Hash&      editVersion);
      void  appendToDefineGeometryTarget(const Op::Input& opConnection, bool includeTopologyHash = true);
      void  appendToDefineGeometryTarget(Knob* knob);

      // Convenience variants that affect the 'modify-values' GeomStateTarget:
      void  appendToModifyValuesTarget(const fdk::Hash& editVersion, bool isAnimatingSource = false);
      void  appendToModifyValuesTarget(const Hash&      editVersion, bool isAnimatingSource = false);
      void  appendToModifyValuesTarget(const Op::Input& opConnection, bool includeTopologyHash = true);
      void  appendToModifyValuesTarget(Knob* knob);

      //----------------------------------------------
      // For internal use only
      //----------------------------------------------

      //! GeomOpNode owner.
      ndk::GeomOpNode*  geomNode() const { return _geomNode; }


      //! Retrieve a specific GeomEngine for a specific NodeEvalContext. If the context matches
      //! the GeomOp geomEngine context the geomEngine is directly returned. Otherwise it
      //! requests the engine from the owning GeomOpNode which may create a custom
      //! context-specific engine.
      GeomOpEngine*  geomEngineForContext(const ndk::NodeEvalContext& engineGraphContext) const;


    protected:

      //! Called by Nuke immediately BEFORE Op::knobs() store. Calls resetGeomState().
      void  _knobStorePrep(const OutputContext& opStoreContext) override;

      //! Called by Nuke immediately AFTER Op::knobs() store. Calls buildGeomState().
      void  _knobStorePost(const OutputContext& opStoreContext, Hash& newOpHash, Hash& newOpEditVersionHash) override;

      //! Called by Nuke after knobStorePost() to assign the Op's OutputContext, allowing
      //! changes to be applied. GeomOp rounds the frame to 3 decimal points
      //! (ie .001, a thousandth of a frame) to avoid tiny double-precision differences
      //! from adversely affecting the state hashes.
      void  setOutputContext(const OutputContext& context) override;

      //!
      int   knob_changed(Knob* k) override;


      /*! Append arbitrary hash contributions to this GeoOp's GeomStateTargets which
          affect the local GeomState. These hash contribution sources are normally from
          non-Knob sources like inputs or some other Op value like frame or view.
          \a newOpHash is provided as a convenience to allow the GeomOp to affect the Op's
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
      virtual void  appendGeomState(DD::Image::Hash& newOpHash);


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

      //! Non-Op (timeless) version of GeomOp::buildGeometryStage() forwarded from that method.
      static void _buildGeometryStage(GeomOp*                  firstGeomOp,
                                      const fdk::Hash&         newRequestHash,
                                      usg::StageRef&           stage,
                                      const usg::ArgSet&       requestArgs,
                                      const fdk::TimeValueSet& sampleTimes,
                                      const std::set<int32_t>& sampleViews);

      //----------------------------------------------


    protected:
      GeneralInfo  info_;     //!< Global intended frame range (must remain 'info_' for MultiTypeNode support!)

    private:
      friend class NullGeomOp;
      friend class GeomOpEngine;
      friend class GeomOpNodeImpl;
      GeomOpEngineBuilder _engineBuilder;     //!< GeomOpEngine allocator function, passed into ctor
      ndk::GeomOpNode*    _geomNode;          //!< Node owner, set by GeomOpNode
      GeomOpEngine*       _defaultGeomEngine; //!< Default GeomEngine shared graph processor for the Op's default node context
      void _initialize(ndk::GeomOpNode*, GeomOpEngine*);  //!< Called by GeomOpNode owner to initialize a new GeomOp

      struct TargetHash
      {
        fdk::Hash editVersionHash;      //!< Edit version state of target
        bool      isAnimating = false;  //!< If any of the target hashes are from an animating source like a Knob
      };
      using TargetHashMap = std::unordered_map<usg::Token, TargetHash>;

      TargetHashMap             _localTargetHashes; //!< 
      usg::GeomStateTargetArray _localTargets;      //!< What the GeomOp/GeomEngine intends to affect
      GeomStateTargetIdxMap     _localTargetsMap;   //!<
      //
      usg::GeomState  _inputGeomState;    //!< Input GeomState built from the output states of input GeomOps.
      usg::GeomState  _localGeomState;    //!< Local GeomState built from Knob edit versions, input versions, etc.
      usg::GeomState  _outputGeomState;   //!< Output GeomState built from _inputGeomState + _localGeomState

    }; // GeomOp


    //--------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------


    /*! This is a stub GeomOp, intended to be the dead-end in a geometry graph.
    */
    class DDImage_API NullGeomOp : public GeomOp
    {
    public:
      class NullEngine : public GeomOpEngine
      {
      public:
        NullEngine(GeomOpNode* parent) : GeomOpEngine(parent) {}

      protected:
        std::string name() const override { return std::string("NullEngine"); }

        //! No work to do, do nothing
        void  processScenegraph(usg::GeomSceneContext& context) override {}
      };

    public:
      NullGeomOp();
      NullGeomOp(Node* node) : GeomOp(node, BuildEngine<NullEngine>()) { inputs(0); }

      int          minimum_inputs() const override { return 0; }
      int          maximum_inputs() const override { return 0; }

      const char*  Class()     const override { return "NullGeomOp"; }
      const char*  node_help() const override { return ""; }

    };


    //--------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------

    inline OutputContext GeomOpEngine::KnobBinding::getContext(const fdk::TimeValue& time, int view) const
    {
      return OutputContext(_engine->graphScopeId(), fdk::isDefaultTimeValue(time) ? 0.0 : time, view);
    }

    template<class T>
    inline T GeomOpEngine::KnobBinding::get(int view) const
    {
      const OutputContext evalCtx = getContext(0.0, view);
      return _knob->get<T>(&evalCtx);
    }

    template<class T>
    inline T GeomOpEngine::KnobBinding::get(const fdk::TimeValue& time, int view) const
    {
      const OutputContext evalCtx = getContext(time, view);
      return _knob->get<T>(&evalCtx);
    }

    inline const char* GeomOpEngine::KnobBinding::getText() const
    {
      const OutputContext evalCtx = getContext();
      return _knob->get_text(&evalCtx);
    }

    //-----------------------------------------

    template<typename T>
    inline T
    GeomOpEngine::getValue(const T&           defaultValue,
                           const KnobBinding& knob,
                           double             frame,
                           int                view) const { return (knob) ? knob.get<T>(frame, view) : defaultValue; }

    template<typename T>
    inline T
    GeomOpEngine::getValue(const T&    defaultValue,
                           const char* knobName,
                           double      frame,
                           int         view) const { return getValue<T>(defaultValue, KnobBinding(*this, knobName), frame, view); }

    template<typename T>
    inline T
    GeomOpEngine::getValue(const T&           defaultValue,
                           const KnobBinding& knob) const { return (knob) ? knob.get<T>() : defaultValue; }

    template<typename T>
    inline T
    GeomOpEngine::getValue(const T&    defaultValue,
                           const char* knobName) const { return getValue<T>(defaultValue, KnobBinding(*this, knobName)); }

    template<typename T>
    inline T
    GeomOpEngine::getValue(const KnobBinding& knob) const { return getValue<T>(T(), knob); }

    template<typename T>
    inline T
    GeomOpEngine::getValue(const char* knobName) const { return getValue<T>(KnobBinding(*this, knobName)); }

    //-----------------------------------------

    template<typename T>
    inline void
    GeomOpEngine::getValue(T&          v,
                           const char* knobName,
                           double      frame,
                           int         view) const { getValue<T>(v, KnobBinding(*this, knobName), frame, view); }

    template<typename T>
    inline void
    GeomOpEngine::getValue(T&           v,
                           const char*  knobName) const { getValue<T>(v, KnobBinding(*this, knobName)); }

    //--------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------

    template <typename T>
    inline T GeomOp::getKnobValue(const char*          knobName,
                                  const OutputContext& context,
                                  const T&             default_value) const
    {
      Knob* k = knob(knobName);
      return (k) ? k->get<T>(&context) : default_value;
    }

    template <typename T>
    inline bool GeomOp::setKnobValue(const char* knobName,
                                     const T&    value,
                                     double      time,
                                     int32_t     element_offset,
                                     int32_t     view)
    {
      Knob* k = knob(knobName);
      return (k) ? k->set(value, time, element_offset, view) : false;
    }

    template <typename T>
    inline bool GeomOp::setKnobValues(const char*                knobName,
                                      bool                       clear_keys,
                                      const std::vector<T>&      values,
                                      const std::vector<double>& times,
                                      int32_t                    element_offset,
                                      int32_t                    view)
    {
      Knob* k = knob(knobName);
      if (k && times.size() == values.size()) {
        return k->set(clear_keys, values, times, element_offset, view);
      }
      return false;
    }

    inline void GeomOp::KnobDefinesGeometry(const Knob_Closure& f)
      { AppendKnobToTarget(f, usg::EngineTokens.target.defineGeometry, false/*targetCanAnimate*/); }
    //
    inline void GeomOp::KnobModifiesAttribValues(const Knob_Closure& f)
    {
      if (f.storeKnobs()) {
        // Knob authoring mode value affects define-geometry:
        if (f.lastStoredKnob() && f.lastStoredKnob()->getAuthoring()) {
          Hash author_hash;
          author_hash.append(static_cast<int>(f.lastStoredKnob()->getAuthoring()->getMode()));
          author_hash.append(f.lastStoredKnob()->editVersionHash());
          appendToDefineGeometryTarget(author_hash);
        }
      }
      // Knob hash affects modify-values:
      AppendKnobToTarget(f, usg::EngineTokens.target.modifyValues, true/*targetCanAnimate*/);
    }
    //
    inline void GeomOp::KnobEditStage(const Knob_Closure& f)
      { AppendKnobToTarget(f, usg::EngineTokens.target.editStage, false); }

    inline void GeomOp::appendToDefineGeometryTarget(const fdk::Hash& editVersion)
      { appendToTarget(usg::EngineTokens.target.defineGeometry, false, editVersion); }
    inline void GeomOp::appendToDefineGeometryTarget(const Hash& editVersion)
      { appendToTarget(usg::EngineTokens.target.defineGeometry, false, editVersion); }
    //
    inline void GeomOp::appendToModifyValuesTarget(const fdk::Hash& editVersion, bool isAnimatingSource)
      { appendToTarget(usg::EngineTokens.target.modifyValues, true, editVersion, isAnimatingSource); }
    inline void GeomOp::appendToModifyValuesTarget(const Hash& editVersion, bool isAnimatingSource)
      { appendToTarget(usg::EngineTokens.target.modifyValues, true, editVersion, isAnimatingSource); }


}} // namespace DD::Image

#endif
