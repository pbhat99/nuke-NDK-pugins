// Copyright (c) 2011 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_ParticleRender_h
#define DDImage_ParticleRender_h

#include "DDImage/GeoOp.h"

namespace DD {
  namespace Image {

    class ParticleOp;
    class ParticleSystem;
    class ParticleHolder;

    /**
     * Base class for Particles Ops.  Takes a ParticleOp (either as input
     * or one deriving from this) and turns this into geo for the viewer
     * and renderers
     **/
    class DDImage_API ParticleRender : public GeoOp
    {
      friend class ParticleHolder;

    public:
      int minimum_inputs() const override { return 1; }
      int maximum_inputs() const override { return 1; }

      bool test_input( int input, Op* op ) const override;
      const char* input_label( int input, char* buffer ) const override;
      Op* default_input(int n) const override;

      const char* node_shape() const override { return "[)"; }

      void append( Hash& hash ) override;
      void get_geometry_hash() override;
      void geometry_engine(Scene& scene, GeometryList& out) override;

      //! preValidate step to simulate particles if necessary.  returns false
      //! if particle render was cancelled.
      PrevalidateResult doPreValidate(ViewerContext* ctx, PreValidateContext*) override;

      /*** generate and run a particle system for this op, returns null if the particle system could not be built
       @param prevTime - returns the time for the step before the time particle simulation was generated for.  Relevant for particle properties such as particleLastPosition.
       @param outTime - return the time that the particle simulation was generated for.  This time will be rounded up to the nearest step per frame that matches the output context.
       @param canCancel - if true the user can cancel the operation
       @param result - optional PrevalidateResult result can be passed in if calling from prevalidate
      */
      ParticleSystem* getParticleSystem(
                                     float& prevTime,
                                     float& outTime,
                                     bool canCancel = false,
                                     Op::PrevalidateResult* result = nullptr
                                     );
      
    protected:
      ParticleRender( Node* node );
      ~ParticleRender() override;
      
      ParticleHolder* particleHolder()
      {
        return _particleHolder;
      }

      //! If this is a ParticleOp, returns this, otherwise returns input(0) cast to a ParticleOp. Returns nullptr if neither are ParticleOps.
      ParticleOp* particleRenderOp();

      bool isLicensed() const;

      ParticleHolder* _particleHolder;

    private:
      typedef std::map<Iop*, std::vector<int> > ParticleTextureMap;

      std::set<ParticleRender*> _particleUsers;      
      ParticleOp* _mover;

      std::set<float> getNeededFrames() const;

      ParticleOp* mover();

      ParticleRender* firstOp()
      {
        return (ParticleRender*)(GeoOp::firstOp());
      }
      
      const ParticleRender* firstOp() const
      {
        return (const ParticleRender*)(GeoOp::firstOp());
      }

      bool doConditionsApply( ParticleSystem* ps, int idx );

      void filterParticle( std::vector<int>& particles, ParticleSystem* ps );
      void textureParticle(const std::vector<int>& particles, ParticleSystem* ps, ParticleTextureMap& particle_map);
      void updateParticleRepresentation(const std::vector<int>& particles, ParticleSystem* ps);
      void setParticleShaderOp(const std::vector<int>& particles, ParticleSystem* ps, GeometryList& out);
      void create_geometry(const std::vector<int>& particles, ParticleSystem* ps, Scene& scene, GeometryList& out, float posLerp);

    };

  }
}

#endif
