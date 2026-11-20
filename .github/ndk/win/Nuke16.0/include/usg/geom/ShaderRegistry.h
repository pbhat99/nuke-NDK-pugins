// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef USG_SHADER_REGISTRY_H
#define USG_SHADER_REGISTRY_H

#include "ShaderProperty.h"

#include "usg/base/Token.h"

#include <memory>

namespace usg
{
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
class ShaderNodeHandle;

class USG_API ShaderNode
{
  public:
  ShaderNode(const ShaderNodeHandle& handle);
  ShaderNode(ShaderNodeHandle* pHandle);
  ShaderNode();

  // Copy constructor
  ShaderNode(const ShaderNode& shaderNode);

  // Assignment operator
  ShaderNode& operator=(const ShaderNode&);

  virtual ~ShaderNode();

  bool isValid() const;

  Token getContext() const;
  Token getIdentifier() const;

  std::vector<Token> getInputNames() const;
  std::vector<Token> getOutputNames() const;
  ShaderProperty getShaderInput(const Token& inputName) const;
  ShaderProperty getShaderOutput(const Token& outputName) const;
  fdk::KeyValueMap getMetadata() const;

  protected:
  std::unique_ptr<ShaderNodeHandle, void (*)(ShaderNodeHandle*)> _pImpl;

};

//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
class ShaderRegistryHandle;

class USG_API ShaderRegistry
{
public:
  ShaderRegistry(ShaderRegistryHandle* p);

  std::vector<Token> getAllNodeSourceTypes() const;
  std::vector<Token> getNodeIdentifiers() const;

  ShaderNode getShaderNodeByIdentifier(const Token& identifier) const;
  ShaderNode getShaderNodeByIdentifierAndType(const Token& identifier, const Token& type) const;

  static ShaderRegistry getInstance();

protected:
  std::unique_ptr<ShaderRegistryHandle, void(*)(ShaderRegistryHandle*)> _pImpl;

private:
  //! Disabled default constructor as SdrRegistry is not instantiable.
  ShaderRegistry() = delete;
  //! Disabled copy constructor.
  ShaderRegistry(const ShaderRegistry&) = delete;
  //! Disabled assignment operator.
  ShaderRegistry& operator = (const ShaderRegistry&) = delete;
};
  
} // namespace usg

#endif
