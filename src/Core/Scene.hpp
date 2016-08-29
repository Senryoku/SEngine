#pragma once

#include <DirectionalLight.hpp>
#include <OmnidirectionalLight.hpp>
#include <PointLight.hpp>
#include <MeshRenderer.hpp>
#include <Skybox.hpp>

/**
 * @todo Octree
**/
class Scene
{
public:
	Scene() =default;
	
	~Scene()
	{
		for(auto p : _lights)
			delete p;
	}
	
	void init()
	{
		_pointLightBuffer.init();
		_pointLightBuffer.bind(1);
	}
	
	std::vector<MeshRenderer>& getObjects()  { return _objects; }
	const std::vector<MeshRenderer>& getObjects() const { return _objects; }
	
	const std::vector<DirectionalLight*>& getLights() const { return _lights; }
	
	std::vector<DirectionalLight*>& getLights() { _dirtyLights = true; return _lights; }
	std::vector<OmnidirectionalLight>& getOmniLights() { _dirtyLights = true; return _omniLights; }
	
	template<typename T>
	inline T* add(T* dl)
	{
		getLights().push_back(dl);
		return dl;
	}
	
	std::vector<PointLight>& getPointLights()
	{
		_dirtyPointLights = true;
		return _pointLights;
	}
	
	const UniformBuffer& getPointLightBuffer() const { return _pointLightBuffer; }
	
	void updatePointLightBuffer()
	{
		_pointLightBuffer.data(_pointLights.data(), _pointLights.size() * sizeof(PointLight), Buffer::Usage::DynamicDraw);
		_dirtyPointLights = false;
	}

	void updateLights()
	{
		for(size_t i = 0; i < _lights.size(); ++i)
		{
			if(_dirtyLights)
			{
				_lights[i]->getGPUBuffer().bind(i + 2);
				_lights[i]->updateMatrices();
			}
		}
		for(size_t i = 0; i < _omniLights.size(); ++i)
		{
			if(_dirtyLights)
			{
				_omniLights[i].getGPUBuffer().bind(i + 12);
				_omniLights[i].updateMatrices();
			}
		}
		_dirtyLights = false;
	}
	
	void draw(const glm::mat4& p, const glm::mat4& v)
	{
		if(_skybox)
			_skybox.draw(p, v);

		updateLights();

		if(_dirtyPointLights)
			updatePointLightBuffer();

		for(const auto& o : _objects)
		{
			if(o.isVisible(p, v))
				o.draw();
		}
	}
	
	MeshRenderer& add(const MeshRenderer& m)
	{
		_objects.push_back(m);
		return _objects.back();
	}
	
	Skybox& getSkybox() { return _skybox; }
	
private:
	std::vector<MeshRenderer>	_objects;
	
	bool								_dirtyLights = true;
	std::vector<DirectionalLight*>		_lights;
	std::vector<OmnidirectionalLight>	_omniLights;
	
	bool							_dirtyPointLights = true;
	std::vector<PointLight>			_pointLights;
	UniformBuffer					_pointLightBuffer;
	
	Skybox							_skybox;
};
