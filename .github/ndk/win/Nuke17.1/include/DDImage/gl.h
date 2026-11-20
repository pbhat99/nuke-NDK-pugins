// gl.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

/*! \file gl.h
   This header file allows a plugin to include the OpenGL header file
   in a portable way. It also defines some OpenGL convienence functions.
 
   From Nuke 16.0, on macOS only, Nuke no longer uses the system OpenGL library,
   /System/Library/Frameworks/OpenGL.framework, which Apple has deprecated.
   Instead, Nuke uses FoundryGL, an alternative library developed by Foundry
   that mostly matches the OpenGL API. For more information see
   "Appendix A: Setting up Projects & Compilers » macOS" in the
   NDK Developers Guide.

   \b Note: Linux and Windows versions will continue to use the system OpenGL.
*/

#ifndef DDImage_gl_H
#define DDImage_gl_H

#include "DDWindows.h"
#include "ViewerContext.h"

#if USE_GLEW
  #include <GL/glew.h>
#elif defined(__GLEW_H__)
/* do nothing if they included glew.h */
#else
  #define GL_GLEXT_PROTOTYPES 1
  #if defined(__APPLE__)
    #if defined(FOUNDRYGL_USE)
      #include <FoundryGL/FoundryGL.h>
    #else
      #include <OpenGL/gl.h>
    #endif
  #else
    #include <GL/gl.h>
  #endif
#endif

#include "DDImage_API.h"

/*////////////////////////////////////////////////////////////// */

#include "Box.h"
#include "Box3.h"
#include "Matrix4.h"
#include "Vector2.h"

// Utility macro for checking OpenGL errors after calls
#ifdef FN_DEBUG
#define mFnGlCall(fun) fun; DD::Image::glGetErrors();
#else
#define mFnGlCall(fun) fun
#endif //FN_DEBUG

namespace DD
{

namespace Image
{

/*! Set the current OpenGL color to a color specified by the bytes of
    the unsigned value. The highest byte is red, the next green, then
    blue, then alpha. For instance 0xff00ff00 is purple (red+blue).
    This is the same form the Nuke and fltk use to identify GUI colors.
    Currently the alpha value is ignored.
 */
DDImage_API void glColor(unsigned);
DDImage_API void glColor4(unsigned);
DDImage_API void glColor4fvLUT(const float*);

/*! Draw the null-terminated string, appending it to the previous string
    or at the last call to glRasterPos. */
DDImage_API void gl_text(const char* str);

/*! Draw the first n bytes of the string, appending them to the previous
    string or last call to glRasterPos. */
DDImage_API void gl_text(const char* str, int n);

/*! Draw the null-terminated string, at position x,y,z.
    The font is controlled by the current glListBase(), Nuke will
    have preset this and it is probably best to leave it alone.
 */
DDImage_API void gl_text(const char* str, float x, float y, float z = 0);

/*! Draw the first n bytes of the string at position x,y,z */
DDImage_API void gl_text(const char* str, int n, float x, float y, float z = 0);

/*! Draw the null-terminated string, at position x,y,z.
    The font is controlled by the current glListBase(), Nuke will
    have preset this and it is probably best to leave it alone.
 */
DDImage_API void gl_text(const char* str, int x, int y, int z = 0);

/*! Draw the first n bytes of the string at position x,y,z */
DDImage_API void gl_text(const char* str, int n, int x, int y, int z = 0);
DDImage_API void gl_text(const char* str, double x, double y, double z = 0);
DDImage_API void gl_text(const char* str, int n, double x, double y, double z = 0);

DDImage_API void gl_enable_line_smoothing(float width);
DDImage_API void gl_disable_line_smoothing();

/*! gl_circlef: Draw a circle or disc.
    Uses a precalculated cosine table of 51 entries encompassing 360degs.
    x, y, z = position of center of circle
    diam    = diameter of circle
    plane   = which plane to draw on: XY, YZ, XZ - Provided for convenience
    filled  = whether to make them polygons
 */
DDImage_API void gl_circlef(float x, float y, float z, float diam, int plane, bool filled = false, float normal = 1.0f);

/*! gl_circlefAsTris: Draw a circle or disc using a tristrip.
    Uses a precalculated cosine table of 51 entries encompassing 360degs.
    x, y, z = position of center of circle
    diam    = diameter of circle
    lineWidth = width of the circle border
    plane   = which plane to draw on: XY, YZ, XZ - Provided for convenience
    filled  = whether to make them polygons
 */
DDImage_API void gl_circlefAsTris(float x, float y, float z, float diam, float lineWidth, int plane, bool filled = false, float normal = 1.0f);

/*! gl_cylinder: Draw two connected discs forming a cylinder.
    Uses a precalculated cosine table of 51 entries encompassing 360degs.
    x, y, z = position of center of cylinder
    bottomCapDiameter = diameter of endcap 1
    topCapDiameter = diameter of endcap 2
    plane   = which plane to draw in: XY, YZ, XZ - Provided for convenience
    filled  = whether to make them polygons
 */
DDImage_API void gl_cylinderf(float x, float y, float z, float width,
                              float bottomCapDiameter, float topCapDiameter, int plane,
                              bool capped = true, bool filled = false);

/*! gl_cone: Draw a open or closed cone.
    Uses a precalculated cosine table of 51 entries encompassing 360degs.
    x, y, z = position of tip
    length  = length of cone
    diam    = diameter of cone
    plane   = which plane to draw on: XY, YZ, XZ - Provided for convenience
 */
DDImage_API void gl_conef(float x, float y, float z, float length, float diam, int plane);

/*! Draw a wireframe sphere around 0,0,0 */
DDImage_API void gl_sphere(float radius = 0.5);

enum { XY, XZ, YZ, YX, ZX, ZY };

/*! Draw a flat plane in any of the three basic planes */
DDImage_API void gl_planef(float x, float y, float z, float width, float height, int plane);
DDImage_API void gl_gridf(float x, float y, float z, int numW, int numH, int plane);

/*! gl_quadi: Draws a filled quad (int version).
    x       = left bottom value along x axis
    y       = left bottom value along y axis
    right   = right top value along x axis
    top     = right top value along y axis
 */
DDImage_API void gl_quadi(int x, int y, int right, int top);

/*! gl_rectangle: Draw the outline of a 2-D rectangle (float version).
    x       = left bottom value along x axis
    y       = left bottom value along y axis
    right   = right top value along x axis
    top     = right top value along y axis
 */
DDImage_API void gl_rectangle(float x, float y, float r, float t); // outlined

/*! gl_rectangle: Draw the outline of a 2-D rectangle (int version).
    x       = left bottom value along x axis
    y       = left bottom value along y axis
    r       = right top value along x axis
    t       = right top value along y axis
 */
DDImage_API void gl_rectangle(int x, int y, int r, int t);

/*! gl_circle: Draw the outline of a 2-D circle.
    x, y = position of center of cylinder
    radius  = radius of circle
    intervalDegrees = number of points that define the precision of the shape of circle -
              the more points are used the more accurate shape of circle is drawn
 */
DDImage_API void gl_circle(float x, float y, float radius, float intervalDegrees);

/*! gl_ellipse: Draw the outline of a 2-D ellipse.
    x, y = position of center of ellipse
    xradius = radius of ellipse along x axis
    yradius = radius of ellipse along y axis
    nPoints = number of points that define the precision of the shape of ellipse -
              the more points are used the more accurate shape of ellipse is drawn
 */
DDImage_API void gl_ellipse(float x, float y, float xradius, float yradius, unsigned nPoints);

/*! gl_bracket: Draw brackets using 2-D lines.
    x       = left bottom value along x axis
    y       = left bottom value along y axis
    r       = right top value along x axis
    t       = right top value along y axis
    sx      = length of a bracket segment along x axis
    sy      = length of a bracket segment along y axis
 */
DDImage_API void gl_bracket(float x, float y, float r, float t, float sx, float sy);

/*! gl_crossf: Draw a cross on a 3-D axes plane (float version).
    x, y, z = position of cross
    size    = length of a segment from the cetner to a side of cross
    plane   = which plane to draw on: XY, YZ, XZ - Provided for convenience
 */
DDImage_API void gl_crossf(float x, float y, float z, float size, int plane);

/*! gl_crossi: Draw a cross on a 3-D axes plane (int version).
    x, y, z = position of cross
    size    = length of a segment from the cetner to a side of cross
    plane   = which plane to draw on: XY, YZ, XZ - Provided for convenience
 */
DDImage_API void gl_crossi(int x, int y, int z, int size, int plane);

/*! Draw a 3-D T with each beam 1 unit long */
DDImage_API void gl_axis();

/*! Draw labels at the ends of the T drawn by gl_axis() */
DDImage_API void gl_axis_label();

/*! gl_boxf: Draw a 3-D box.
    x       = position of the bottom left point along the x axis
    y       = position of the bottom left point along the y axis
    n       = position of the near bottom left point along the z axis
    r       = position of the bottom right point from the origin along the x axis
    f       = position of the far bottom left point along the z axis
    t       = position of the top left point from the origin along the y axis
    filled  = pass true for drawing the box filled in a solid color or false for the outline
 */
DDImage_API void gl_boxf(float x, float y, float n, float r, float t, float f, bool filled = false);

/*! gl_cubef: Draw a 3-D cube.
    x, y, z = position of center of cube
    width   = width of cube edges
 */
DDImage_API void gl_cubef(float x, float y, float z, float width);

/*! Call glVertex2f with points on a 2-D bezier curve. x,y are
    the initial point, which is not drawn (this is done so several curves
    join together, you should draw the point yourself for the first curve
    in the sequence. The curve goes to x3,y3. x1,y1 and x2,y2 are the
    bezier handles.

    Enough points are drawn so that the error of the curve is less than
    one pixel anywhere and the straight edges should not be visible. To
    to this it needs to know how big one unit is in pixels, this can be
    passed as \a scale.

    This should not be used for curves that are way bigger than the
    viewport. The algorithim would normally divide the line into millions
    of pieces and waste a huge amount of time drawing the clipped pieces.
    This version does a maximum of 100 segments, so the machine does not
    lock up, but the result is then inaccurate.
 */
DDImage_API void gl_bezierf(float x, float y,
                            float x1, float y1,
                            float x2, float y2,
                            float x3, float y3,
                            float scale = 1);

/*! gl_polygon: Draw the outline of a 2-D polygon.
    points  = points that define the shape of polygon
    nPoints = number of given points
    offset  = drawing offsets along horizontal and vertical axes
 */
DDImage_API void gl_polygon(const DD::Image::Vector2* points, size_t nPoints, DD::Image::Vector2 offset = DD::Image::Vector2());

//! draws text in the top left of the ViewerWindow tied to the ViewerContext. By default draws it in red text, if you
//! want another colour, set redColour to false and set the colour before calling this then pop the attribute after calling this.
DDImage_API void gl_drawTextTopLeftOrthogonal(const std::string& text, DD::Image::ViewerContext* ctx, bool redColour = true);

// Routines that emulate GLU calls:

/*! Print GL errors - reasonably emulates gluErrorString */
DDImage_API const char* gl_errorstring(int errorcode);

/*! Clear accumulated OpenGL errors. If FN_DEBUG is on, it prints them to
   stderr, prefixed with \a message. Returns number of errors found.
 */
DDImage_API int glGetErrors(const char* message = nullptr);

// Routines that are a *bit* like GLU calls:
DDImage_API bool GLProject(float x, float y, float z, const DD::Image::Matrix4& m, const DD::Image::Box& vp, float* winx, float* winy, float* winz);
DDImage_API bool GLUnproject(float wx, float wy, float wz, const DD::Image::Matrix4& m, const DD::Image::Box& vp, float* objx, float* objy, float* objz);
DDImage_API DD::Image::Box GLProjectCube(const DD::Image::Box3& cube, const DD::Image::Matrix4& m, const DD::Image::Box& vp);
DDImage_API void GLZOffsetProjection(float offsetAmount);

} // namespace Image

} // namespace DD

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
