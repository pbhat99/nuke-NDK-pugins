// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef USG_GEOM_LIGHTPRIM_H
#define USG_GEOM_LIGHTPRIM_H

#include "usg/geom/XformablePrim.h"

namespace usg
{

  //--------------------------------------------------------------
  // Inherited standard attributes from ImageablePrim class:
  //  token          visibility = "inherited" [inherited, invisible]
  //  uniform token  purpose = "default" [default, render, proxy, guide]
  //  <relationship> proxyPrim
  //
  // Inherited standard attributes from XformablePrim class:
  //  uniform token[] xformOpOrder
  //
  // Standard attributes for LightPrim:
  //  float   intensity = 1
  //  float   exposure = 0
  //  float   diffuse = 1
  //  float   specular = 1
  //  bool    normalize = 0
  //  color3f color = (1, 1, 1)
  //  bool    enableColorTemperature = 0
  //  float   colorTemperature = 6500
  //


  class USG_API LightPrim : public usg::XformablePrim
  {
  public:
    //! Default ctor builds an empty (invalid) prim.
    LightPrim();
    //! Does not test if castable to LightPrim!
    LightPrim(const Prim&);
    ~LightPrim();


    /*! Call this after construction to validate this Prim as a LightPrim type, returning
        true if it's still valid.
        If not the Prim::Impl will be invalidated and isValid() will then return false.
    */
    bool validate() override;

    //! Returns true if prim is a LightPrim subclass.
    static bool  isValidPrim(const Prim& prim);


    /*! Create a ShaderDesc instance from this ShaderPrim, sampled at an optional time,
        filling in any overrides or connections present.
        The parent_material prim is used to create a relative path for the resulting
        ShaderDesc inside the material group.

        Uses the schema name for the registry id, then converts each property
        override and connection.

        Enabling debug will print shader creation debug info to terminal (cout).

        Calling code takes ownership of ShaderDesc allocation.
    */
    ShaderDesc*  createShaderDesc(const fdk::TimeValue& time = fdk::defaultTimeValue()) const;

    Attribute createColorAttr(const Value& defaultValue = Value());
    Attribute getColorAttr() const;
    void setColor(const fdk::Vec3f& value, fdk::TimeValue time = fdk::defaultTimeValue());
    fdk::Vec3f getColor(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createColorTemperatureAttr(const Value& defaultValue = Value());
    Attribute getColorTemperatureAttr() const;
    void setColorTemperature(float value, fdk::TimeValue time = fdk::defaultTimeValue());
    float getColorTemperature(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createDiffuseAttr(const Value& defaultValue = Value());
    Attribute getDiffuseAttr() const;
    void setDiffuse(float value, fdk::TimeValue time = fdk::defaultTimeValue());
    float getDiffuse(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createEnableColorTemperatureAttr(const Value& defaultValue = Value());
    Attribute getEnableColorTemperatureAttr() const;
    void setEnableColorTemperature(bool value, fdk::TimeValue time = fdk::defaultTimeValue());
    bool getEnableColorTemperature(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createExposureAttr(const Value& defaultValue = Value());
    Attribute getExposureAttr() const;
    void setExposure(float value, fdk::TimeValue time = fdk::defaultTimeValue());
    float getExposure(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createIntensityAttr(const Value& defaultValue = Value());
    Attribute getIntensityAttr() const;
    void setIntensity(float value, fdk::TimeValue time = fdk::defaultTimeValue());
    float getIntensity(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createNormalizeAttr(const Value& defaultValue = Value());
    Attribute getNormalizeAttr() const;
    void setNormalize(bool value, fdk::TimeValue time = fdk::defaultTimeValue());
    bool getNormalize(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Attribute createSpecularAttr(const Value& defaultValue = Value());
    Attribute getSpecularAttr() const;
    void setSpecular(float value, fdk::TimeValue time = fdk::defaultTimeValue());
    float getSpecular(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    Relationship createFiltersRel();
    Relationship getFiltersRel() const;
    void setFilters(const PathArray& value);
    PathArray getFilters() const;

  };


} // namespace usg

#endif
