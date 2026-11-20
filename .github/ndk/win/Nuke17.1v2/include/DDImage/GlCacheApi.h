// Copyright (c) 2012 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef GLCacheApi_h
#define GLCacheApi_h

#include "DDImage/DDImage_API.h"
#include "DDImage/Vector2.h"
#include "Build/fnBuild.h"
#include "DDImage/Deprecation.h"
#include <memory>

#if !defined(FN_COMPILER_INTEL) && defined(FN_OS_WINDOWS)
#  pragma warning(push)
#  pragma warning( disable : 4251 ) // warning #4251: needs to have dll-interface to be used by clients of <X>
#endif // FN_OS_WINDOWS &&  FN_COMPILER_INTEL

namespace DD {

  namespace Image {
    class GLCacheImpl;
    class DDImage_API GLCache
    {
      // Non-copyable 
      GLCache(const GLCache&);
      GLCache& operator =(const GLCache&);
      std::unique_ptr<GLCacheImpl> _pImpl;

    public:
      GLCache();
      ~GLCache();

      // Enable caching of OpenGL functions.
      void enable();

      // Disable caching of OpenGL functions.
      void disable();

      // Reset the cache. All cached functions are deleted.
      void invalidate();

      // Creates a new frame in the cache.
      // The function does nothing if the cache is disabled.
      void beginFrame(double frame, int event);
      
      // Flags the frame as fully cached. After calling this function no more calls will 
      // be cached for this frame. 
      // The function does nothing if the cache is disabled.
      void endFrame(double frame, int event);
      
      // Calls all the cached functions.
      // This function can only be called after a frame has been fully cached by calling
      // endFrame.
      // The function does nothing if the cache is disabled.
      void executeFrame(double frame, int event);
      
      // The following two functions will be in a utility class in a future version.

      // Returns true if either Nuke's tree version or Nuke's handle tree version have changed.
      // We always must call this function with the following values for the parameters:
      // treeVersion = ::GlobalVersion.treeVersion()
      // handleTreeVersion = ::GlobalVersion::handleTreeVersion()
      bool checkHandlesChange(int treeVersion, int handleTreeVersion);
      
      // Returns true if the viewport size changes. 
      // We must pass to this function the current viewer's width and height.
      bool checkViewportSizeChange(int width, int height);
      bool checkZoomChange(float zoom);
      bool checkPanChange(float xpan, float ypan);
      
      // Check if the frame has been cached for the event.
      bool frameIsCached(double frame, int event);
    };
    
    // Cached OpenGL calls
    void  DDImage_API cacheglBegin(unsigned int mode);
    void  DDImage_API cacheglEnd();
    void  DDImage_API cacheglVertex2f(float x, float y);
    void  DDImage_API cacheglVertex2fv(const float* v);
    void  DDImage_API cacheglVertex2d(double x, double y);
    void  DDImage_API cacheglVertex3f(float x, float y, float z);
    void  DDImage_API cacheglVertex3fv(const float* c);
    void  DDImage_API cacheglNormal3f(float nx, float ny, float nz);
    void  DDImage_API cacheglNormal3fv(const float* n);
    void  DDImage_API cacheglColor3f(float r, float g, float b);
    void  DDImage_API cacheglColor4fv(const float* c);
    void  DDImage_API cacheglLineWidth(float width);
    void  DDImage_API cacheglLineStipple(int factor, short pattern);
    void  DDImage_API cacheglPushAttrib(unsigned int mask);
    void  DDImage_API cacheglPopAttrib();
    void  DDImage_API cacheglPushMatrix();
    void  DDImage_API cacheglPopMatrix();
    void  DDImage_API cacheglMatrixMode(unsigned int mode);
    void  DDImage_API cacheglLoadIdentity();
    void  DDImage_API cacheglEnable(unsigned int cap);
    void  DDImage_API cacheglDisable(unsigned int cap);
    void  DDImage_API cacheglHint(unsigned int target, unsigned int mode);
    void  DDImage_API cacheglLoadName(unsigned int name);
    void  DDImage_API cacheglLoadMatrixf(const float* m);
    void  DDImage_API cacheglStoreColor(float* c);
    void  DDImage_API cacheglRestoreColor(float* c);
    void  DDImage_API cacheglPointSize(float size);
    void  DDImage_API cacheglTranslatef(float tx, float ty, float tz);
    void  DDImage_API cacheglScaled(double sx, double sy, double sz);
    void  DDImage_API cacheglAlphaFunc(unsigned int func, float ref);
    void  DDImage_API cacheglBlendFunc(unsigned int sfactor, unsigned int dfactor);
    void  DDImage_API cacheglDepthMask(bool flag);
    void  DDImage_API cacheglClear(unsigned long mask);
    void  DDImage_API cacheglDepthFunc(unsigned int func);
    void  DDImage_API cacheglFlush();
    void  DDImage_API cacheglPolygonMode(unsigned int face, unsigned int mode);
    void  DDImage_API cacheglColor4f(float r, float g, float b, float a);
    void  DDImage_API cacheglTexCoord2f(float s, float t);
    void  DDImage_API cacheglVertex2i(int x, int y);
    void  DDImage_API cacheglScalef(float sx, float sy, float sz);
    void  DDImage_API cacheglRectf(float x1, float y1, float x2, float y2);
    void  DDImage_API cacheglOrtho(double left, double right, double bottom, double top, double zNear, double zFar);
    void  DDImage_API cacheglRotatef(float angle, float x, float y, float z);
    void  DDImage_API cacheglRotated(double angle, double x, double y, double z);

    // Utility graphic functions
    void DDImage_API cacheuglColor(unsigned int color);
    void DDImage_API cacheuglText(const char* str, float x, float y, float z = 0.0f);
    void DDImage_API cacheuglZOffsetProjection(float offsetAmount);
    void DDImage_API cacheuglBezierf(float x, float y,
                                     float x1, float y1,
                                     float x2, float y2,
                                     float x3, float y3,
                                     float scale = 1.0f);
    void DDImage_API cacheuglProjectionMatrix();
    void DDImage_API cacheuglRectangle(float x, float y, float r, float t);
    void DDImage_API cacheuglEllipse(float center_x, float center_y,
                                      float xradius, float yradius, unsigned nPoints);
    void DDImage_API cacheuglPolygon(const DD::Image::Vector2* points, size_t nPoints,
                                     DD::Image::Vector2 offset = DD::Image::Vector2());
  }
}

#if !defined(FN_COMPILER_INTEL) && defined(FN_OS_WINDOWS)
#  pragma warning(pop)  // reenable 4251: needs to have dll-interface to be used by clients of <X>
#endif // FN_OS_WINDOWS &&  FN_COMPILER_INTEL

#endif // GLCacheApi_h
