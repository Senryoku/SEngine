#pragma once

#include <vector>

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
	
	inline const UniformBuffer& getPointLightBuffer() const { return _pointLightBuffer; }
	inline std::vector<PointLight>& getPointLights();
	inline void updatePointLightBuffer();

	void updateLights();
	void update();
	void draw(const glm::mat4& p, const glm::mat4& v) const;
	
	inline Skybox& getSkybox() { return _skybox; }
	
private:
	bool							_dirtyPointLights = true;
	std::vector<PointLight>			_pointLights;
	UniformBuffer					_pointLightBuffer;
	
	Skybox							_skybox;
};

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
