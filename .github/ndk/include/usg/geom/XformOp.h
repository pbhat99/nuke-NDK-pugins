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
    /// Equivalent to USD's \c UsdGeomXformOp::Type (values and order may not match).
    enum class Type {
      Invalid,
      TranslateX,  // Since USD 25.02
      TranslateY,  // Since USD 25.02
      TranslateZ,  // Since USD 25.02
      Translate,
      ScaleX,  // Since USD 25.02
      ScaleY,  // Since USD 25.02
      ScaleZ,  // Since USD 25.02
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

      ResetXformStack,  // Unique to UPE
    };

    static Token getOpTypeToken(const XformOp::Type& type);
    static XformOp::Type getOpTypeEnum(const Token& type);

    static Value::Type
    opTypeToValueType(const XformOp::Type& opType)
    {
      switch (opType) {
        case Type::Translate:
        case Type::Scale:
          return Value::Double3;
        case Type::TranslateX:
        case Type::TranslateY:
        case Type::TranslateZ:
        case Type::ScaleX:
        case Type::ScaleY:
        case Type::ScaleZ:
        case Type::RotateX:
        case Type::RotateY:
        case Type::RotateZ:
          return Value::Double;
        case Type::RotateXYZ:
        case Type::RotateXZY:
        case Type::RotateYXZ:
        case Type::RotateYZX:
        case Type::RotateZXY:
        case Type::RotateZYX:
          return Value::Double3;
        case Type::Orient:
          return Value::Quatd;
        case Type::Transform:
          return Value::Matrix4d;
        default:
          return Value::InvalidType;
      }
    }
  };

  extern USG_API XformOpTypesDef xformOpTypes;

};

#endif
