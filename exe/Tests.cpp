#include <Application.hpp>
#include <NoisyTerrain.hpp>

#include <glm/gtx/transform.hpp>

class Test : public Application
{
	virtual void run_init() override
	{
		Application::run_init();
		
		auto& Deferred = loadProgram("Deferred",
			load<VertexShader>("src/GLSL/Deferred/deferred_vs.glsl"),
			load<FragmentShader>("src/GLSL/Deferred/deferred_normal_map_fs.glsl")
		);

		ComputeShader& DeferredShadowCS = ResourcesManager::getInstance().getShader<ComputeShader>("DeferredShadowCS");

		_camera.speed() = 15;
		_camera.setPosition(glm::vec3(0.0, 15.0, -20.0));
		_camera.lookAt(glm::vec3(0.0, 5.0, 0.0));
		
		Deferred.bindUniformBlock("Camera", _camera_buffer); 

		Texture2D& GroundTexture = ResourcesManager::getInstance().getTexture<Texture2D>("GroundTexture");
		GroundTexture.load("in/Textures/Tex0.jpg");
		Texture2D& GroundNormalMap = ResourcesManager::getInstance().getTexture<Texture2D>("GroundNormalMap");
		GroundNormalMap.load("in/Textures/Tex0_n.jpg");
		Texture2D& GrassTexture = ResourcesManager::getInstance().getTexture<Texture2D>("GrassTexture");
		GrassTexture.load("in/Textures/Lawn_grass_pxr128.png");
		Texture2D& GrassNormalMap = ResourcesManager::getInstance().getTexture<Texture2D>("GrassNormalMap");
		GrassNormalMap.load("in/Textures/Lawn_grass_pxr128_normal.png");

		Mesh& Plane = ResourcesManager::getInstance().getMesh("Plane");
		float s = 250.f;
		auto T = NoisyTerrain({3.f, 0.5f, 0.1f, 0.05f, 0.005f},
								{15.f, 4.f, 3.f, 2.f, 1.f},
								{0.125f/2.f, 0.125f, 0.25f, 0.5f, 1.f});
		Plane = create(T, glm::vec2(0.0), glm::vec2(s), glm::vec2(200));
		for(auto& v : Plane.getVertices())
		{
			v.texcoord *= 0.3f;
		}
		Plane.setBoundingBox({glm::vec3(-s, 0.f, -s), glm::vec3(s, 100.f, s)});
		Plane.createVAO();
		Plane.getMaterial().setShadingProgram(loadProgram("Terrain",
			load<VertexShader>("src/GLSL/Deferred/deferred_vs.glsl"),
			load<FragmentShader>("src/GLSL/Deferred/deferred_terrain_fs.glsl")
		));
		Plane.getMaterial().setUniform("Texture0", GroundTexture);
		Plane.getMaterial().setUniform("Texture1", GrassTexture);
		Plane.getMaterial().setUniform("useNormalMap", 1);
		Plane.getMaterial().setUniform("NormalMap0", GroundNormalMap);
		Plane.getMaterial().setUniform("NormalMap1", GrassNormalMap);
		Plane.getMaterial().setUniform("R", 0.8f);
		Plane.getMaterial().setUniform("F0", 0.5f);

		_scene.add(MeshInstance(Plane));

		auto Model = Mesh::load("in/3DModels/dragon/Figurine Dragon N170112.3DS");
		auto& ModelTexture = ResourcesManager::getInstance().getTexture<Texture2D>("Dragon");
		ModelTexture.load("in/3DModels/dragon/AS2_concrete_02.jpg");
		for(auto part : Model)
		{
			part->createVAO();
			part->getMaterial().setShadingProgram(Deferred);
			part->getMaterial().setUniform("Texture", ModelTexture);
			part->getMaterial().setUniform("NormalMap", GroundNormalMap);
			part->getMaterial().setUniform("useNormalMap", 1);
			for(int i = 0; i < 10; ++i)
			{
				for(int j = 0; j < 10; ++j)
				{
					auto& m = _scene.add(MeshInstance(*part, glm::scale(glm::translate(glm::mat4(1.0), glm::vec3(10.0 * i, 0.0, 10.0 * j)), glm::vec3(0.04))));
					m.getMaterial().setUniform("R", (i + 1) * 0.1f);
					m.getMaterial().setUniform("F0", (j + 1) * 0.1f);
				}
			}
		}
		
		auto Sponza = Mesh::load("in/3DModels/sponza/sponza.obj");
		for(auto part : Sponza)
		{
			part->createVAO();
			part->getMaterial().setShadingProgram(Deferred);
			//part->getMaterial().setUniform("useNormalMap", 0);
			part->getMaterial().setUniform("R", 0.8f);
			part->getMaterial().setUniform("F0", 0.5f);
			_scene.add(MeshInstance(*part, glm::scale(glm::translate(glm::mat4(1.0), glm::vec3(150.0, 0.0, 150.0)), glm::vec3(0.04))));
		}
		
		const size_t LightCount = 1000;
		DeferredShadowCS.getProgram().setUniform("lightCount", LightCount);
		DeferredShadowCS.getProgram().setUniform("lightRadius", 10.0f);
		DeferredShadowCS.getProgram().bindUniformBlock("LightBlock", _scene.getPointLightBuffer());

		for(size_t i = 0; i < LightCount; ++i)
		{
			_scene.getPointLights().push_back(PointLight{
				glm::vec4((float) (i % 25) * 10.0f, 3.f, (float) (i / 25) * 10.0f - 5.0f, 1.0f), 	// Position
				glm::vec4(1.0)		// Color
			});
		}

		// Shadow casting lights ---------------------------------------------------

		_scene.getLights().resize(2);
		
		_scene.getLights()[0].init();
		_scene.getLights()[0].setColor(glm::vec4(1.0));
		_scene.getLights()[0].setPosition(glm::vec3(50.0, 50.0, 120.0));
		_scene.getLights()[0].lookAt(glm::vec3(50.0, 0.0, 50.0));
		_scene.getLights()[0].updateMatrices();
		
		_scene.getLights()[1].init();
		_scene.getLights()[1].setColor(glm::vec4(0.9, 0.6, 0.6, 1.0));
		_scene.getLights()[1].setPosition(glm::vec3(120.0, 50.0, 50.0));
		_scene.getLights()[1].lookAt(glm::vec3(50.0, 0.0, 50.0));
		_scene.getLights()[1].updateMatrices();

		for(size_t i = 0; i < _scene.getLights().size(); ++i)
		{
			_scene.getLights()[i].drawShadowMap(_scene.getObjects());
			DeferredShadowCS.getProgram().setUniform(std::string("ShadowMaps[").append(std::to_string(i)).append("]"), (int) i + 3);
		}
		
		DeferredShadowCS.getProgram().setUniform("shadowCount", _scene.getLights().size());
	}
	
	virtual void in_loop_update() override
	{
		for(auto& l :_scene.getPointLights())
		{
			l.position.y = 8.0 + 4.0 * std::sin(TimeManager::getInstance().getRuntime() + l.position.x * l.position.z);
		}
	}
};

int main(int argc, char* argv[])
{
	Test _app;
	_app.init();	
	_app.run();
}
