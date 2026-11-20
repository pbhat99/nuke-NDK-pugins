#ifndef DD_IMAGE_QUATERNION_H
#define DD_IMAGE_QUATERNION_H

#include "DDImage_API.h"
#include "Vector3.h"
#include "Matrix4.h"

#include <cmath>
#include <iostream>

namespace DD
{
  namespace Image
  {
  
    /*! \class DD::Image::TQuaternion Quaternion.h

       \brief A implementation of a way of representing rotations that avoid a lot
       of the problems that the standard rotation about the axis methods have.

       Quaternions are a modification of the concept of a vector in space,
       but specially tailored for spherical space. The cool thing about
       quaternions is that they are perfectly suited to representing
       rotations and orientations of objects in three space.

       Basically, in a quaternion there are four values: a scalar part and
       a vector part. <b>q</b> = ( s, <b>v</b> ). Typically, when dealing
       with rotations, the scalar part represents the rotation about an
       arbitrary axis. The axis is represented by a unit vector in the
       vector part.

       Since the quaternion is a representation of a rotation, it can be
       converted into a Euler angle rotation matrix and a rotation matrix
       can be converted into a quaternion.

       Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
     */
    template <class ValueType>
    class TQuaternion
    {
      static_assert(std::is_floating_point<ValueType>::value, "");

    public:
      ValueType s, vx, vy, vz;

      /*! \brief Returns the additive identity for quaternions (which is all
         zeros)
       */
      static DDImage_API TQuaternion add_identity();

      /*! \brief Returns the multipicative identity for quaternions (which is
         1,0,0,0)
       */
      static DDImage_API TQuaternion mult_identity();
      /*! \brief Default constructor.
       */

      TQuaternion () :
        s(1), vx(0), vy(0), vz(0)
      {}

      /*! \brief Constructor for specifying values.
       */
      TQuaternion (ValueType a, ValueType b, ValueType c, ValueType d) :
        s(a), vx(b), vy(c), vz(d)
      {}

      /*! \brief Set individual quaternion values.
       */
      void set(ValueType a, ValueType b, ValueType c, ValueType d)
      {
        s = a;
        vx = b;
        vy = c;
        vz = d;
      }

      /*! \brief This constructor takes an angle in radians and a vector to
         rotate around.
       */
      TQuaternion ( ValueType sval, const Vector3& v ) {
        s = std::cos ( sval / 2.0 );

        ValueType sang = std::sin ( sval / 2.0 );
        vx = sang * v.x;
        vy = sang * v.y;
        vz = sang * v.z;
      }

      /*! \brief Constructor. Given a Matrix that represents a rotation,
         calculate the quaternion that is equivalent to that rotation.
       */
      /*! Given a Matrix that represents a rotation, calculate
         the quaternion that is equivalent to that rotation.
         A matrix3 could be used instead, last row and column are ignored.
       */
      DDImage_API TQuaternion ( const Matrix4& mat );

      /*! \brief Constructor from two vectors. The quaternion will represent
         the angle between the two vectors.
       */
      /*! Constructor from two vectors. The quaternion will represent the
         angle between the two vectors.
       */
      DDImage_API TQuaternion ( const Vector3& org_vec, const Vector3& new_vec );


      /*! \brief Addition of two quaternions. This follows this rule:

         <tt><b> q1 + q2 </b> = ( s1 + s2, vx1 + vx2, vy1 + vy2, vz1 + vz2 )</tt>
       */
      TQuaternion operator+ ( const TQuaternion& q ) const
      {
        TQuaternion ret_val;

        ret_val.s  = s + q.s;
        ret_val.vx = vx + q.vx;
        ret_val.vy = vy + q.vy;
        ret_val.vz = vz + q.vz;

        return ret_val;
      }

      /*! \brief Multiplication of two quaternions. This follows this rule:

         <pre><b>q1 q2</b> = ( s1 s2 - vx1 vx2 - vy1 vy2 - vz1 vz2,
         vy1 vz2 - vy2 vz1 + s1 vx2 + s2 vx1,
         vz1 vx2 - vz2 vx1 + s1 vy2 + s2 vy1,
         vx1 vy2 - vx2 vy1 + s1 vz2 + s2 vz1 ))</pre>

         The result is a quaternion representing the rotation specifed by this quaternion followed by that of \a q.
       */
      TQuaternion operator* ( const TQuaternion& q ) const
      {
        TQuaternion ret_val;
        ret_val.s = s * q.s - ( vx * q.vx + vy * q.vy + vz * q.vz );

        ret_val.vx = vy * q.vz - q.vy * vz + s * q.vx + q.s * vx;
        ret_val.vy = vz * q.vx - q.vz * vx + s * q.vy + q.s * vy;
        ret_val.vz = vx * q.vy - q.vx * vy + s * q.vz + q.s * vz;
        return ret_val;
      }

      /*! \brief Multiplication of a quaternion by a double number. This
         follows this rule:

         <tt><b> f * q </b> = ( f * s, f * vx, f * vy, f * vz )</tt>
       */
      TQuaternion operator* ( ValueType f ) const
      {
        return TQuaternion ( f * s, f * vx, f * vy, f * vz );
      }

      /*! \brief Returns the conjugate of this quaternion. This follows this
         rule:  <tt><b> q.conjugate </b> = ( s - <b> v </b> )</tt>
       */
      TQuaternion conjugate () const
      {
        return TQuaternion ( s, -vx, -vy, -vz );
      }

      /*! \brief Returns the square of the length of the quaternion.
         This follows this rule:

         <tt><b> q.lengthSquared </b> = <b> s^2 + vx^2 + vy^2 + vz^2 </b> </tt>
       */
      ValueType length_squared() const
      {
        return s*s + vx*vx + vy*vy + vz*vz;
      }

      /*! \brief Returns the length of the quaternion.
         This follows this rule:

         <tt><b> q.length </b> = <b> sqrt(s^2 + vx^2 + vy^2 + vz^2 </b>) </tt>
       */
      ValueType length() const
      {
        return std::sqrt(s*s + vx*vx + vy*vy + vz*vz);
      }

      /*! \brief Returns the square of the magnitude of the quaternion.
         To avoid confusion, pleasse use length()/length_squared() instead.
         This follows this rule:

         <tt><b> q.magnitude </b> = <b> q q.conjugate </b>
         = s^2 + vx^2 + vy^2 + vz^2 </tt>
       */
      ValueType magnitude() const
      {
        TQuaternion mult = *this * this->conjugate();
        return mult.s + mult.vx + mult.vy + mult.vz;
      }

      /*! \brief Returns the additive inverse of the quaternion. This is:
         <tt><b> q.add_inverse </b> = ( -s, -vx, -vy, -vz ) </tt>
       */
      TQuaternion add_inverse () const
      {
        return TQuaternion ( -s, -vx, -vy, -vz );
      }

      /*! \brief Returns the multiplicative inverse of the quaternion. This
         is:
         <tt><b> q.mult_inverse </b> =
         ( 1 / <b>q.magnitude</b> ) * <b> q.conjugate </b> </tt>
       */
      TQuaternion mult_inverse () const
      {
        return conjugate() * static_cast<ValueType>(1.0 / length());
      }

      /*! \brief Spherical linear interpolation.
       */
      /*! This method interpolates
         smoothly between two quaternions. The value t should be a
         number between 0.0 and 1.0. When t = 0.0, *this is
         returned. When t = 1.0, end_quat is returned.

         Because of the way quaternions
         work, you can't just linearly interpolate between two of
         them. You must interpolate along the surface of a sphere. This
         method returns a quaternion that is between the current
         quaternion and the end_quat. The value of t (which should be
         between 0 and 1) determines the amount of interpolation.
       */
      DDImage_API TQuaternion slerp ( const TQuaternion& end_quat, ValueType t ) const;

      /*! \brief Return the transformation matrix that will represent the
         the Euler angle rotations that this quaternion embodies. Note - this
         method affects all components of the matrix
       */
      /*! Return the transformation matrix that will represent the
         the Euler angle rotations that this quaternion embodies.
       */
      DDImage_API Matrix4 matrix () const;

      /*! \brief Return the transformation matrix that will represent the
         the Euler angle rotations that this quaternion embodies. Note - this
         method only affects the rotation part of the matrix. NOTE: The quaternion must
         be normalised before using this function
       */
      DDImage_API Matrix4 matrix2 () const;

      /* \brief Normalize the quaternion to unit length
      */
      DDImage_API void normalize();

      //! Writes it in nuke/tcl notation "{s x y z}"
      template <typename T>
      friend DDImage_API std::ostream&  operator<< (std::ostream& o, const TQuaternion<T>& q );

    };

    using Quaternion = TQuaternion<double>;
    using Quaternion4d = TQuaternion<double>;
    using Quaternion4f = TQuaternion<float>;

  } // namespace Image
} // namespace DD

#endif
