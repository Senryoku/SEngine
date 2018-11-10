#pragma once

#include <vector>

#include <PointLight.hpp>
#include <Skybox.hpp>
#include <Camera.hpp>

#include <ComponentTypes.hpp>

using ComponentTypes = TList<Transformation, MeshRenderer, SpotLight, CollisionBox>;

/**
 * @todo Octree
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
	void occlusion_query();
	/// @return The number of draw calls generated.
	unsigned int draw(const Camera& c) const;
	
	inline Skybox& getSkybox() { return _skybox; }
	
	bool UseFrustumCulling = true;
	bool UseOcclusionCulling = false;
	
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
