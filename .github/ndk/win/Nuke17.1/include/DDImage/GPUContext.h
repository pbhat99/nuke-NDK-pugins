// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef _DDImage_GPUContext_h_
#define _DDImage_GPUContext_h_

#include <string>
#include <vector>

#include <GL/glew.h>
#include "DDImage/DDImage_API.h"

namespace DD
{
  namespace Image
  {
    class ChannelSet;
    class Matrix3;
    class Matrix4;

    //! Abstracts the Viewer GPU pipeline in an independent way.
    //! To be used by GPU Ops that implement the OpenGL callbacks.
    class DDImage_API GPUContext
    {
    public:
      explicit GPUContext();
      ~GPUContext();

      /// @name Internal
      /// Nuke internal use only.
      //@{
      void init();
      void fini();
      bool isInitialised() const;
      void attachGLSLProgram(void*);
      void resetAllTextureUnits();
      void setWidth(int width);
      void setHeight(int height);
      void resetCurrentOp();
      void incrementCurrentOp();
      void decrementCurrentOp();
      std::string currentOpPrefix() const;
      void setRenderChannels(const ChannelSet& channels);
      const ChannelSet& getRenderChannels() const;
      //@}

      /// @name GPU
      /// GPU context public interface.
      //@{
      int acquireTextureUnit();
      void releaseTextureUnit(int unit);
      int getNumFreeTextureUnits() const;
      //@}

      /// @name Width and Height
      /// Get the computed result width and height
      //@{
      int width() const;
      int height() const;
      //@}

      /// @name GLSL
      /// GLSL program interface
      //@{
      bool enable() const;
      bool disable() const;

      /// Program handle.
      GLhandleARB programHandle() const;

      /// Bind a bool to the named variable.
      bool bind(const std::string& name, bool v) const;

      /// Bind an integer to the named variable.
      bool bind(const std::string& name, int v) const;

      /// Bind a float to the named variable.
      bool bind(const std::string& name, float v) const;

      /// Bind a bvec2, bvec3 or bvec4 to the named variable.
      bool bind(const std::string& name, int siz, int count,
                const bool v[]) const;

      /// Bind a ivec2, ivec3 or ivec4 to the named variable.
      bool bind(const std::string& name, int siz, int count,
                const int v[]) const;

      /// Bind a vec2, vec3 or vec4 to the named variable.
      bool bind(const std::string& name, int siz, int count,
                float v[]) const;

      /// Bind a Matrix3 to the named variable.
      bool bind(const std::string& name, const Matrix3& mat) const;

      /// Bind a Matrix4 to the named variable.
      bool bind(const std::string& name, const Matrix4& mat) const;
      //@}
      
    private:
      class GPUInternals;
      GPUInternals* _internals;
    };

  }
}

#endif
