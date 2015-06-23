#include <ResourcesManager.hpp>

Shader& ResourcesManager::getShader(const std::string& name) throw(std::runtime_error)
{
	auto it = _shaders.find(name);
	if(it != _shaders.end())
	{
		return *it->second.get();
	} else {
		throw std::runtime_error(name + " shader not found. Use a specialized version of getShader or make sure you referenced it to the ResourcesManager before calling getShader.");
	}
}

Texture& ResourcesManager::getTexture(const std::string& name) throw(std::runtime_error)
{
	auto it = _textures.find(name);
	if(it != _textures.end())
	{
		return *it->second.get();
	} else {
		throw std::runtime_error(name + " texture not found. Use a specialized version of getTexture or make sure you referenced it to the ResourcesManager before calling getTexture.");
	}
}

Program& ResourcesManager::getProgram(const std::string& name)
{ 
	return _programs[name];
}

void ResourcesManager::reloadShaders()
{
	for(auto& S : _shaders)
	{
		S.second->reload();
		S.second->compile();
	}
	
	for(auto& P : _programs)
		P.second.link();
}

bool ResourcesManager::isMesh(const std::string& name) const
{
	auto it = _meshes.find(name);
	return it != _meshes.end();
}
	
Mesh& ResourcesManager::getMesh(const std::string& name)
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
