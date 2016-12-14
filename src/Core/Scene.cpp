#include <Scene.hpp>

#include <Meta.hpp>
#include <ComponentValidation.hpp>

#include <SpotLight.hpp>

Scene::~Scene()
{
}

void Scene::init()
{
	_pointLightBuffer.init();
	_pointLightBuffer.bind(1);
}
	
void Scene::updateLights()
{
	// Updates GPU Buffer
	size_t sl = 0;
	for(auto& it : ComponentIterator<SpotLight>{})
	{
		it.getGPUBuffer().bind(sl++ + 2);
		if(it.dynamic) // Update Shadow maps (only if asked to)
		{
			it.updateMatrices();
			it.drawShadowMap(ComponentIterator<MeshRenderer>{});
		}
	}
}

void Scene::update()
{
	updateLights();
	if(_dirtyPointLights)
		updatePointLightBuffer();
	
	/// @todo Move elsewhere?
	for_each<deletion_pass_wrapper, TList<Transformation, MeshRenderer, SpotLight>>{}();
}

void Scene::draw(const glm::mat4& p, const glm::mat4& v) const
{
	if(_skybox)
		_skybox.draw(p, v);

	for(const auto& it : ComponentIterator<MeshRenderer>{})
	{
		if(it.isVisible(p, v))
			it.draw();
	}
}
