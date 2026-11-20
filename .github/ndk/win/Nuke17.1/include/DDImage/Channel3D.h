// Channel3D.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

// Defines channel numbers used by 3D, in the arrays stored for
// vertices in a GeoInfo, and in the VArray used by the fragment_shader.
// This should be merged with the normal image channels.

#ifndef DDImage_Channel3D_h
#define DDImage_Channel3D_h

#include "DDImage/Channel.h"

namespace DD
{
  namespace Image
  {

    enum {
      /* Defined in Channel.h:
         Chan_Black  =  0,
         Chan_Red    =  1,
         Chan_Green  =  2,
         Chan_Blue   =  3,
         Chan_Alpha  =  4,
       */
      Chan_Px     =  5,
      Chan_Py     =  6,
      Chan_Pz     =  7,
      Chan_Pw     =  8,
      Chan_UVx    =  9,
      Chan_UVy    = 10,
      Chan_UVz    = 11,
      Chan_UVw    = 12,
      Chan_Nx     = 13,
      Chan_Ny     = 14,
      Chan_Nz     = 15,
      Chan_PLx    = 16,
      Chan_PLy    = 17,
      Chan_PLz    = 18,
      Chan_PWx    = 19,
      Chan_PWy    = 20,
      Chan_PWz    = 21,
      Chan_MBx    = 22,
      Chan_MBy    = 23,
      Chan_MBz    = 24,
      Chan_VELx   = 25,
      Chan_VELy   = 26,
      Chan_VELz   = 27,
      //
      Chan_P_     = Chan_Px,
      Chan_UV_    = Chan_UVx,
      Chan_N_     = Chan_Nx,
      Chan_PL_    = Chan_PLx,
      Chan_PW_    = Chan_PWx,
      Chan_MB_    = Chan_MBx,
      Chan_VEL_   = Chan_VELx,
    };

    enum ChannelSet3DInit {
      /* Defined in Channel.h:
         Mask_Black  = 0,                  //!< 0
         Mask_Red    = 1<<(Chan_Red  -1),  //!< Cf red
         Mask_Green  = 1<<(Chan_Green-1),  //!< Cf green
         Mask_Blue   = 1<<(Chan_Blue -1),  //!< Cf blue
         Mask_Alpha  = 1<<(Chan_Alpha-1),  //!< Cf alpha
       */
      Mask_Px     = 1 << (Chan_Px  - 1), //!< Point x
      Mask_Py     = 1 << (Chan_Py  - 1), //!< Point y
      Mask_Pz     = 1 << (Chan_Pz  - 1), //!< Point z
      Mask_Pw     = 1 << (Chan_Pw  - 1), //!< Point w
      Mask_UVx    = 1 << (Chan_UVx - 1), //!< UV x
      Mask_UVy    = 1 << (Chan_UVy - 1), //!< UV y
      Mask_UVz    = 1 << (Chan_UVz - 1), //!< UV z
      Mask_UVw    = 1 << (Chan_UVw - 1), //!< UV w
      Mask_Nx     = 1 << (Chan_Nx  - 1), //!< Normal x
      Mask_Ny     = 1 << (Chan_Ny  - 1), //!< Normal y
      Mask_Nz     = 1 << (Chan_Nz  - 1), //!< Normal z
      Mask_PLx    = 1 << (Chan_PLx - 1), //!< Local-space point x
      Mask_PLy    = 1 << (Chan_PLy - 1), //!< Local-space point y
      Mask_PLz    = 1 << (Chan_PLz - 1), //!< Local-space point z
      Mask_PWx    = 1 << (Chan_PWx - 1), //!< World-space point x
      Mask_PWy    = 1 << (Chan_PWy - 1), //!< World-space point y
      Mask_PWz    = 1 << (Chan_PWz - 1), //!< World-space point z
      Mask_MBx    = 1 << (Chan_MBx - 1), //!< Motion-blur x
      Mask_MBy    = 1 << (Chan_MBy - 1), //!< Motion-blur y
      Mask_MBz    = 1 << (Chan_MBz - 1), //!< Motion-blur z
      Mask_VELx   = 1 << (Chan_VELx - 1), //!< Point velocity vector x
      Mask_VELy   = 1 << (Chan_VELy - 1), //!< Point velocity vector y
      Mask_VELz   = 1 << (Chan_VELz - 1), //!< Point velocity vector z
      Mask_CF_    = unsigned(Mask_Red)  | Mask_Green | Mask_Blue | Mask_Alpha,
      Mask_P_     = Mask_Px   | Mask_Py    | Mask_Pz   | Mask_Pw,
      Mask_UV_    = Mask_UVx  | Mask_UVy   | Mask_UVz  | Mask_UVw,
      Mask_N_     = Mask_Nx   | Mask_Ny    | Mask_Nz,
      Mask_PL_    = Mask_PLx  | Mask_PLy   | Mask_PLz,
      Mask_PW_    = Mask_PWx  | Mask_PWy   | Mask_PWz,
      Mask_MB_    = Mask_MBx  | Mask_MBy   | Mask_MBz,
      Mask_VEL_   = Mask_VELx | Mask_VELy  | Mask_VELz,
    };

  }
}

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
// end of Channel3D.h
