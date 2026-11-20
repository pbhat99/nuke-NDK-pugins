// Copyright (c) 2025 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef USG_GEOM_NODEGRAPHPRIM_H
#define USG_GEOM_NODEGRAPHPRIM_H

#include "usg/geom/Prim.h"
#include "usg/geom/ShaderPrim.h"

namespace usg
{
  class SchemaDescription;


  /*! Container of shader nodes and top-level terminal connections.
  */
  class USG_API NodeGraphPrim : public usg::Prim
  {
  public:
    //! Default ctor builds an empty (invalid) prim.
    NodeGraphPrim();
    //! Attempt to cast Prim to a NodeGraphPrim. NodeGraphPrim will be invalid if not successful.
    NodeGraphPrim(const Prim&);
    ~NodeGraphPrim();


    /*! Call this after construction to validate this Prim as a NodeGraphPrim type, returning
        true if it's still valid.
        If not the Prim::Impl will be invalidated and isValid() will then return false.
    */
    bool validate() override;

    //! Returns true if prim is a NodeGraph subclass.
    static bool  isValidPrim(const Prim& prim);


    //--------------------------------------------------------------


    /*! Defines a NodeGraph prim in the owner Layer, returning a constructed reference.
    */
    static NodeGraphPrim  defineInLayer(const LayerRef& layer,
                                        const Path&     path);

    /*! Retrieves an editable NodeGraph prim in the owner Stage, or an invalid one if none exists.
    */
    static NodeGraphPrim  getInStage(const StageConstRef& stage,
                                     const Path&          path);

  };


}

#endif
