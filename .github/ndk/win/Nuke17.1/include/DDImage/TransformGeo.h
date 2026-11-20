// TransformGeo.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_TransformGeo_h
#define DDImage_TransformGeo_h

#include "DDImage/GeoOp.h"
#include "DDImage/LookAt.h"

#ifdef _WIN32
  #pragma warning(disable:4251)
#endif

namespace DD
{
  namespace Image
  {

    class AxisOp;

    /*! \class DD::Image::TransformGeo

        \brief Operator that transforms geometric points.
     */
    class DDImage_API TransformGeo : public GeoOp, public DD::Image::LookAt
    {
    public:
      TransformGeo(Node*);

      int minimum_inputs() const override;
      int maximum_inputs() const override;

      /*! Return a null geo for input0 and NULL for any other. */
      Op* default_input(int input) const override;

      /*! Only GeoOp allowed on zero, only AxisOp allowed on 1 */
      bool test_input(int input, Op*) const override;
      const char* input_label(int input, char*) const override;

      AxisOp* axis_input() const { return inputs() < 2 ? nullptr : (AxisOp*)(Op::input(1)); }
      /// override from LookAt
      Op* lookat_input() const override { return inputs() < 3 ? nullptr : (Op*)(Op::input(2)); }

      const Matrix4& matrix() const { return matrix_; }
      const Matrix4& concat_matrix() const { return concat_matrix_; }
      virtual void matrix(const Matrix4& m) { matrix_ = m; }
      GeoOp* concat_input() const { return concat_input_; }

      void knobs(Knob_Callback) override;
      int knob_changed(Knob *k) override;

      static const Description description;
      const char* Class() const override;
      const char* node_help() const override;

    protected:
      /* Object matrices: */
      Matrix4 matrix_;    //!< Object matrix - parent * local
      Matrix4 concat_matrix_; //!< Concatented input matrix * matrix_
      GeoOp* concat_input_; //!< Op this one concatenates its matrix with

      /*! Validate our parent axis first, if any, then apply our local
          transform to that.
       */
      void _validate(bool for_real) override;

      /*! Hash the matrix so that any change causes the points to be invalid.
       */
      void get_geometry_hash() override;

      /*! Apply the concat matrix to all the GeoInfos.
       */
      void geometry_engine(Scene&, GeometryList& out) override;

      /*! The default GeoOp::build_handles will build a Scene object and draw
         it. This is not needed by TransformGeo, as it can just change the OpenGL
         transform and then ask the input to draw. Also the Axis knobs do not draw
         right unless it sets the transform for any parent transform.
       */
      void build_handles(ViewerContext*) override;
   };

  }
}

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
