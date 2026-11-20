// Copyright (c) 2024 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef NDK_GEO_UTILS_DRAW_H
#define NDK_GEO_UTILS_DRAW_H

#include "ndk/api.h"

#include "usg/base/ArrayTypes.h"
#include "usg/geom/Path.h"

#include "fdk/base/PluginRegistry.h"
#include "fdk/base/Time.h"
#include "fdk/math/Mat4.h"

#include <cstdint> // for uint32_t
#include <functional>
#include <memory>
#include <optional>
#include <string>

namespace usg {
  using LayerRef = std::shared_ptr<class Layer>;
  class Prim;
  class Token;
}

namespace ndk {

  enum class Plane
  {
    XY,
    XZ,
    YZ,
    YX,
    ZX,
    ZY
  };

  enum class LineStyle
  {
    Solid,
    Dotted,
    DashedSmall,
    DashedMedium,
    DashedLarge,
    DashedVeryLarge,
    DashDotted,
    DashDotDash,
  };

  //! Convert a value from one linear scale to another.
  template <typename T>
  inline T linearConversion(T oldMin, T oldMax, T newMin, T newMax, T oldVal)
  {
    return ((oldVal - oldMin) * ((newMax - newMin) / (oldMax - oldMin))) + newMin;
  }

  //! Flags are defined as uints vs. enums so they can be OR'd & AND'd together.
  using DisplayOptionsMask = uint32_t;

  static constexpr DisplayOptionsMask   defaultDisplay() { return 0x00000000; }


  //! Options for drawing object icons.
  struct DisplayOptions
  {
    DisplayOptionsMask  mask;
    fdk::TimeValue time;
    fdk::Mat4f modelMatrix;
    fdk::Vec3f scale;
    double worldToMeters = 1.0;
    bool isSelected = false;
    uint32_t selectedColor = 0xe98828ff;
  };

  //! Base class to draw a USD locator in the viewer.
  //! DrawableLocator should be stateless as the same object will be used for
  //! the same type of Prims.
  class NDK_API DrawableLocator
  {
  public:
    virtual ~DrawableLocator() = default;

    //! Called to draw a locator in the viewer.
    virtual void drawLocator(const usg::Prim& prim, const DisplayOptions& displayOptions) = 0;

    //! The prim type this locator draws.
    virtual const usg::Token& primType() = 0;
  };
  using DrawableLocatorPtr = std::shared_ptr<DrawableLocator>;

  //! Registry class for Locator plugins.
  class NDK_API DrawableLocatorRegistry
  {
  public:
    //! Function type to build a DrawableLocator
    using DescriptionBuilder = std::function<DrawableLocatorPtr(void)>;

    //! Statically instantiate an object of this class to register the plugin.
    class NDK_API Description
    {
    public:
      DescriptionBuilder builderMethod;
      Description(const std::string&, DescriptionBuilder builder);
    };

    //! Constructs the registry indicating that \a pluginRegistry should be used
    //! to look for and load the locator plugins.
    DrawableLocatorRegistry(const std::shared_ptr<fdk::PluginRegistry>& pluginRegistry);
    ~DrawableLocatorRegistry();

    //! Checks if a locator plugin exists for the given \a primType.
    //! File system access will only be done the first time the method is called
    //! for any given \a primType. The results will be cached and reused.
    bool pluginExistsForPrim(const usg::Token& primType) const;

    //! Loads and registers a locator plugin for \a primType and
    //! returns the corresponding DrawableLocatorPtr if successful, nullptr
    //! otherwise.
    //! Will only try to load and register the plugin once for any give \a
    //! primType, caching the resulting DrawableLocatorPtr for next calls.
    DrawableLocatorPtr getDrawable(const usg::Token& primType) const;

    class Impl;
  private:
    std::unique_ptr<class DrawableLocatorRegistryImpl> _impl;
  };


  // Common drawables

  NDK_API void drawMitchellCamera(bool solid);

  //! Draws the supplied frustum. It is assumed the near plane is defined by the
  //! first 4 vertices and the far by the last 4 vertices.
  NDK_API void DrawFrustumOutline(const std::array<fdk::Vec3d, 8>& frustum);

  //! Draws the supplied cross hair consisting of 4 vertices.
  NDK_API void DrawCrossHair(const std::array<fdk::Vec3d, 4>& crossPoints);

  //! Draws a cylinder along an axis described by 2 vertices.
  NDK_API void DrawCylinder(const fdk::Vec3f& p1, const fdk::Vec3f& p2, float radius, int sides = 32);

  //! Draws a filled, backface-culled sphere.
  //! The smoothness of the sphere is controlled by the specified number of rows and columns.
  NDK_API void DrawSphere(const fdk::Vec3f& centre, float radius, int rows = 10, int cols = 20, bool filled = true);

  //! Draws a filled arrow perpendicular to the specified plane.
  //! The arrow will be drawn along the positive axis by default.
  //! pos: The position of the base of the arrow tail.
  //! headLength: The length of the arrow head.
  //! headRadius: The radius of the arrow head cone.
  //! tailLength: The length of the arrow tail.
  //! tailRadius: The radius of the arrow tail.
  //! plane: which plane to draw on: XY, YZ, XZ.
  //! reverse: If true, the arrow will be drawn along the negative axis.
  NDK_API void DrawArrow(const fdk::Vec3f& pos, float headLength = 0.2f, float headRadius = 0.075f, float tailLength = 0.8f, float tailRadius = 0.005f, Plane plane = Plane::XY, bool reverse = false);

  //! Draws a filled disk with an outline.
  NDK_API void DrawDisk(const fdk::Vec3f& centre, float radius, const fdk::Vec4f& fillColor, const fdk::Vec4f& outlineColor, Plane plane);

  //! Draws a torus.
  //! This is the general version which allows a partial torus (i.e. an arc) to be drawn between 2 angles.
  //! Any styling (color, fill mode, etc) is assumed to have already been applied.
  NDK_API void DrawTorus(const fdk::Vec3f& centre, float majorRadius, int majorSegments, float minorRadius, int minorSegments, Plane plane, float startAngle, float endAngle);

  //! Draws a torus.
  //! The shape will be filled by default but this can be overridden.
  NDK_API void DrawTorus(const fdk::Vec3f& centre, float majorRadius, float minorRadius, Plane plane, bool filled = true);

  //! Draws an arc.
  //! The shape will be filled by default but this can be overridden.
  NDK_API void DrawArc(const fdk::Vec3f& centre, float majorRadius, float minorRadius, float startAngle, float endAngle, Plane plane, bool filled = true);

  //! Draws a circle.
  NDK_API void DrawCircle(const fdk::Vec3f& centre, float diameter, Plane plane, bool filled = true, float normal = 1.0f);
  NDK_API void DrawCircle(const fdk::Vec3f& centre, float diameter, float lineWidth, Plane plane, bool filled = true, float normal = 1.0f);

  //! Draws a cube.
  NDK_API void DrawCube(const fdk::Vec3f& centre, float size);

  //! Draw a series of independant lines.
  NDK_API void DrawLines(const std::vector<std::pair<fdk::Vec3f, fdk::Vec3f>>& lines);

  //! Set the current drawing color.
  NDK_API void SetDrawColor(float r, float g, float b, float a);
  NDK_API void SetDrawColor(float r, float g, float b);
  NDK_API void SetDrawColor(const fdk::Vec4f& color);
  NDK_API void SetDrawColor(const fdk::Vec3f& color);
  NDK_API void SetDrawColor(const fdk::Vec3f& color, float a);
  NDK_API void SetDrawColor(unsigned int color);

  //! Set the current line style.
  NDK_API void SetLineStyle(LineStyle style);

  //! Set the current line width.
  NDK_API void SetLineWidth(float width);

  //! Convert a byte packed color to a vector of 4 elements.
  NDK_API fdk::Vec4f ConvertColor4f(unsigned int color);

  //! Convert a byte packed color to a vector of 3 elements.
  NDK_API fdk::Vec3f ConvertColor3f(unsigned int color);

  NDK_API void drawSphereLightLocator(float intensity, bool treatAsPoint, float radius, const fdk::Vec3f& outlineColor, const fdk::Vec3f& fillColor, const fdk::Vec3f& spokeColor);
  NDK_API void drawDistantLightLocator(float intensity, const fdk::Vec3f& outlineColor, const fdk::Vec3f& fillColor);
  NDK_API void drawDomeLightLocator(float intensity, float guideRadius, const fdk::Vec3f& outlineColor, const fdk::Vec3f& fillColor);
  NDK_API void drawDiskLightLocator(float intensity, float radius, float coneAngle, float coneSoftness, const fdk::Vec3f& outlineColor, const fdk::Vec3f fillColor, float coneTipRadius = 0.0f, bool treatAsSpotlight = false);
} // namespace ndk

#endif
