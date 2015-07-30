#pragma once

#include <map>
#include <memory>

#include <Singleton.hpp>

#include <Texture.hpp>
#include <Shader.hpp>

#include <Mesh.hpp>

class ResourcesManager : public Singleton<ResourcesManager>
{
public:
	ResourcesManager() =default;
	~ResourcesManager() =default;
	
	Shader& getShader(const std::string& name) throw(std::runtime_error);
	
	template<typename ShaderType>
	inline ShaderType& getShader(const std::string& name);
	
	Texture& getTexture(const std::string& name) throw(std::runtime_error);
	
	template<typename T>
	inline T& getTexture(const std::string& name);
	
	Program& getProgram(const std::string& name);
	
	void reloadShaders();
	
	bool isMesh(const std::string& name) const;
	
	Mesh& getMesh(const std::string& name);
	std::unique_ptr<Mesh>& getMeshPtr(const std::string& name) { return _meshes[name]; }

private:
	std::map<std::string, std::unique_ptr<Texture>>	_textures;
	std::map<std::string, std::unique_ptr<Shader>>		_shaders;
	
	std::map<std::string, Program>						_programs;
	
	std::map<std::string, std::unique_ptr<Mesh>>		_meshes;
};

template<typename ShaderType>
ShaderType& load(const std::string& path);

template<typename ... ShaderTypes>
Program& loadProgram(const std::string& name, const ShaderTypes& ... shaders);

#include <ResourcesManager.tcc>
