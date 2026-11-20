// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef USG_GEOM_MATERIALPRIM_H
#define USG_GEOM_MATERIALPRIM_H

#include "usg/geom/NodeGraphPrim.h"
#include "usg/geom/ShaderPrim.h"

namespace usg
{
  class SchemaDescription;


  /*! Container of shader nodes and top-level terminal connections.
  */
  class USG_API MaterialPrim : public usg::NodeGraphPrim
  {
  public:
    //! Default ctor builds an empty (invalid) prim.
    MaterialPrim();
    //! Attempt to cast Prim to a MaterialPrim. MaterialPrim will be invalid if not successful.
    MaterialPrim(const Prim&);
    ~MaterialPrim();


    /*! Call this after construction to validate this Prim as a MaterialPrim type, returning
        true if it's still valid.
        If not the Prim::Impl will be invalidated and isValid() will then return false.
    */
    bool validate() override;

    //! Returns true if prim is a Material subclass.
    static bool  isValidPrim(const Prim& prim);


    //--------------------------------------------------------------


    /*! Defines a Material prim in the owner Layer, returning a constructed reference.
    */
    static MaterialPrim  defineInLayer(const LayerRef& layer,
                                       const Path&     path);

    /*! Retrieves an editable Material prim in the owner Stage, or an invalid one if none exists.
    */
    static MaterialPrim  getInStage(const StageConstRef& stage,
                                    const Path&          path);

    //--------------------------------------------------------------

    //! Set the path to the base Material of this Material. An empty path is equivalent to clearing the base Material.
    void setBaseMaterialPath(const Path& base_path);

    //--------------------------------------------------------------
    // Standard attributes for MaterialPrim:
    //  token outputs:surface
    //  token outputs:displacement
    //  token outputs:volume
    // 


    //! Return true if the prim as a valid surface shader output connection for the named 'render_target'.
    bool    hasSurfaceOutput(const Token& render_target = GeomTokens.universalRenderContext) const;

    //! Return true if the prim as a valid displacement shader output connection for the named 'render_target'.
    bool    hasDisplacementOutput(const Token& render_target = GeomTokens.universalRenderContext) const;

    //! Return true if the prim as a valid volume shader output connection for the named 'render_target'.
    bool    hasVolumeOutput(const Token& render_target = GeomTokens.universalRenderContext) const;


    /*! Build a ShaderDescGroup from this material's graph, sampled at an optional time.

        If 'render_target' is not empty it will be prefixed to 'terminal_type' to form
        the full connection point name. For example if render_target is 'slr' and
        terminal_type is 'surface' then the full name of the surface output property
        is 'outputs:slr:surface'.

        Returns 0 if the requested terminal for the render target does not exist, and the
        ShaderDescGroup will be left empty/uninitialized.

        If the material does not have the requested terminal for the render target this is
        not necessarily an error case as the requesting code can be trying various preferred
        render targets that may not exist for this particular Material. It's only an error if
        the requesting code cannot find an expected target port - but it's up to that code to
        consider it an error and respond appropriately.

        Method only supports UsdShade Prims.
    */
    size_t  getShaderDescGroup(const Token&          render_target,
                               const std::string&    terminal_type,
                               const fdk::TimeValue& time,
                               ShaderDescGroup&      shader_desc_group_out) const;


  };


}

#endif
