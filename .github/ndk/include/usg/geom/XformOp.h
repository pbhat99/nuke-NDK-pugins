// Copyright (c) 2024 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef USG_GEOM_XFORMOP_H
#define USG_GEOM_XFORMOP_H

#include "usg/base/Token.h"
#include "usg/base/Value.h"

namespace usg {

  struct USG_API XformOpTypesDef {
    Token::Lazy resetXformStack{ "!resetXformStack!" };
  };

  class USG_API XformOp
  {
    public:
    enum Type {
      // Keep this order as same as USD's UsdGeomXformOp::Type
      Invalid,
      Translate,
      Scale,
      RotateX,
      RotateY,
      RotateZ,
      RotateXYZ,
      RotateXZY,
      RotateYXZ,
      RotateYZX,
      RotateZXY,
      RotateZYX,
      Orient,
      Transform,
      ResetXformStack
    };

    static Token getOpTypeToken(const XformOp::Type& type);
    static Value::Type
    opTypeToValueType(const XformOp::Type& opType)
    {
      switch (opType) {
        case Translate:
        case Scale:
          return Value::Double3;
        case RotateX:
        case RotateY:
        case RotateZ:
          return Value::Double;
        case RotateXYZ:
        case RotateXZY:
        case RotateYXZ:
        case RotateYZX:
        case RotateZXY:
        case RotateZYX:
          return Value::Double3;
        case Orient:
          return Value::Quatd;
        case Transform:
          return Value::Matrix4d;
        default:
          return Value::InvalidType;
      }
    }
  };

  extern USG_API XformOpTypesDef xformOpTypes;

};

#endif
