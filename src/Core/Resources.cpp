#include <Core/Resources.hpp>

std::unordered_map<std::string, std::unique_ptr<Texture>>	Resources::_textures;
std::unordered_map<std::string, std::unique_ptr<Shader>>	Resources::_shaders;

std::unordered_map<std::string, Program>					Resources::_programs;

std::unordered_map<std::string, std::unique_ptr<Mesh>>		Resources::_meshes;

Shader& Resources::getShader(const std::string& name) throw(std::runtime_error)
{
	auto it = _shaders.find(name);
	if(it != _shaders.end())
	{
		return *it->second.get();
	} else {
		throw std::runtime_error(name + " shader not found. Use a specialized version of getShader or make sure you referenced it to the ResourcesManager before calling getShader.");
	}
}

Texture& Resources::getTexture(const std::string& name) throw(std::runtime_error)
{
	auto it = _textures.find(name);
	if(it != _textures.end())
	{
		return *it->second.get();
	} else {
		throw std::runtime_error(name + " texture not found. Use a specialized version of getTexture or make sure you referenced it to the ResourcesManager before calling getTexture.");
	}
}

Program& Resources::getProgram(const std::string& name)
{ 
	return _programs[name];
}

void Resources::reloadShaders()
{
	for(auto& S : _shaders)
	{
		S.second->reload();
		S.second->compile();
	}
	
	for(auto& P : _programs)
		P.second.link();
}

bool Resources::isMesh(const std::string& name)
{
	auto it = _meshes.find(name);
	return it != _meshes.end();
}
	
Mesh& Resources::getMesh(const std::string& name)
{
	auto it = _meshes.find(name);
	if(it != _meshes.end())
	{
		return *it->second;
	} else {
		auto newMesh = new Mesh();
		_meshes[name].reset(newMesh);
		return *newMesh;
	}
}
