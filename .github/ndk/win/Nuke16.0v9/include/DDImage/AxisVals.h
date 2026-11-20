//------------------------------------------------------------------------------
// DDImage/AxisVals.h
//
// Copyright (c) 2021 The Foundry Visionmongers Ltd.  All Rights Reserved.
//------------------------------------------------------------------------------

#ifndef DDIMAGE_AXISVALS_H
#define DDIMAGE_AXISVALS_H

#include "DDImage/DDImage_API.h"

#include "fdk/math/Vec3.h"
#include "fdk/math/Mat4.h"
#include "fdk/base/Time.h"

namespace DD
{
  namespace Image
  {
    class AxisVals;
    using AxisValsArray = std::vector<AxisVals>;  // Typically used for animation curves


    /*! Encapsulates all the parameters in an Axis_Knob so they can be considered
        as a set and processed as arrays of coherent values.
    */
    class DDImage_API AxisVals
    {
    public:
      fdk::TimeValue   time;              //!< Sample time

      // Parent xform params:
      bool             parent_enable;     //!< Are parent transform values being used?
      fdk::Vec3d       parentT;           //!< Parent xform translation
      fdk::Vec3d       parentR;           //!< Parent xform rotation angles *in degrees* (always XYZ order)
      fdk::Vec3d       parentS;           //!< Parent xform scale

      // Local xform params:
      fdk::XformOrder  xform_order;       //!< (matches the Axis_KnobI enums)
      fdk::RotateOrder rot_order;         //!< (matches the Axis_KnobI enums)
      fdk::Vec3d       translate;         //!< Local xform translation
      fdk::Vec3d       rotate;            //!< Local xform rotation angles *in degrees*
      fdk::Vec3d       scaling;           //!< Local xform scale factor
      double           uniform_scale;     //!< Local xform scalar value multiplied with scaling vector
      fdk::Vec3d       skew;              //!< Local xform skew/shear
      fdk::Vec3d       pivotT;            //!< Local xform pivot translation (the location of the rotation/scale/shear center)
      fdk::Vec3d       pivotR;            //!< Local xform pivot rotation angles *in degrees* (always XYZ order)

      // Local xform override:
      bool             use_matrix;        //!< Ignore separate transform params, use an explicit matrix
      fdk::Mat4d       matrix;            //!< If use_matrix is true this defines the local xform


    public:
      //! Default ctor leaves junk in values.
      AxisVals() = default;

      //! Assigns standard default values to all vars.
      AxisVals(const fdk::TimeValue& time,
               bool                  _parent_enable);

      //----------------------------------------------------------------------

      //! Compares time value. Used by the sort routine.
      bool operator < (const AxisVals& b) const { return (time < b.time); }

      //! Print values out.
      void        print(const char*   prefix,
                        std::ostream& o) const;

      //----------------------------------------------------------------------

      /*! Build a matrix from the current parent TRS values.
          Transform order is always SRT and rotation order is always XYZ.
      */
      fdk::Mat4d  parentTransform() const;

      /*! Build a matrix from the current local TRS values - unless \a use_matrix
          is true, then the \a matrix var is returned.
          \a xform_order and \a rot_order control the composition sequence.
      */
      fdk::Mat4d  localTransform()  const;

      //! Get the total scaling factor as a Vec3 (scaling * uniform_scale.)
      fdk::Vec3d  totalScaling() const { return (scaling * uniform_scale); }

      //----------------------------------------------------------------------

      //! Assigns standard default values to all params.
      void        setToDefault(const fdk::TimeValue& _time = fdk::defaultTimeValue());

      //! Assigns standard default values to transform params (rotate, scale, etc.)
      void        setLocalXformValsToDefault();
      void        setParentXformValsToDefault();

      //! Return true if xform vals are at default settings.
      bool        isLocalXformValsDefault() const;
      bool        isParentXformValsDefault() const;

      //! Enable the parent knobs so that they are stored and sampled.
      void        enableParentXformVals(bool enable = true) { parent_enable = enable; }

      /*! Decompose the matrix into TRS values, targeting either the local or
          parent TRS params. If a TRS enable is false the respective param is
          set to its default value.
          Targeting the parent TRS will also turn on \a parent_enable.
      */
      bool        extractFromMatrix(const fdk::Mat4d& m,
                                    bool              T_enable,
                                    bool              R_enable,
                                    bool              S_enable,
                                    fdk::RotateOrder  decompose_rot_order,
                                    bool              apply_to_parent = false);

      //----------------------------------------------------------------------

      /*! Apply an euler filter to the local and parent rotations in the array of
          AxisVals. This is typically needed after matrix decomposition of a series
          of animated matrix keyframes.

          \a target_rot_order specifies the preferred rotation order for the filter
          to use on the entire series, usually taken from \a rot_order of the first
          AxisVal.

          If \a sort is true the \a vals array is first sorted by time, then filtered.
      */
      static void applyEulerFilter(fdk::RotateOrder target_rot_order,
                                   AxisValsArray&   vals,
                                   bool             sort = false);

    };

  }
}
#endif
