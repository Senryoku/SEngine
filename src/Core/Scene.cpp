#include <Scene.hpp>

#include <Meta.hpp>
#include <ComponentValidation.hpp>

#include <Resources.hpp>

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
	for_each<deletion_pass_wrapper, ComponentTypes>{}();
	
	updateLights();
	if(_dirtyPointLights)
		updatePointLightBuffer();
}

void Scene::occlusion_query()
{
	if(!UseOcclusionCulling) return;

	static auto& shader = Resources::getProgram("AABB");
	shader.use();
	for(auto& it : ComponentIterator<MeshRenderer>{})
	{
		it.occlusion_query();
	}
	shader.useNone();
}

unsigned int Scene::draw(const Camera& c) const
{
	unsigned int draw_calls = 0;
	if(_skybox)
	{
		_skybox.draw(c.getProjectionMatrix(), c.getViewMatrix());
		++draw_calls;
	}
	
	for(const auto& it : ComponentIterator<MeshRenderer>{})
	{
		if(UseOcclusionCulling)
		{
			it.draw_occlusion_culled();
			++draw_calls;
		} else if(!UseFrustumCulling || it.isVisible(c.getFrustum())) {
			it.draw();
			++draw_calls;
		}
	}
	
	return draw_calls;
}
