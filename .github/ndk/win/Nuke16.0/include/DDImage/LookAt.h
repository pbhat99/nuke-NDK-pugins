// LookAt.h
// Copyright (c) 2014 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_LookAt_h
#define DDImage_LookAt_h

#include "DDImage/Matrix4.h"
#include "DDImage/Knobs.h"

#ifdef _WIN32
  #pragma warning(disable:4251)
#endif

namespace DD
{
  namespace Image
  {

    class AxisOp;

    /*! \class DD::Image::LookAt

        \brief Helper class that adds LookAt functionality
     */
    class DDImage_API LookAt
    {
    public:

      LookAt(int defaultLookAxis = kAxisZPlus );
      virtual ~LookAt() {}

      /// The lookat input op
      // subclasses should override to return the an Axis or TransformGeo op to look at
      virtual Op* lookat_input() const  = 0;

      /// Adds the 'Look' tab and knobs for all the look at options
      void knobs(Knob_Callback f);

      /// disables the look at input is not present
      int knobChanged(Op* parent, Knob* k);

      /// performs the look at on the incoming matrix
      void lookAt( Matrix4& matrix );

      /// performs the look at on the incoming matrix for a given context
      // Note that this value is only valid if the lookat input is an Axis
      void lookMatrixAt(const OutputContext& oc, Matrix4& concatMatrix);

    protected:

      void perform_lookat(Matrix4& concatMatrix, const Matrix4& axisMatrix);

      Matrix4 lookat_matrix_; //!< Matrix for rotating to look at the look input

      enum {
        kAxisZPlus = 0,
        kAxisZMinus,
        kAxisYPlus,
        kAxisYMinus,
        kAxisXPlus,
        kAxisXMinus
      };

      static const char* const ms_lookat_axis_choices[];

      Matrix4    my_local; //!< For the Axis_Knob to store into
      bool       my_transform_normals;
      int        my_lookat_axis;
      bool       my_rotate_x;
      bool       my_rotate_y;
      bool       my_rotate_z;
      bool       my_lookat_use_quat;
      double     my_lookat_strength;
    };

  }
}

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
