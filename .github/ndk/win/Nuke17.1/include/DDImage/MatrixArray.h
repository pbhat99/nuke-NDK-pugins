// MatrixArray.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_MatrixArray_h
#define DDImage_MatrixArray_h

#include "DDImage_API.h"
#include "Matrix4.h"

/* MatrixArray Map:
          -----------------|
      4 S |*20*21*22*23 xx |
          -----------------|
    F 3 C |*15*16*17 xx 19 |
    R     -----------------|
    O 2 E |*10*11 xx 13 14 |
    M     -----------------|
      1 W | *5 xx  7  8  9 |
          -----------------|
      0 L | xx  1  2  3  4 |
          -----------------|
             L  W  E  C  S
             0  1  2  3  4
                  TO
   xx = identity matrix
 * = inverse of the transpose

   TRANSFORMATION ORDER:
   ---------------------
   (L)ocal space  = Untransformed

    apply OBJECT_MATRIX to get:

   (W)orld_space  = Parent object transform applied

    apply CAMERA_MATRIX to get:

   (E)ye space    = Inverse camera transform applied(camera becomes 0,0,0)
                    TODO: in the CAMERA_MATRIX case, its inverse(#11) is
                    actually a CameraOp's "positive" matrix.  We should
                    handle this specially as we end up calculating the
                    inversion twice - once in the CameraOp object, and
                    again in this MatrixArray...

    apply PROJECTION_MATRIX to get:

   (C)lip space   = Camera projection applied, it's homogeneous-space once
                    W is divided out of a point.

    apply FORMAT_MATRIX to get:

   (S)creen space = Homogeneous-space scaled to pixel coordinates

 */

namespace DD
{
  namespace Image
  {

    /* Define a 5x5 array: */
#define TRANSFORM_NUM 5

    /*! Transform-space enumerations for shaders.  These numbers are used
       by the MatrixArray class to index the in/out space parameters. */
    enum {
      LOCAL_SPACE  = 0, //!< Point in locally defined space
      WORLD_SPACE  = 1, //!< Object transform applied
      EYE_SPACE    = 2, //!< Inverse camera transform applied(camera becomes 0,0,0)
      CLIP_SPACE   = 3, //!< Camera projection applied, homogeneous-space
      SCREEN_SPACE = 4 //!< Homogeneous-space scaled to pixel coordinates
    };

    /* Important matrix indices in a MatrixArray. */
    enum {
      OBJECT_MATRIX      =  1,
      OBJECT_iMATRIX     =  5,
      CAMERA_MATRIX      =  7,
      CAMERA_iMATRIX     = 11,
      PROJECTION_MATRIX  = 13,
      PROJECTION_iMATRIX = 17,
      FORMAT_MATRIX      = 19,
      FORMAT_iMATRIX     = 23
    };

    /*! An array of Matrix4 transformation matrices organized
       into a square matrix for constructing fast from/to transform-space
       conversions.  This structure keeps track of the dependencies
       between the transform matrices so that if one changes which
       others depend on, the dependent matrices are automatically
       recalculated when they are next accessed.  This mechanism reduces
       the overhead required in keeping all the transforms up-to-date,
       specifically keeping the number of expensive matrix inversions
       at a minimum.
     */
    class DDImage_API MatrixArray
    {
      /*! A 4x4 transform matrix and its housekeeping routines. */
      struct MatrixEntry
      {
        Matrix4 matrix; //!< The 4x4 matrix
        bool inverse, valid; //!< State flags
        /* Management methods: */
        void invalidate() { inverse = false;
                            valid = false; }
        void invalidate_inverse() { inverse = true;
                                    valid = false; }
        void set(const Matrix4& m) { matrix = m;
                                     inverse = false;
                                     valid = true; }
        void set_inverse(const Matrix4& m) { matrix = m;
                                             inverse = true;
                                             valid = true; }
      }
      entry[TRANSFORM_NUM* TRANSFORM_NUM];

      int index(int from, int to) const { return from * TRANSFORM_NUM + to; }
      int from(int n) const { return n / TRANSFORM_NUM; }
      int to(int n) const { return n % TRANSFORM_NUM; }
      int transpose(int n) const { return index(to(n), from(n)); }

    public:

      /* Constructor invalidates all matrices and assigns the identity to
          null transforms along the diagonal.
       */
      MatrixArray();

      /* Copy constructor.
       */
      MatrixArray(const MatrixArray &);

      /* Constructor invalidates all matrices and assigns the four base
          matrices.
       */
      MatrixArray(const Matrix4 &object, const Matrix4 &camera,
                  const Matrix4 &projection, const Matrix4 &format);

      int num_matrices() const { return TRANSFORM_NUM * TRANSFORM_NUM; }

      const Matrix4& object_matrix() const { return entry[OBJECT_MATRIX].matrix; }

      /*! Update the transform matrix array with a new object matrix.
          The matrices that are dependant upon it are marked invalid.
       */
      void set_object_matrix(const Matrix4& m);

      const Matrix4& camera_matrix() const { return entry[CAMERA_MATRIX].matrix; }

      /*! Update the transform matrix array with a new camera matrix.
          The matrices that are dependant upon it are marked invalid.
       */
      void set_camera_matrix(const Matrix4& m);

      const Matrix4& projection_matrix() const { return entry[PROJECTION_MATRIX].matrix; }

      /*! Update the transform matrix array with a new projection matrix.
          The matrices that are dependant upon it are marked invalid.
       */
      void set_projection_matrix(const Matrix4& m);

      /*! Update the transform matrix array with a new format matrix.
          The matrices that are dependant upon it are marked invalid.
       */
      void set_format_matrix(const Matrix4& m);
      const Matrix4& format_matrix() const { return entry[FORMAT_MATRIX].matrix; }

      /*! Return the 4x4 matrix at entry 'n'.
          Check if it's been calculated yet(validated.)  If not,
          call matrix() on the dependency matrix until we can return a
          result (a dependency matrix may in turn have a dependency it must
          also resolve, so this can be a recursive call.)
       */
      const Matrix4& matrix(int n);
      const Matrix4& matrix(int from, int to) { return matrix(index(from, to)); }
      const Matrix4& inverse_matrix(int n) { return matrix(transpose(n)); }
      const Matrix4& inverse_matrix(int from, int to) { return matrix(index(to, from)); }

      const Matrix4& operator [] (int n) { return matrix(n); }

      /*! Reset all matrices to their identity.
       */
      void reset();

      /*! Calculate all invalid matrices for testing purposes.
          Only use this for testing as it will calculate all the inverses!
       */
      void calculate_all_now();

      void print_info(const char* title);
    };

  }
}

    /* Predefined matrix addresses:
                         FROM_SPACE                     TO_SPACE   */
#define LOCAL_TO_WORLD   DD::Image::LOCAL_SPACE  * TRANSFORM_NUM + DD::Image::WORLD_SPACE
#define LOCAL_TO_EYE     DD::Image::LOCAL_SPACE  * TRANSFORM_NUM + DD::Image::EYE_SPACE
#define LOCAL_TO_CLIP    DD::Image::LOCAL_SPACE  * TRANSFORM_NUM + DD::Image::CLIP_SPACE
#define LOCAL_TO_SCREEN  DD::Image::LOCAL_SPACE  * TRANSFORM_NUM + DD::Image::SCREEN_SPACE

#define WORLD_TO_LOCAL   DD::Image::WORLD_SPACE  * TRANSFORM_NUM + DD::Image::LOCAL_SPACE
#define WORLD_TO_EYE     DD::Image::WORLD_SPACE  * TRANSFORM_NUM + DD::Image::EYE_SPACE
#define WORLD_TO_CLIP    DD::Image::WORLD_SPACE  * TRANSFORM_NUM + DD::Image::CLIP_SPACE
#define WORLD_TO_SCREEN  DD::Image::WORLD_SPACE  * TRANSFORM_NUM + DD::Image::SCREEN_SPACE

#define EYE_TO_LOCAL     DD::Image::EYE_SPACE    * TRANSFORM_NUM + DD::Image::LOCAL_SPACE
#define EYE_TO_WORLD     DD::Image::EYE_SPACE    * TRANSFORM_NUM + DD::Image::WORLD_SPACE
#define EYE_TO_CLIP      DD::Image::EYE_SPACE    * TRANSFORM_NUM + DD::Image::CLIP_SPACE
#define EYE_TO_SCREEN    DD::Image::EYE_SPACE    * TRANSFORM_NUM + DD::Image::SCREEN_SPACE

#define CLIP_TO_LOCAL    DD::Image::CLIP_SPACE   * TRANSFORM_NUM + DD::Image::LOCAL_SPACE
#define CLIP_TO_WORLD    DD::Image::CLIP_SPACE   * TRANSFORM_NUM + DD::Image::WORLD_SPACE
#define CLIP_TO_EYE      DD::Image::CLIP_SPACE   * TRANSFORM_NUM + DD::Image::EYE_SPACE
#define CLIP_TO_SCREEN   DD::Image::CLIP_SPACE   * TRANSFORM_NUM + DD::Image::SCREEN_SPACE

#define SCREEN_TO_LOCAL  DD::Image::SCREEN_SPACE * TRANSFORM_NUM + DD::Image::LOCAL_SPACE
#define SCREEN_TO_WORLD  DD::Image::SCREEN_SPACE * TRANSFORM_NUM + DD::Image::WORLD_SPACE
#define SCREEN_TO_EYE    DD::Image::SCREEN_SPACE * TRANSFORM_NUM + DD::Image::EYE_SPACE
#define SCREEN_TO_CLIP   DD::Image::SCREEN_SPACE * TRANSFORM_NUM + DD::Image::CLIP_SPACE

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
