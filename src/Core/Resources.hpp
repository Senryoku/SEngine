#pragma once

#include <unordered_map>
#include <memory>

#include <Texture.hpp>
#include <Shader.hpp>

#include <Graphics/Mesh.hpp>

namespace Resources
{

extern std::unordered_map<std::string, std::unique_ptr<Texture>>	_textures;
extern std::unordered_map<std::string, std::unique_ptr<Shader>>		_shaders;

extern std::unordered_map<std::string, Program>						_programs;

extern std::unordered_map<std::string, std::unique_ptr<Mesh>>		_meshes;

Shader& getShader(const std::string& name) throw(std::runtime_error);

template<typename ShaderType>
inline ShaderType& getShader(const std::string& name);

Texture& getTexture(const std::string& name) throw(std::runtime_error);

template<typename T>
inline T& getTexture(const std::string& name);

Program& getProgram(const std::string& name);

void reloadShaders();

bool isMesh(const std::string& name);

void clearMeshes();

Mesh& getMesh(const std::string& name);
inline std::unique_ptr<Mesh>& getMeshPtr(const std::string& name) { return _meshes[name]; }

template<typename ShaderType>
inline ShaderType& load(const std::string& path);

template<typename ShaderType>
inline ShaderType& load(const std::string& name, const std::string& path);

template<typename ... ShaderTypes>
inline Program& loadProgram(const std::string& name, ShaderTypes& ... shaders);

} // Resources namespace

#include <Core/Resources.tcc>
