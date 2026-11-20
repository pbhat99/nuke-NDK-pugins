// ParticlesSprite.h
// Copyright (c) 2011 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_ParticlesSprite_H
#define DDImage_ParticlesSprite_H

#include "Primitive.h"
#include "Point.h"

#include <memory>

namespace DD
{
  namespace Image
  {
  
    struct SpriteGeneratorImpl;
    struct ParticleSearchInfoImpl;
    struct MBSceneSpriteGeneratorImpl;
    class MBParticleSpriteGeneratorImpl;  
    class SpriteGenerator;
    class MBSceneSpriteGenerator;
    class ParticleSearchInfo;

    /*! Structure to contain all the necessary info for a particular sprite */
    struct DDImage_API Sprite 
    {
      Vector3 pos[4];
      Vector4 uv[4];
      Vector4 color;
      Vector3 normal;
      const GeoInfo* info;
    };
    
    /*! ParticlesSprite primitive.
     */
    class DDImage_API ParticlesSprite : public Primitive
    {
    protected:
      Point::RenderMode render_mode_;

      /*! Destructive copy. */
      void copy(const ParticlesSprite*);

    public:
      const char* Class() const override;
      ParticlesSprite(Point::RenderMode render_mode, unsigned points = 0, unsigned start = 0);
      ParticlesSprite(const ParticlesSprite &);

      Point::RenderMode render_mode() const { return render_mode_; }
      void render_mode(Point::RenderMode v) { render_mode_ = v; }

      ParticlesSprite& operator = (const ParticlesSprite& b) { copy(&b);
        return *this; }

      /*! Copy this Particles into a new one, and return a pointer to it.
        Vertex data is duplicated, point indices are unchanged.
      */
      Primitive* duplicate() const override;
      unsigned faces() const override { return 0; }

      /*! Get the tessellation for an individual sprite, by index. This 
        will return true if the sprite is visible, false if not. */
      bool tessellateSprite(unsigned int spriteIndex,
                            Scene* scene, 
                            PrimitiveContext* ptx,  /*! Primitive context */ 
                            PrimitiveContext* nptx,  /*! Primitive context with local transforms removed,
                                                        to use for adding sprites into the scene */
                            SpriteGenerator* spriteGenerator,
                            MBSceneSpriteGenerator* mbSceneGenerator,
                            ParticleSearchInfo& searchInfo) const;

      /*! Push a single rParticles primitive into the rendering stream.
       */
      void tessellate(Scene*, PrimitiveContext*) const override;

      void draw_wireframe(ViewerContext*, PrimitiveContext*,
                                      Primitive* prev_prim = nullptr) const override;
      void draw_solid(ViewerContext*, PrimitiveContext*,
                                  Primitive* prev_prim = nullptr) const override;
      void draw_primitive_num(ViewerContext*, PrimitiveContext*) const override;

      //! type identifier to avoid dynamic casts for any derived classes
      PrimitiveType getPrimitiveType() const override  { return eParticlesSprite; };

      Box3 get_bbox(const GeoInfo* info) const override;

      //! draw a single particle quad
      static void draw_quad ( const Vector3& p,
                              const Vector4* cf,
                              const Vector3* vel,
                              const float* size,
                              const float* spin,
                              const float ar,
                              const Matrix4& modelview,
                              const Matrix4& viewModel );

      //! draw a single particle lineloop
      static void draw_loop ( const Vector3& p,
                              const Vector4* cf,
                              const Vector3* vel,
                              const float* size,
                              const float* spin,
                              const float ar,
                              const Matrix4& modelview,
                              const Matrix4& viewModel );
    };
    
  
    /*! Helper structure to generate a particular sprite */
    class DDImage_API SpriteGenerator
    {
      std::unique_ptr<SpriteGeneratorImpl> _pImpl;

      /*! not implemented */
      SpriteGenerator(const SpriteGenerator& other);
      
      /*! not implemented */
      void operator=(const SpriteGenerator& other);   

    public:
      SpriteGenerator(const GeoInfo* info);
      
      ~SpriteGenerator();

      /*! get the number of available particles */
      size_t getParticleCount() const;

      /*! fill the sprite structure passed in for the particle at
        the given index, applying the given transform. This function
        returns true if the particle is visible after applying the 
        transform, false if not. If the particle is not visible, the 
        sprite structure will not contain a valid sprite and should 
        not be used */
      bool getSprite(unsigned index, MatrixArray *transform, Sprite &sprite);
      
      /*! get a unique hash for the particle; this can 
        be used to identify the same particle at 
        another point in time */
      Hash getHash(int i) const;
    };

    /* Helper structure used to optimise multiple
       particle searches by storing information from 
       past searches */
    class DDImage_API ParticleSearchInfo
    {
      std::unique_ptr<ParticleSearchInfoImpl> _pImpl;

      friend struct MBSceneSpriteGeneratorImpl;

      /*! not implemented */
      ParticleSearchInfo(const ParticleSearchInfo &other);
      
      /*! not implemented */
      void operator=(const ParticleSearchInfo& other); 

    public:
      ParticleSearchInfo();

      ~ParticleSearchInfo();

      /*! Get the id of the scene this search info
        was last used for */
      int sceneId() const;
    };
    
    /*! Helper structure to identify a matching particle
      in another scene (for generating motion blur). */
    class DDImage_API MBSceneSpriteGenerator
    {
      std::unique_ptr<MBSceneSpriteGeneratorImpl> _pImpl;

      /*! not implemented */
      MBSceneSpriteGenerator(const MBSceneSpriteGenerator& other);
      
      /*! not implemented */
      void operator=(const MBSceneSpriteGenerator& other);   

    public:
      // Construct an MBSceneSpriteGenerator
      MBSceneSpriteGenerator();

      ~MBSceneSpriteGenerator();

      // set the motion blur scene (not thread-safe)
      void setMBScene(Scene *scene);

      /* get the position of the particle with the given hash,
         if it can be found in the motion blur scene. This
         function will return false if the particle was not
         found 
         searchInfo will be used to store information which 
         is used internally to optimise subsequent particle 
         searches. Each thread that calls getSpritePosition 
         will require its own instance of ParticleSearchInfo,
         which should persist between searches for optimum 
         speed. */
      bool getSpritePosition(Hash hash, Vector3* pt, ParticleSearchInfo &searchInfo);

      /*! reset the search info to search in a different scene.
        The user is responsible for calling this when the scene changes.*/
      void resetSearchInfo(ParticleSearchInfo &searchInfo, int sceneId);
      
    };

  
    /**
     * MBSceneParticlesSprite.
     *
     * 
     *
     */
    class DDImage_API MBParticleSpriteGenerator
    {    
      std::unique_ptr<MBParticleSpriteGeneratorImpl> _pImpl;
      
      /*! not implemented */
      MBParticleSpriteGenerator(const MBParticleSpriteGenerator& other);

      /*! not implemented */
      void operator=(const MBParticleSpriteGenerator& other);      
    
    public:
      
      typedef std::map<float, Sprite> TimeSpriteMap;
      
      /*! constructor */
      MBParticleSpriteGenerator();
        
      /*! destructor */
      ~MBParticleSpriteGenerator();
        
      /*! initialize the sprite generator with all scenes */
      void init( Scene** scenes, unsigned count );
      
      /*! get the number of available particles */
      size_t getParticleCount() const;
        
      /*! fill the sprite structure */
      void getSprite( unsigned index, TimeSpriteMap& sprites ) const;
    };
    
  }
}
#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
