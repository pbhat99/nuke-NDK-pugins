#ifndef DDImage_RenderParticles_H
#define DDImage_RenderParticles_H

#include "DDImage/Particles.h"
#include "DDImage/Point.h"

namespace DD {
  namespace Image {
    DDImage_API Particles* MakeRenderParticles(Point::RenderMode renderMode, unsigned points, unsigned start, bool haveNormals, float particleSize);
    DDImage_API float GetRenderParticlesPointSize(const Primitive* prim);
  }
}

#endif
