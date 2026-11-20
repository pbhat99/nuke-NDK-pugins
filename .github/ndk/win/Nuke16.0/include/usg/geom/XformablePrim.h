// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

// This is automatically-generated code.
// You may add custom code between the --(BEGIN CUSTOM CODE)-- and --(END CUSTOM CODE)-- markers.
// Code added anywhere else will be destroyed when the code is generated.

#ifndef USG_GEOM_XFORMABLEPRIM_H
#define USG_GEOM_XFORMABLEPRIM_H

#include "usg/api.h"

#include "usg/geom/GeomTokens.h"
#include "usg/geom/XformOp.h"

#include "usg/geom/Prim.h"
#include "usg/base/ArrayTypes.h"
#include "usg/geom/ImageablePrim.h"

namespace usg {

  // class XformablePrim
  //   inherits from Imageable
  //
  // Attributes:
  //     uniform token[] xformOpOrder
  //
  // Inherited attributes:
  //   From Imageable:
  //     rel proxyPrim
  //     uniform token purpose = "default" (allowed values: default, render, proxy, guide)
  //     token visibility = "inherited" (allowed values: inherited, invisible)

  class USG_API XformablePrim : public ImageablePrim {
  public:
    XformablePrim();
    XformablePrim(const Prim& prim);
    ~XformablePrim();

    bool validate() override;
    static bool isValidPrim(const Prim& prim);
    static XformablePrim overrideInLayer(const LayerRef& layer, const Prim& prim);
    static XformablePrim getInStage(const StageRef& stage, const Path& path);

    Attribute createXformOpOrderAttr(const Value& defaultValue = Value());
    Attribute getXformOpOrderAttr() const;
    void setXformOpOrder(const TokenArray& value, fdk::TimeValue time = fdk::defaultTimeValue());
    TokenArray getXformOpOrder(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    // --(BEGIN CUSTOM CODE)--
    /*! Return the XformOp attribute name.
        If \a opSuffix is not empty it will be appended to the end of the XformOp attribute name so it can be made unique.
    */
    static std::string getXformOpAttrName(const std::string& opSuffix,
                                          const XformOp::Type& opType = XformOp::Type::Transform);

    //! Return true if prim can concatenate its transform (valid XformablePrim or ScopePrim.)
    static bool        canConcatenateTransform(const Prim& prim);

    //! Find the total first-last keyframe range for the prim and all its concatenable parents.
    static size_t      getConcatenatedXformOpTimeSamples(const Prim&        prim,
                                                         fdk::TimeValueSet& times);

    //! Return the local transform matrix for the prim, if valid, otherwise the identity.
    static fdk::Mat4d  getLocalMatrixAtPrim(const Prim&           prim,
                                            const fdk::TimeValue& time);

    //! Return the concatenated world transform matrix for the prim, if valid, otherwise the identity.
    static fdk::Mat4d  getConcatenatedMatrixAtPrim(const Prim&           prim,
                                                   const fdk::TimeValue& time);

    //! Return the concatenated world transform matrices for the prim at specific times, if valid, otherwise the array is empty.
    static void        getConcatenatedMatricesAtPrim(const Prim&               prim,
                                                     const fdk::TimeValueList& times,
                                                     usg::Mat4dArray&          matrices);

    //! Return the concatenated world transform matrices beginning at the parent of the prim, if valid, otherwise the identity.
    static fdk::Mat4d  getConcatenatedParentMatrixAtPrim(const Prim&           prim,
                                                         const fdk::TimeValue& time);

    //--------------------------------------------------------------
    // Inherited standard attributes from ImageablePrim class:
    //  token          visibility = "inherited" [inherited, invisible]
    //  uniform token  purpose = "default" [default, render, proxy, guide]
    //  <relationship> proxyPrim
    //
    // Standard attributes for this class:
    //  uniform token[] xformOpOrder
    //

    //----------------------------------

    //! Retrieves the local transform of this Prim.
    fdk::Mat4d  getLocalXform(const fdk::TimeValue& time = fdk::defaultTimeValue()) const;

    //! Retrieves the concatenated transform at this Prim (includes parents.)
    fdk::Mat4d  getConcatenatedXform(const fdk::TimeValue& time = fdk::defaultTimeValue()) const;

    //! Retrieve the union of the time samples for all assigned XformOps.
    bool        getTimeSamples(fdk::TimeValueList& times) const;

    //----------------------------------

    /*! Sets the transform stack of this prim to a single matrix xform op, replacing
        any existing xform ops.
        If \a opSuffix is not empty it will be appended to the end of the XformOp attribute
        name so it can be made unique.
    */
    void   setXformTo(const std::string&    opSuffix,
                      const fdk::Mat4d&     xform,
                      const fdk::TimeValue& time = fdk::defaultTimeValue());

    /*! Sets the transform stack of this prim to a single the specified type xform op without
        settings any value and returns the xform op as Attribute, replacing any existing xform ops.
        If \a opSuffix is not empty it will be appended to the end of the XformOp attribute
        name so it can be made unique.
    */
    Attribute setXformTo(const std::string& opSuffix,
                         const XformOp::Type& opType);

    /*! Adds a matrix transform op to this prim's transform stack.
        If \a opSuffix is not empty it will be appended to the end of the XformOp
        attribute name so it can be made unique.

        If an existing XformOp attribute with the same already exists this will
        replace or add to its value, depending on the \a time value.

        If prepend is true, the transform op will be added at the start.
    */
    void   addXformOp(const std::string&    opSuffix,
                      const fdk::Mat4d&     xform,
                      const fdk::TimeValue& time,
                      bool                  prepend = false);

    /*! Adds the specified type xform op to this prim's transform stack without settings any value
        and returns the xform op as Attribute.
        If \a opSuffix is not empty it will be appended to the end of the XformOp
        attribute name so it can be made unique.

        If an existing XformOp attribute with the same already exists this will
        replace or add to its value, depending on the \a time value.

        If prepend is true, the transform op will be added at the start.
    */
    Attribute addXformOp(const std::string& opSuffix,
                         const XformOp::Type& opType,
                         bool prepend = false);

    //! Removes all xform ops in this prim.
    void   clearAllXformOps();

    //! Retrieves the name of the ordered xformOps.
    fdk::StringList getOrderedXformOps() const;

    //! Copies the xformOps from the other xformable prim.
    void   copyXformOps(const XformablePrim& xformable, fdk::TimeValue time);

    /*! Specifies whether this prim's transform should reset the transformation stack inherited from
        its parent prim.
    */
    bool setResetXformStack(const bool resetXformStack);
    // --(END CUSTOM CODE)--
  };

}

#endif
