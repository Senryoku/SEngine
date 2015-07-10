#pragma once

#include <Light.hpp>
#include <PointLight.hpp>
#include <MeshInstance.hpp>
#include <Skybox.hpp>

/**
 * @todo Octree
**/
class Scene
{
public:
	Scene() =default;
	
	void init()
	{
		_pointLightBuffer.init();
		_pointLightBuffer.bind(1);
	}
	
	const std::vector<MeshInstance>& getObjects() const { return _objects; }
	
	const std::vector<Light>& getLights() const { return _lights; }
	
	std::vector<Light>& getLights() { _dirtyLights = true; return _lights; }
	
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
			_lights[i].getGPUBuffer().bind(i + 2);
			_lights[i].updateMatrices();
		}
		_dirtyLights = false;
	}
	
	void draw(const glm::mat4& p, const glm::mat4& v)
	{
		if(_skybox)
			_skybox.draw(p, v);
		
		if(_dirtyLights)
			updateLights();
		
		if(_dirtyPointLights)
			updatePointLightBuffer();
		
		for(auto& o : _objects)
		{
			if(o.isVisible(p, v))
				o.draw();
		}
	}
	
	MeshInstance& add(const MeshInstance& m)
	{
		_objects.push_back(m);
		return _objects.back();
	}
	
	Skybox& getSkybox() { return _skybox; }
	
private:
	std::vector<MeshInstance>	_objects;
	
	bool							_dirtyLights = true;
	std::vector<Light>			_lights;
	
	bool							_dirtyPointLights = true;
	std::vector<PointLight>		_pointLights;
	UniformBuffer					_pointLightBuffer;
	
	Skybox							_skybox;
};
