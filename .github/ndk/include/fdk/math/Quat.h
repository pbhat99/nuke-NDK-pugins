// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FDK_MATH_QUAT_H
#define FDK_MATH_QUAT_H

#include "fdk/math/Math.h"
#include "fdk/math/Vec3.h"

#include <cmath>
#include <limits>
#include <type_traits>


namespace fdk {
  class Hash;
  template <typename T> class Mat4;

  //! \class fdk::Quat
  //!
  //! An implementation of a way of representing rotations that avoid a lot
  //! of the problems that the standard rotation about the axis methods have.
  //!
  //! Quaternions are a modification of the concept of a vector in space,
  //! but specially tailored for spherical space. The cool thing about
  //! quaternions is that they are perfectly suited to representing
  //! rotations and orientations of objects in three space.
  //!
  //! Basically, in a quaternion there are four values: a scalar part and
  //! a vector part. <b>q</b> = ( s, <b>v</b> ). Typically, when dealing
  //! with rotations, the scalar part represents the rotation about an
  //! arbitrary axis. The axis is represented by a unit vector in the
  //! vector part.
  //!
  //! Since the quaternion is a representation of a rotation, it can be
  //! converted into a Euler angle rotation matrix and a rotation matrix
  //! can be converted into a quaternion.
  template <class T>
  class Quat
  {
  public:
    using value_type = T;
    using reference = T&;
    using const_reference = const T&;

    static constexpr uint8_t kNumElements = 4; //< The number of elements in the quat

    static constexpr uint8_t X = 0; //< Constant index for the X axis
    static constexpr uint8_t Y = 1; //< Constant index for the Y axis
    static constexpr uint8_t Z = 2; //< Constant index for the Z axis
    static constexpr uint8_t S = 3; //< Constant index for the scalar part


    // These are public for ease of access.
    Vec3<T> v;  //!< the vector part
    T       s;  //!< the scalar part

    //! \brief Return the additive identity for quaternions (which is all
    //! zeros)
    static Quat addIdentity();

    //! \brief Return the multiplicative identity for quaternions (which is
    //! 1,0,0,0)
    static Quat multIdentity();

    //! \brief Build a quaternion from an angle in radians and a vector to
    //! rotate around.
    static Quat fromAngleAxis(T radians, const Vec3<T>& vec);


    //! \brief Default constructor leaves garbage in contents.
    Quat() = default;

    //! \brief Construct from a scalar and vector defaulted to 0.
    explicit Quat(T scalar) : v(0), s(scalar) {}

    //! \brief Constructor for specifying values.
    Quat(T scalar, T x, T y, T z) : v(x, y, z), s(scalar) {}

    //! \brief Constructor for specifying values.
    template <typename R>
    Quat(T scalar, const Vec3<R>& vec);

    //! \brief Copy constructor supports type conversion.
    template <typename R>
    Quat(const Quat<R>& q);

    //! \brief Constructor. Given a Matrix that represents a rotation,
    //! calculate the quaternion that is equivalent to that rotation.
    //! Given a matrix that represents a rotation, calculate the
    //! quaternion that is equivalent to that rotation. The last row
    //! and column of the Mat4 are ignored.
    Quat(const Mat4<T>& mat);

    //! \brief Constructor from two vectors. The quaternion will represent
    //! the angle between the two vectors.
    //! Constructor from two vectors. The quaternion will represent the
    //! angle between the two vectors.
    Quat(const Vec3<T>& originalVector, const Vec3<T>& newVector);


    //! Returns value as a specific type:
    operator Quat<fdk::half>() const;
    operator Quat<float>()     const;
    operator Quat<double>()    const;

    Quat<fdk::half> asQuath() const;
    Quat<float>     asQuatf() const;
    Quat<double>    asQuatd() const;


    //! Returns a pointer to the first element.
    T*        array()       { return &v.x; }
    const T*  array() const { return &v.x; }


    //! \brief Set individual quaternion values.
    void set(T scalar, T x, T y, T z);

    //! \brief Set individual quaternion values.
    void set(T scalar, const Vec3<T>& vec);

    //! \brief Set to an angle in radians and a vector to rotate around.
    void setFromAngleAxis(T radians, const Vec3<T>& vec);


    //! \brief Addition of two quaternions. This follows this rule:
    //! <tt><b> q1 + q2 </b> = ( s1 + s2, vx1 + vx2, vy1 + vy2, vz1 + vz2 )</tt>
    Quat operator+(const Quat& q) const;

    //! \brief Multiplication of two quaternions. This follows this rule:
    //!
    //! <pre><b>q1 q2</b> = ( s1 s2 - vx1 vx2 - vy1 vy2 - vz1 vz2,
    //! vy1 vz2 - vy2 vz1 + s1 vx2 + s2 vx1,
    //! vz1 vx2 - vz2 vx1 + s1 vy2 + s2 vy1,
    //! vx1 vy2 - vx2 vy1 + s1 vz2 + s2 vz1 ))</pre>
    //!
    //! (I think this is the same as doing the two rotations one after
    //! another?)
    Quat operator*(const Quat& q) const;

    //! \brief Multiplication of a quaternion by a double number. This
    //! follows this rule:
    //!
    //! <tt><b> f * q </b> = ( f * s, f * v.x, f * v.y, f * v.z )</tt>
    Quat operator*(T f) const;


    //! \brief Returns the conjugate of this quaternion. This follows this
    //! rule:  <tt><b> q.conjugate </b> = ( s - <b> v </b> )</tt>
    Quat conjugate() const;

    //! \brief Returns the square of the length of the quaternion.
    //! This follows this rule:
    //!
    //! <tt><b> q.lengthSquared </b> = <b> s^2 + v.x^2 + v.y^2 + v.z^2 </b> </tt>
    T lengthSquared() const;

    //! \brief Returns the length of the quaternion.
    //! This follows this rule:
    //!
    //! <tt><b> q.length </b> = <b> sqrt(s^2 + v.x^2 + v.y^2 + v.z^2 </b>) </tt>
    T length() const;

    //! \brief Returns the square of the magnitude of the quaternion.
    //! To avoid confusion, use length()/lengthSquared() instead.
    //! This follows this rule:
    //!
    //! <tt><b> q.magnitude </b> = <b> q q.conjugate </b>
    //! = s^2 + v.x^2 + v.y^2 + v.z^2 </tt>
    T magnitude() const;

    //! \brief Normalize the quaternion to unit length
    void normalize();

    //! \brief Return a normalized version of this quaternion
    Quat normalized();

    //! \brief Returns the additive inverse of the quaternion. This is:
    //! <tt><b> q.addInverse </b> = ( -s, -v.x, -v.y, -v.z ) </tt>
    Quat addInverse() const;

    //! \brief Returns the multiplicative inverse of the quaternion. This
    //! is:
    //! <tt><b> q.multInverse </b> =
    //! ( 1 / <b>q.magnitude</b> ) * <b> q.conjugate </b> </tt>
    Quat multInverse() const;

    //! \brief Spherical linear interpolation.
    //! This method interpolates smoothly between two quaternions.
    //! The value t should be a number between 0.0 and 1.0. When t = 0.0,
    //! *this is returned. When t = 1.0, end_quat is returned.
    //!
    //! Because of the way quaternions
    //! work, you can't just linearly interpolate between two of
    //! them. You must interpolate along the surface of a sphere. This
    //! method returns a quaternion that is between the current
    //! quaternion and the end_quat. The value of t (which should be
    //! between 0 and 1) determines the amount of interpolation.
    Quat slerp(const Quat& end_quat, T t) const;


    //! \brief Return the transformation matrix that will represent the
    //! the Euler angle rotations that this quaternion embodies. Note - this
    //! method replaces all components of the matrix.
    //! Return the transformation matrix that will represent the the Euler
    //! angle rotations that this quaternion embodies.
    Mat4<T> matrix() const;

    //! \brief Return the transformation matrix that will represent the
    //! the Euler angle rotations that this quaternion embodies. Note - this
    //! method only affects the rotation part of the matrix. NOTE: The quaternion must
    //! be normalized before using this function
    Mat4<T> rotationMatrix() const;


    //! Add this to a fdk::Hash object.
    void append(fdk::Hash& hash) const;

  private:
    static_assert(fdk::is_floating_point<T>, "Quat can only be instantiated with a floating point type.");
  };


  /*--------------------------------------*/
  /*    Aliases for supported data types  */
  /*--------------------------------------*/
  using Quath = Quat<fdk::half>;
  using Quatf = Quat<float>;
  using Quatd = Quat<double>;


  template<typename T>
  template<typename R>
  inline Quat<T>::Quat(const Quat<R>& q)
  {
    v.x = static_cast<T>(q.v.x);
    v.y = static_cast<T>(q.v.y);
    v.z = static_cast<T>(q.v.z);
    s   = static_cast<T>(q.s);
  }

  template<typename T>
  template<typename R>
  inline Quat<T>::Quat(T scalar, const Vec3<R>& vec)
  {
    s = scalar;
    v.x = static_cast<T>(vec.x);
    v.y = static_cast<T>(vec.y);
    v.z = static_cast<T>(vec.z);
  }

  // Return the additive identity for quaternions (which is all zeros)
  // static
  template<typename T>
  inline Quat<T> Quat<T>::addIdentity()
  {
    return Quat<T>(0.0, 0.0, 0.0, 0.0);
  }

  // Return the multiplicative identity for quaternions (which is 1,0,0,0)
  // static
  template<typename T>
  inline Quat<T> Quat<T>::multIdentity()
  {
    return Quat<T>(1.0, 0.0, 0.0, 0.0);
  }

  // Build a quaternion from an angle in radians and a vector to rotate around.
  // static
  template<typename T>
  inline Quat<T> Quat<T>::fromAngleAxis(T radians, const Vec3<T>& vec)
  {
    const auto a = radians / static_cast<T>(2.0);
    return Quat<T>(std::cos(a), vec * std::sin(a));
  }

  template<typename T>
  inline void Quat<T>::set(T scalar, T x, T y, T z)
  {
    s = scalar;
    v.set(x, y, z);
  }

  template<typename T>
  inline void Quat<T>::set(T scalar, const Vec3<T>& vec)
  {
    s = scalar;
    v = vec;
  }

  template<typename T>
  inline void Quat<T>::setFromAngleAxis(T radians, const Vec3<T>& vec)
  {
    *this = fromAngleAxis(radians, vec);
  }

  //-----------------------------------------------------------

  template<typename T>
  inline Quat<fdk::half> Quat<T>::asQuath() const
  {
    if (std::is_same<fdk::half, T>::value) {
      return *this;
    }
    else {
      return Quat<fdk::half>(fdk::half(static_cast<float>(s)),
                             fdk::half(static_cast<float>(v.x)),
                             fdk::half(static_cast<float>(v.y)),
                             fdk::half(static_cast<float>(v.z)));
    }
  }
  template<typename T>
  inline Quat<T>::operator Quat<fdk::half>() const { return asQuath(); }

  template<typename T>
  inline Quat<float> Quat<T>::asQuatf() const
  {
    if (std::is_same<float, T>::value) {
      return *this;
    }
    else {
      return Quat<float>(static_cast<float>(s),
                         static_cast<float>(v.x),
                         static_cast<float>(v.y),
                         static_cast<float>(v.z));
    }
  }

  template<typename T>
  inline Quat<T>::operator Quat<float>() const { return asQuatf(); }

  template<typename T>
  inline Quat<double> Quat<T>::asQuatd() const
  {
    if (std::is_same<double, T>::value) {
      return *this;
    }
    else {
      return Quat<double>(static_cast<double>(s),
                          static_cast<double>(v.x),
                          static_cast<double>(v.y),
                          static_cast<double>(v.z));
    }
  }

  template<typename T>
  inline Quat<T>::operator Quat<double>() const { return asQuatd(); }

  //-----------------------------------------------------------

  template<typename T>
  inline Quat<T> Quat<T>::operator+(const Quat<T>& q) const
  {
    return Quat<T>(s + q.s, v + q.v);
  }

  template<typename T>
  inline Quat<T> Quat<T>::operator*(const Quat<T>& q) const
  {
    Quat<T> ret_val;

    ret_val.s = s * q.s - (v.x * q.v.x + v.y * q.v.y + v.z * q.v.z);

    ret_val.v.x = v.y * q.v.z - q.v.y * v.z + s * q.v.x + q.s * v.x;
    ret_val.v.y = v.z * q.v.x - q.v.z * v.x + s * q.v.y + q.s * v.y;
    ret_val.v.z = v.x * q.v.y - q.v.x * v.y + s * q.v.z + q.s * v.z;

    return ret_val;
  }

  template<typename T>
  inline Quat<T> Quat<T>::operator*(T f) const
  {
    return Quat<T>(s * f, v * f);
  }

  template<typename T>
  inline Quat<T> Quat<T>::conjugate() const
  {
    return Quat<T>(s, -v);
  }

  template<typename T>
  inline T Quat<T>::lengthSquared() const
  {
    return s * s + v.lengthSquared();
  }

  template<typename T>
  inline T Quat<T>::length() const
  {
    return std::sqrt(lengthSquared());
  }

  template<typename T>
  inline T Quat<T>::magnitude() const
  {
    Quat<T> mult = *this * conjugate();
    return mult.s + mult.v.x + mult.v.y + mult.v.z;
  }

  template<typename T>
  inline Quat<T> Quat<T>::addInverse() const
  {
    return Quat<T>(-s, -v);
  }

  template<typename T>
  inline Quat<T> Quat<T>::multInverse() const
  {
    return conjugate() * (static_cast<T>(1) / length());
  }

  template<typename T>
  inline void Quat<T>::normalize()
  {
    const T n = static_cast<T>(1) / length();
    v *= n;
    s *= n;
  }

  template<typename T>
  inline Quat<T> Quat<T>::normalized()
  {
    const T n = static_cast<T>(1) / length();
    return Quat<T>(s*n, v*n);
  }

  template<typename T>
  inline std::ostream& operator<<(std::ostream& o, const Quat<T>& q)
  {
    o << '{' << q.v.x << ' ' << q.v.y << ' ' << q.v.z << ' ' << q.s << '}';
    return o;
  }

  template<typename T>
  inline bool operator==(const Quat<T>& lhs, const Quat<T>& rhs)
  {
    return lhs.s == rhs.s && lhs.v == rhs.v;
  }

  template<typename T>
  inline bool operator!=(const Quat<T>& lhs, const Quat<T>& rhs)
  {
    return !(lhs == rhs);
  }

  extern template class FDK_API Quat<fdk::half>;
  extern template class FDK_API Quat<float>;
  extern template class FDK_API Quat<double>;

}

#endif
