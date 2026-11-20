// Copyright (c) 2011 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_ParticleOp_h
#define DDImage_ParticleOp_h

  #include "Op.h"
  #include "Quaternion.h"
  #include "Hash.h"
  #include "ParticleRender.h"

  #include <memory>

namespace DD
{
  namespace Image
  {

    // sprite particles alignment mode
    enum {
      eAlignNone      = 0,
      eAlignSpin      = 1,
      eAlignVelocity  = 2,
    };

    // blending mode with input shader
    enum {
      eBlendNone       = 0,
      eBlendModulate   = 1,
    };

    class ImageCacheReadI;
    class ImageCacheWriteI;

    typedef unsigned int ParticleChannelSet;

    enum FrameAdvance : unsigned char {
      eFrameAdvanceConstant, /// do not advance frame
      eFrameAdvanceStep,     /// advance a frame each frame
      eFrameAdvanceRandom,   /// pick a random frame each time
    };

    class ParticleNodeState;

    class ParticleNodeStateFactory;

    /// A particle system consisting of a number of particles, each with a set of attributes such as position, velocity, color etc.
    /// The attributes are guaranteed to be stored as consecutive arrays for efficiency. When iterating through the particles,
    /// it is far faster to use the methods which return a pointer to the start of the attribute array, than to make a call for every element.
    ///  e.g. do this:
    /// const Vector3* position = particleSystem->particlePosition();
    /// for ( int i : particleSystem )
    ///   doSomethingWith( position[i] );
    ///  and not this:
    /// for ( int i : particleSystem )
    ///   doSomethingWith( particleSystem->particlePosition(i) );
    class DDImage_API ParticleSystem
    {
    public:
      
      /// the Source of a particle representation (a Node, and its input)
      struct Source
      {
        Node* first;
        int second;
      };

      /// Information about particle collisions which is filled in by the ParticleBounce node
      enum CollisionFlags: unsigned char
      {
        eCollided     = 0x0001,   //! particle has collided this frame
        eKillParticle = 0x0002    //! bounce should kill the particle
      };

      struct BounceInfo
      {
        Vector3         impulse;            //! impulse that should be applied to the velocity
        Vector3         collisionPosition;  //! position the particle collided at
        float           collisionTime;      //! collision time in the frame. 0 (start of frame) to 1 (end of frame)
        unsigned int    flags;              //! logical OR of CollisionFlags
      };

      ParticleSystem( int maxParticles = 1000000 );

      // ParticleSystems shouldn't be cloned
      ParticleSystem(const ParticleSystem& particleSystem) = delete;

      virtual ~ParticleSystem();

      void operator=(const ParticleSystem& other);

      //! The number of particles in the system.
      unsigned int numParticles() const;

      //! The maximum number of particles which can be created in the system.
      unsigned int maxParticles() const;

      //! Create a new particle. Be aware that this may cause internal arrays to be reallocated
      //! and so invalidate any attribute pointers you are holding.
      int allocNewParticle(Vector3 position, float startTime, unsigned pathMask);

      //! Kill a particles. This will never invalidate the attribute pointers.
      void expireParticle(unsigned int i);

      //! Reset the whole system back to an empty state.
      void reset();

      //! Set the system time. This is the time at the end of the current time step.
      void setSystemTime( float time );

      //! Return the system time. This is the time at the end of the current time step.
      float systemTime() const;

      /**
       * Write the entire state of the particle system to the cache.
       */
      void write(const DD::Image::Hash& h, Op* node);

      /**
       * Write the entire state of the particle system to disk.
       */
      void write(const std::string& filename, const DD::Image::Hash& hash, Op* node);

      void doWrite(ImageCacheWriteI* f, const DD::Image::Hash& hash, Op* node);

      enum ReadResult {
        eReadNotFound,
        eReadCorrupt,
        eReadOK,
        eReadOutOfDate
      };

      /**
       * Read the particle system from the cache.
       *
       * If no cache is found, it returns eReadNodeFound, in which case the
       * particle system has been left in the state it was originally.  If
       * all is ok, it returns eReadOK.  If the file was corrupt in some way
       * which means that the particle system is now in a potentially invalid state,
       * then it will return eReadCorrupt.
       */
      ReadResult read(const DD::Image::Hash& h, Op* node);

      /**
       * Read the entire state of the particle system to disk.
       */
      ReadResult read(const std::string& filename, const DD::Image::Hash& hash, Op* node);

      ReadResult doRead(ImageCacheReadI* f, const DD::Image::Hash& hash, Op* node);

      /**
       * Find a NodeState for the given node.  If one cannot be find,
       * it uses the factory to make one
       */
      ParticleNodeState* nodeState(Node* node, ParticleNodeStateFactory* factory);

      static void MakeNodeOpMap(Op* op, std::map<Node*, Op*>& out);

      //! The position at which each particle was initially created
      Vector3* particleInitialPosition();
      const Vector3* const particleInitialPosition() const;
      Vector3& particleInitialPosition(unsigned idx);
      const Vector3& particleInitialPosition(unsigned idx) const;

      //! The current position of the particles
      Vector3* particlePosition();
      const Vector3* const particlePosition() const;
      Vector3& particlePosition(unsigned idx);
      const Vector3& particlePosition(unsigned idx) const;

      //! The previous position of the particles
      Vector3* particleLastPosition();
      const Vector3* const particleLastPosition() const;
      Vector3& particleLastPosition(unsigned idx);
      const Vector3& particleLastPosition(unsigned idx) const;

      //! The current velocity of the particles (in units/frame)
      Vector3* particleVelocity();
      const Vector3* const particleVelocity() const;
      Vector3& particleVelocity(unsigned idx);
      const Vector3& particleVelocity(unsigned idx) const;

      //! The size of the particle (the same units as the position)
      Vector3* particleSize();
      const Vector3* const particleSize() const;
      Vector3& particleSize(unsigned idx);
      const Vector3& particleSize(unsigned idx) const;

      //! The color of the particles, RGBA
      Vector4* particleColor();
      const Vector4* const particleColor() const;
      Vector4& particleColor(unsigned idx);
      const Vector4& particleColor(unsigned idx) const;

      //! The orientation of each particle.
      Quaternion4f* particleOrientation();
      const Quaternion4f* const particleOrientation() const;
      Quaternion4f& particleOrientation(unsigned idx);
      const Quaternion4f& particleOrientation(unsigned idx) const;

      //! The rotation axis of each particle.
      Vector3* particleRotationAxis();
      const Vector3* const particleRotationAxis() const;
      Vector3& particleRotationAxis(unsigned idx);
      const Vector3& particleRotationAxis(unsigned idx) const;

      //! The rotation angle of each particle around its axis.
      float* particleRotationAngle();
      const float* const particleRotationAngle() const;
      float& particleRotationAngle(unsigned idx);
      const float& particleRotationAngle(unsigned idx) const;

      //! The rotation velocity of each particle around its axis.
      float* particleRotationVelocity();
      const float* const particleRotationVelocity() const;
      float& particleRotationVelocity(unsigned idx);
      const float& particleRotationVelocity(unsigned idx) const;

      //! The mass of each particle.  Defaults to 1.
      float* particleMass();
      const float* const particleMass() const;
      float& particleMass(unsigned idx);
      const float& particleMass(unsigned idx) const;

      //! The total number of frames that each particle is expected to live for.
      float* particleLife();
      const float* const particleLife() const;
      float& particleLife(unsigned idx);
      const float& particleLife(unsigned idx) const;

      //! The chance that each particle will die each frame (applied separately from _life)
      float* particleExpirationChance();
      const float* const particleExpirationChance() const;
      float& particleExpirationChance(unsigned idx);
      const float& particleExpirationChance(unsigned idx) const;

      //! The time at which each particle came alive
      float* particleStartTime();
      const float* const particleStartTime() const;
      float& particleStartTime(unsigned idx);
      const float& particleStartTime(unsigned idx) const;

      //! A unique particle-ID.  Consistent within the lifetime of the particle, and not re-used.
      int* particleId();
      const int* const particleId() const;
      int& particleId(unsigned idx);
      const int& particleId(unsigned idx) const;

      //! The "channels" on which this particle exist (ie the sets it is part of)
      ParticleChannelSet* particleChannels();
      const ParticleChannelSet* const particleChannels() const;
      ParticleChannelSet& particleChannels(unsigned idx);
      const ParticleChannelSet& particleChannels(unsigned idx) const;

      //! Internal, used to implement ParticleMerge
      unsigned int* particlePathMask();
      const unsigned int* const particlePathMask() const;
      unsigned int& particlePathMask(unsigned idx);
      const unsigned int& particlePathMask(unsigned idx) const;

      //! Whether or not this particle exists.
      bool* particleActive();
      const bool* const particleActive() const;
      bool& particleActive(unsigned idx);
      const bool& particleActive(unsigned idx) const;

      //! The source of each particle (ie a ParticleEmitter or ParticleSpawn)
      //! and the input number from that that is used for the representation.
      Source* particleSource();
      const Source* const particleSource() const;
      Source& particleSource(unsigned idx);
      Source const& particleSource(unsigned idx) const;

      //! The frame to use from the representation
      float* particleRepresentationFrame();
      const float* const particleRepresentationFrame() const;
      float& particleRepresentationFrame(unsigned idx);
      const float& particleRepresentationFrame(unsigned idx) const;

      //! The advancement mode for the representation frame
      FrameAdvance* particleFrameAdvance();
      const FrameAdvance* const particleFrameAdvance() const;
      FrameAdvance& particleFrameAdvance(unsigned idx);
      const FrameAdvance& particleFrameAdvance(unsigned idx) const;

      //! The first frame for which the representation is available
      int* particleRepresentationFirstFrame();
      const int* const particleRepresentationFirstFrame() const;
      int& particleRepresentationFirstFrame(unsigned idx);
      const int& particleRepresentationFirstFrame(unsigned idx) const;

      //! The last frame for which the representation is available
      int* particleRepresentationLastFrame();
      const int* const particleRepresentationLastFrame() const;
      int& particleRepresentationLastFrame(unsigned idx);
      const int& particleRepresentationLastFrame(unsigned idx) const;

      //! information about the first bounce for each particle
      BounceInfo* particleBounceInfo();
      const BounceInfo* const particleBounceInfo() const;
      BounceInfo& particleBounceInfo(unsigned idx);
      const BounceInfo& particleBounceInfo(unsigned idx) const;

      //! The time remaining on this frame
      float* particleDeltaT();
      const float* const particleDeltaT() const;
      float& particleDeltaT(unsigned idx);
      const float& particleDeltaT(unsigned idx) const;

      //! The momentum of a particle
      Vector3 particleMomentum(unsigned idx) const;

      //! The representation of each particle (ie a GeoOp or an Iop)
      Op*& particleRepresentation(unsigned idx);
      Op* const& particleRepresentation(unsigned idx) const;

    private:
      std::unique_ptr<class ParticleStorage> _particleStorage;   //!< Private implementation
    };


    /**
     * ParticleNodeState is used for nodes to store state that
     * should be preserved with the particle system, and serialised/
     * unserialised from disk with the cache.
     *
     * For example, the Emitter uses this to keep track of which vertex
     * it has emitted from in the 'in order' option.
     */
    class DDImage_API ParticleNodeState
    {
    public:
      /**
       * Should create a deep copy of itself.
       */
      virtual ParticleNodeState* clone() = 0;

      /**
       * Should write out all data to the passed-in file
       */
      virtual void write(ImageCacheWriteI*) = 0;

      /**
       * Should clear its state and read in data from the file.  File
       * pointer will point at the position that write() had been called
       * from.  Should returns false in case of failure (including under-run)
       */
      virtual bool read(ImageCacheReadI*) = 0;

      /**
       * Destructor
       */
      virtual ~ParticleNodeState();
    };

    /**
     * Factory class used by ParticleSystem::nodeState
     */
    class DDImage_API ParticleNodeStateFactory
    {
    public:
      virtual ParticleNodeState* makeState() = 0;
      
      virtual ~ParticleNodeStateFactory();
    };

    //! A random number generator for the use of ParticleOps
    class DDImage_API Random
    {
    public:
      Random( int seed );

      void setSeed( int seed );

      unsigned int random();

      //! Return a random number in the range 0..n-1
      unsigned int random( unsigned int n );

      //! Return a random number in the range 0..1
      float randomU();

      //! Return a random number in the range -1..1
      float randomS();

      //! Return a random angle in the range 0..2pi
      float randomAngle360();

      //! Return a vaguely Gaussian distributed random number
      float randomGaussian();

      //! Return a number with a vaguely Gaussian spread around "value"
      float randomSpread( float value, float spread );

      //! Return a random vector
      Vector3 randomVector();

      //! Return a random unit vector
      Vector3 randomUnitVector();

      //! Return a random vector in the XY plane
      Vector3 randomUnitVectorXY();

    private:
      unsigned int _x;
      unsigned int _y;
      unsigned int _z;
    };


    class DDImage_API ParticleContext
    {
    public:
      ParticleContext();

      double dt() const { return _dt; }
      double startTime() const { return _startTime; }
      double endTime() const { return _endTime; }

      void setTimes(double startTime, double endTime, double dt)
      {
        _startTime = startTime;
        _endTime = endTime;
        _dt = dt;
      }

      bool isFirstSubframe() const { return _firstSubframe; }
      void firstSubframe() { _firstSubframe = true; }
      void nextSubframe() { _firstSubframe = false; }

    private:
      double _startTime;
      double _endTime;

      double _dt;

      bool _firstSubframe;
    };

    //! Particle operator
    //! This is the base class for all Ops which work on particle systems. To write your own ParticleOp,
    //! override the applyBehaviour method to modify the particle system in place.
    class DDImage_API ParticleOp : public ParticleRender
    {
    public:

      //! Types of particle ops. These are used when reordering the particle graph.
      enum OpType {
        eMove,
        eBounce,
        eSpawn,
        eOther
      };


      ParticleOp( Node * node ) : ParticleRender(node) {}
      ~ParticleOp() override;

      virtual bool applyBehaviour( const ParticleContext& context, ParticleSystem* ps ) = 0;

      unsigned node_color() const override { return 0xcccc8800; }
      const char* node_shape() const override { return "cr"; }

      /** get the curve-hash that affects the particle system's simulation */
      void appendSimulationCurve(DD::Image::Hash& hash) const;
    protected:
      
      /** get the curve-hash for the particle nodeInput.  Subclasses can override this to not do anything or
       * to only hash in particular data if the general curve-hash would not affect the simulation state. */
      virtual void appendSimulationInput(int nodeInput, DD::Image::Hash& hash) const;
      
    public:
      virtual bool doApplyBehaviour( const ParticleContext& context, ParticleSystem& ps, unsigned pathMask ) = 0;

      int minimum_inputs() const override { return 0; } // Base class has only one input
      int maximum_inputs() const override { return 0; }
      Op* default_input(int input) const override { return nullptr; }
      bool test_input(int input, Op* op) const override { return false; }

      void add_input_handle(int i, ViewerContext* ctx) override;

      virtual OpType getOpType()
      {
        return eOther;
      }
      
      //! Cast to a ParticleOp. This is much cheaper and safer than using dynamic_cast.
      ParticleOp* particleOp() override;
      const ParticleOp* particleOp() const override;

      static void ParticleChannels_knob(Knob_Callback f, ParticleChannelSet* ptr, const char* name, const char* label = nullptr);
    };


    class Domain;

    //! The base class for most of the particle forces and behaviours. This extends ParticleOp by adding the standard knobs
    //! for all the conditions that must apply for a particle before the op acts on it.
    class DDImage_API ParticleBehaviour : public ParticleOp
    {
    public:
      enum FalloffType {
        eNoFalloff,
        eInverseFalloff,
        eInverseSquareFalloff,
      };

      ParticleBehaviour(Node * node);
      ~ParticleBehaviour() override;

      int minimum_inputs() const override { return 1; }
      int maximum_inputs() const override { return 1; }
      bool test_input(int input, Op* op) const override;
      Op* default_input(int input) const override;
      const char* input_label(int input, char* buffer) const override;

      int knob_changed( DD::Image::Knob* knob ) override;

      ParticleOp* input_pop() const { return (ParticleOp*)(Op::input0()); }

      bool doApplyBehaviour( const ParticleContext& context, ParticleSystem& ps, unsigned pathMask) override;

      void addConditionsKnobs( Knob_Callback f );
      void addDomainKnobs( Knob_Callback f );
      void addSeedKnob( Knob_Callback f );

      //! add the an Enumeration_Knob for falloff, with given name/label
      static void Falloff_knob( Knob_Callback f, FalloffType* falloffType, const char* name, const char* label = nullptr);

      //! calculate the fallout given distance /r/
      static float calculateFalloff(float r, FalloffType type)
      {
        mFnAssert(r > 0);
        switch (type) {
        case eNoFalloff:
        default:
          return 1.0f;
        case eInverseFalloff:
          return 1.0f / r;
        case eInverseSquareFalloff:
          return 1.0f / (r * r);
        }
      }

      //! apply force to particle, for timeslice dt
      void applyForce(ParticleSystem* ps, unsigned idx, const ParticleContext& ctx, Vector3 force)
      {
        applyAcceleration(ps, idx, ctx, force / ps->particleMass(idx));
      }

      //! apply acceleration to particle, for timeslice dt
      void applyAcceleration(ParticleSystem* ps, unsigned idx, const ParticleContext& ctx, Vector3 accel)
      {
        const double kEnd = ctx.endTime();

        if (ps->particleStartTime(idx) > kEnd)
          return;

        double dt = kEnd - ps->particleStartTime(idx);
        if (dt > ctx.dt())
          dt = ctx.dt();

        ps->particleVelocity(idx) += accel * static_cast<float>(dt);
      }

      //! apply force to particle, for timeslice dt
      //! This (preferred) version is for use when you have the pointers to the particle attributes, which is much faster.
      void applyForce(const ParticleContext& ctx, const Vector3& force, Vector3& particleVelocity, double particleStartTime, float particleMass)
      {
        applyAcceleration(ctx, force / particleMass, particleVelocity, particleStartTime);
      }

      //! apply acceleration to particle, for timeslice dt
      //! This (preferred) version is for use when you have the pointers to the particle attributes, which is much faster.
      void applyAcceleration(const ParticleContext& ctx, const Vector3& acceleration, Vector3& particleVelocity, double particleStartTime)
      {
        const double kEnd = ctx.endTime();

        if (particleStartTime > kEnd)
          return;

        double dt = kEnd - particleStartTime;
        if (dt > ctx.dt())
          dt = ctx.dt();

        particleVelocity += acceleration * static_cast<float>(dt);
      }

      Domain* domain() const;

      //! return true if the conditions apply _and_ the particle is within any domain
      virtual bool conditionsApply( ParticleSystem* system, unsigned idx );

      HandlesMode doAnyHandles(ViewerContext* ctx) override;
      void build_handles( ViewerContext* context ) override;
      void draw_handle( ViewerContext* context ) override;

    protected:
      float _probability;
      ParticleChannelSet _channels;
      unsigned int _pathMask;
      Random _random;
      int _seed;
      float _minAge;
      float _maxAge;
      int _domainType;
      bool _domainInvert;
      Matrix4 _axis;
      Matrix4 _axisInverse;
    };

  }
}


#endif
