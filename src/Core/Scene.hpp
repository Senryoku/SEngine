#pragma once

#include <DirectionalLight.hpp>
#include <OmnidirectionalLight.hpp>
#include <PointLight.hpp>
#include <Skybox.hpp>

/**
 * @todo Octree
 * @todo Remove lights from here (turn them into standard components)
 * @todo Delegates to systems
**/
class Scene
{
public:
	Scene() =default;
	
	~Scene();
	
	void init();
	
	inline const std::vector<DirectionalLight*>& getLights() const { return _lights; }
	inline std::vector<DirectionalLight*>& getLights() { _dirtyLights = true; return _lights; }
	inline std::vector<OmnidirectionalLight>& getOmniLights() { _dirtyLights = true; return _omniLights; }
	inline const UniformBuffer& getPointLightBuffer() const { return _pointLightBuffer; }
	
	template<typename T>
	inline T* add(T* dl);
	
	inline std::vector<PointLight>& getPointLights();
	inline void updatePointLightBuffer();

	void updateLights();
	void update();
	void draw(const glm::mat4& p, const glm::mat4& v) const;
	
	inline Skybox& getSkybox() { return _skybox; }
	
private:	
	bool								_dirtyLights = true;
	std::vector<DirectionalLight*>		_lights;
	std::vector<OmnidirectionalLight>	_omniLights;
	
	bool							_dirtyPointLights = true;
	std::vector<PointLight>			_pointLights;
	UniformBuffer					_pointLightBuffer;
	
	Skybox							_skybox;
};
	
template<typename T>
inline T* Scene::add(T* dl)
{
	getLights().push_back(dl);
	return dl;
}

inline std::vector<PointLight>& Scene::getPointLights()
{
	_dirtyPointLights = true;
	return _pointLights;
}

inline void Scene::updatePointLightBuffer()
{
	_pointLightBuffer.data(_pointLights.data(), _pointLights.size() * sizeof(PointLight), Buffer::Usage::DynamicDraw);
	_dirtyPointLights = false;
}
