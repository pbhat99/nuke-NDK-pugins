// ElementI.h
// Copyright (c) 2012 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_ElementI_h
#define DDImage_ElementI_h

#include "DDImage/CurveI.h"
#include "DDImage/DDImage_API.h"
#include "DDImage/RefCountedObject.h"

namespace DD
{
  namespace Image
  {
    class OutputContext;

    class DDImage_API ElementTransformI : public RefCountedObject 
    {
      public:
        using ElementTransformPtr = RefCountedPtr<ElementTransformI>;

        enum TransformParam {
          eTranslation,
          eRotation,
          eScale,
          ePivotPoint,
          eSkew,
          eExtraMatrix,
          eExtraTranslate,
          eNumTransforms
        };

        ElementTransformI() = default;
        ElementTransformI(const ElementTransformI &) = delete;
        ~ElementTransformI() override = default;

        virtual bool setName(const std::string& name) = 0;

        //! Add or update a Vector3 key for the transform parameter. Returns false if incompatible.
        virtual bool addKey(TransformParam t, const OutputContext &ctx, const Vector3& v) = 0;
        virtual bool addExtraMatrixKey(const OutputContext &ctx, const Matrix4& v) = 0;
        //! Add a keyframe for all transform parameters at the given context.
        virtual void addTransformKey(const OutputContext &ctx) = 0;

        //! Reset transform to identity at the given ctx view.
        virtual void setIdentity(const OutputContext &ctx) = 0;

        virtual void setSkewOrder(float order) = 0;
        virtual void setTransformOrder(Matrix4::TransformOrder order) = 0;
        virtual void setRotationOrder(Matrix4::RotationOrder order) = 0;

        //! Remove the key for the specified transform parameter at the given context.
        virtual bool removeKey(TransformParam t, const OutputContext &ctx) = 0;
        //! Remove all transform keys at the given context.
        virtual bool removeTransformKey(const OutputContext &ctx) = 0;

        virtual const std::string& getName() const = 0;

        //! Get the Vector3 value for the transform parameter at the given context. Returns false if incompatible.
        virtual bool getKey(TransformParam t, const OutputContext &ctx, Vector3& v) const = 0;
        virtual bool getExtraMatrixKey(const OutputContext &ctx, Matrix4& v) const = 0;
        
        virtual float getSkewOrder() const = 0;
        virtual Matrix4::TransformOrder getTransformOrder() const = 0;
        virtual Matrix4::RotationOrder getRotationOrder() const = 0;

        //! Get all key times for the specified transform parameter in the ctx view.
        virtual bool getKeyTimes(TransformParam t, const OutputContext &ctx, std::vector<float>& keys) const = 0;
        //! Get all key times where any transform parameter has a key in the ctx view.
        virtual std::vector<float> getTransformKeyTimes(const OutputContext &ctx) const = 0;

        //! Build the final transform matrix from all parameters and the extra matrix.
        virtual bool getMatrix(const OutputContext &ctx, Matrix4& matrix, float pixelAspect) const = 0;

        //! Reset all transform parameters to default values.
        virtual void reset() = 0;
        virtual bool isDefault() const = 0;
    };

    class DDImage_API ElementI : public RefCountedObject 
    {
      public:
        typedef RefCountedPtr<ElementI> ElementPtr;
        typedef CurveI::CurvePtr CurvePtr;
      protected:
        ElementI();
        ElementI(const ElementI &);
      public:
        ~ElementI() override;

        virtual bool hasChildren() const = 0;
        virtual size_t numChildren() const = 0;
        virtual ElementPtr getChild(size_t childIndex) const = 0;
        virtual ElementPtr getParent() const = 0;

        virtual bool hasCurves() const = 0;
        virtual size_t numCurves() const = 0;
        virtual CurvePtr getCurve(size_t curveIndex, const std::string& view = "") const = 0;

        virtual ElementTransformI::ElementTransformPtr getTransform() = 0; 
        virtual Matrix4 getTransformMatrix(const OutputContext& ctx) const = 0;
        virtual bool setTransformMatrix(const OutputContext& ctx, const Matrix4& m) = 0;

        virtual float getAttribute(const OutputContext &, const std::string& attrName) const = 0;
        virtual void setAttribute(const std::string& attrName, const float value) = 0;
        virtual void setAttribute(const OutputContext &, const std::string& attrName, const float value) = 0;

        virtual bool setName(const std::string& name) = 0;
        virtual const std::string& getName() const = 0;
        virtual std::string getFullName() const = 0;

        virtual ElementPtr clone(bool deepClone) const = 0;

        virtual bool setFlag(CurveFlags flag, bool value) = 0;
        virtual bool getFlag(CurveFlags flag) const = 0;
    };
  }
}

#endif

// Copyright (c) 2012 The Foundry Visionmongers Ltd.  All Rights Reserved.

