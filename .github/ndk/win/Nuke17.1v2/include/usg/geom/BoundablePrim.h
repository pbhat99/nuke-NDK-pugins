// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

// This is automatically-generated code.
// You may add custom code between the --(BEGIN CUSTOM CODE)-- and --(END CUSTOM CODE)-- markers.
// Code added anywhere else will be destroyed when the code is generated.

#ifndef USG_GEOM_BOUNDABLEPRIM_H
#define USG_GEOM_BOUNDABLEPRIM_H

#include "usg/api.h"

#include "usg/geom/GeomTokens.h"

#include "usg/geom/Prim.h"
#include "usg/base/ArrayTypes.h"
#include "usg/geom/XformablePrim.h"

namespace usg {

  // class BoundablePrim
  //   inherits from Xformable, Imageable
  //
  // Attributes:
  //     float3[] extent
  //
  // Inherited attributes:
  //   From Xformable:
  //     uniform token[] xformOpOrder
  //   From Imageable:
  //     rel proxyPrim
  //     uniform token purpose = "default" (allowed values: default, render, proxy, guide)
  //     token visibility = "inherited" (allowed values: inherited, invisible)

  class USG_API BoundablePrim : public XformablePrim {
  public:
    BoundablePrim();
    BoundablePrim(const Prim& prim);
    ~BoundablePrim();

    bool validate();
    static bool isValidPrim(const Prim& prim);
    static BoundablePrim overrideInLayer(const LayerRef& layer, const Prim& prim);
    static BoundablePrim getInStage(const StageRef& stage, const Path& path);

    Attribute createExtentAttr(const Value& defaultValue = Value());
    Attribute getExtentAttr() const;
    void setExtent(const Vec3fArray& value, fdk::TimeValue time = fdk::defaultTimeValue());
    Vec3fArray getExtent(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    // --(BEGIN CUSTOM CODE)--
    /*! Set the 'extent'(bounds) attribute which can significantly speed things
        up for downstream geometry consumers. Will create the property if required.
        Note - variability_hint arg is only respected when first creating(defining) the property.
    */
    virtual bool      setBoundsAttr(const fdk::Box3f&     point_bbox,
                                    const fdk::TimeValue& time = fdk::defaultTimeValue());

    /*! Set the 'extent'(bounds) attribute from the union of all points in the list.
        Will create the property if required.
        Note - variability_hint arg is only respected when first creating(defining) the property.
    */
    virtual bool      setBoundsAttr(const usg::Vec3fArray& points,
                                    const fdk::TimeValue&  time = fdk::defaultTimeValue());

    //! Retrieve the 'extent'(bounds) property.
    virtual Attribute   getBoundsAttr() const;

    //! Retrieve the bounds property value as a Box3f. If there's no attribute optionally build from point locations.
    virtual fdk::Box3f  getBoundsBox(const fdk::TimeValue& time = fdk::defaultTimeValue(),
                                     bool                  build_from_points_fallback = false) const;
    // --(END CUSTOM CODE)--
  };

}

#endif
