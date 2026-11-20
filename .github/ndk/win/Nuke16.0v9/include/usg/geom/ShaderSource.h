// Copyright (c) 2023 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef USG_GEOM_SHADERSOURCE_H
#define USG_GEOM_SHADERSOURCE_H

#include "usg/geom/ShaderProperty.h"
#include "usg/base/ArgSet.h" // for TokenValueMapSorted

#include <unordered_map>

namespace usg
{

  class ShaderSchema;
  class ShaderDesc;
  class ShaderDescGroup;
  class MaterialSource;
  class ShaderSource;
  using MaterialConnectionsMap = std::map<Token, ShaderConnectionList>;
  using ShaderDescConstPtrList = std::vector<const ShaderDesc*>;
  using ShaderDescNameMap = std::unordered_map<const ShaderDesc*, std::string>;

  //-----------------------------------------------------------------------------


  //! Helper interface to define material source code for different targets.
  //! Primarily use case is for creating custom glslfx runtime code.
  class USG_API SourceCodeGenerator
  {
  public:
    //! Returns the source target type of the generator.
    virtual const Token&  sourceType() const = 0;

    //! Create (allocate) and return the MaterialSource generator to use when creating custom
    //! shader code for this generator's source type.
    //! Calling code takes ownership of returned allocation and typically will destroy it
    //! after the MaterialSource's \a emitSourceCode() method has been run.
    virtual MaterialSource*  createMaterialSourceGenerator(const ShaderDescGroup& shaderGroup) const = 0;
  };


  //-----------------------------------------------------------------------------


  //! Interface to define material source code such as the surface shader method.
  class USG_API MaterialSource
  {
  public:
    MaterialSource(const SourceCodeGenerator& generator,
                   const ShaderDescGroup&     shaderGroup);
    virtual ~MaterialSource() {}

    //! Returns the SourceCodeGenerator that created this MaterialSource.
    const SourceCodeGenerator& sourceTypeGenerator() const { return _generator; }

    //! Returns the source target type of the generator.
    const usg::Token&  sourceType() const { return _generator.sourceType(); }

    //! Returns the ShaderDescGroup being converted into source code.
    const ShaderDescGroup&  shaderGroup() const { return _shaderGroup; }

    //! This material has varying opacity values across its surface. Used if shaders
    //! need to declare this as a renderer hint.
    bool  modulatesOpacity() const { return _modulatesOpacity; }

    //! Build a unique name for this shader.
    //! Default implementation combines the name of the ShaderDesc's schema name with the
    //! current size of _outputShaders, thereby numbering them uniquely.
    virtual std::string  buildUniqueShaderName(const ShaderDesc* shaderDesc) const;

    //! Build a unique name for this input name.
    //! Default implementation combines the name of the ShaderDesc's unique output name
    //! with the input name.
    virtual std::string  buildUniqueInputName(const ShaderDesc*  shaderDesc,
                                              const std::string& inputName) const;

    //! Retrieve the unique input property values to set in the output source.
    //! The names of the properties are usually created via \a buildUniqueInputName().
    virtual TokenValueMapSorted     getShaderPropertyValues(const ShaderDesc* shaderDesc) const = 0;

    //! Retrieve the list of valid input connections for this ShaderDesc in this material.
    virtual MaterialConnectionsMap  getShaderConnections(const ShaderDesc* shaderDesc) const = 0;

    //------------------------------------------------------

    //! Create (allocate) and return the ShaderSource generator to call when the given
    //! ShaderSchema needs to create custom shader code for this MaterialSource.
    //! The created ShaderSource is used for a single ShaderSchema instance and can store any
    //! temporary data it needs to prepare for the \a ShaderSchema::sourceCode() method to be
    //! called.
    //! Calling code takes ownership of returned allocation and typically will destroy it
    //! after the ShaderSchema's \a sourceCode() method has been run.
    virtual ShaderSource*  createShaderSourceGenerator(const ShaderSchema* schema,
                                                       std::stringstream&  sourceOut) const = 0;

    //------------------------------------------------------

    //! List of ShaderDesc pointers sorted into topological dependency order so shaders
    //! that depend on other shaders come after their dependencies.
    const ShaderDescConstPtrList&  shadersInTopologicalOrder() const { return _topologicalOrder; }

    //! Map of ShaderDescGroup shaders and their constructed(unique-ified) output shader names.
    const ShaderDescNameMap&  outputShaderNames() const { return _outputShaders; }

    //! Retrieve the constructed(unique-ified) shader name for source code output.
    const std::string&  getOutputShaderName(const ShaderDesc* shaderDesc) const;


  public:
    //! Generate source code text defining everything that the material will need to
    //! compile. This method will typically take the ShaderDescGroup and walk through
    //! all its ShaderSchemas converting them to source by calling
    //! \a createShaderSourceGenerator() for each.
    virtual void  emitSourceCode(std::stringstream& source) const = 0;


  protected:
    const SourceCodeGenerator& _generator;
    const ShaderDescGroup&     _shaderGroup;
    bool                       _modulatesOpacity{false};  //!< Material has varying opacity values across its surface
    ShaderDescConstPtrList     _topologicalOrder;         //!< List of shaders sorted in topological dependency order
    ShaderDescNameMap          _outputShaders;            //!< Map of ShaderDescs and their output shader names

  };


  //-----------------------------------------------------------------------------


  //! Interface to define shader source code.
  class USG_API ShaderSource
  {
  public:
    ShaderSource(const MaterialSource& materialSourceGen,
                 const ShaderSchema*   schema,
                 std::stringstream&    sourceDest);
    virtual ~ShaderSource() {}

    //! Returns the SourceCodeGenerator that created this MaterialSource.
    const SourceCodeGenerator&  sourceTypeGenerator() const { return _materialSource.sourceTypeGenerator(); }

    //! Returns the source target type of the generator.
    const usg::Token&  sourceType() const { return _materialSource.sourceType(); }

    //! Returns the MaterialSource that created this ShaderSource.
    const MaterialSource&  materialSource() const { return _materialSource; }

    //------------------------------------------------------

    //! Starts a shader function with the given set of inputs.
    virtual void  startFunction() = 0;

    //! Emits the shader line source.
    virtual void  emitLine(const std::string& line) = 0;

    //! Returns code required to retrieve a value from the named input.
    virtual std::string  input(const std::string& name) = 0;

    //! Returns code that allows sampling of a texture at the given texture coordinate.
    virtual std::string  sampleTexture(const std::string& inputName, const std::string& texCoordVarName) = 0;

    //! Declares and defines a shader output to be equal to the given expression.
    virtual void  output(const std::string& name, const std::string& expression) = 0;

    //! Ends the shader function.
    virtual void  endFunction() = 0;

    //------------------------------------------------------

    //! Return the list of named inputs created by calls to \a input().
    virtual fdk::StringList  inputs() const = 0;

    //! Return the list of named texture inputs created by calls to \a input().
    virtual const fdk::StringSet&  textures() const = 0;


  protected:
    const MaterialSource& _materialSource;
    const ShaderSchema*   _shaderSchema;
    std::stringstream&    _sourceDest;
  };


} // namespace usg

#endif
