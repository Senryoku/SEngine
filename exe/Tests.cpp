#include <App.hpp>

#include <glm/gtx/transform.hpp>

class Test : public App
{
	virtual void run_init() override
	{
		App::run_init();
		
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

		Mesh& Plane = ResourcesManager::getInstance().getMesh("Plane");;
		float s = 100.f;
		Plane.getVertices().push_back(Mesh::Vertex(glm::vec3(-s, 0.f, -s), glm::vec3(0.f, 1.0f, 0.0f), glm::vec2(0.f, 20.f)));
		Plane.getVertices().push_back(Mesh::Vertex(glm::vec3(-s, 0.f, s), glm::vec3(0.f, 1.0f, 0.0f), glm::vec2(0.f, 0.f)));
		Plane.getVertices().push_back(Mesh::Vertex(glm::vec3(s, 0.f, s), glm::vec3(0.f, 1.0f, 0.0f), glm::vec2(20.f, 0.f)));
		Plane.getVertices().push_back(Mesh::Vertex(glm::vec3(s, 0.f, -s), glm::vec3(0.f, 1.0f, 0.0f), glm::vec2(20.f, 20.f)));
		Plane.getTriangles().push_back(Mesh::Triangle(0, 1, 2));
		Plane.getTriangles().push_back(Mesh::Triangle(0, 2, 3));
		Plane.setBoundingBox({glm::vec3(-s, 0.f, -s), glm::vec3(s, 0.f, s)});
		Plane.createVAO();
		Plane.getMaterial().setShadingProgram(Deferred);
		Plane.getMaterial().setUniform("Texture", GroundTexture);
		Plane.getMaterial().setUniform("useNormalMap", 1);
		Plane.getMaterial().setUniform("NormalMap", GroundNormalMap);

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
			_scene.add(MeshInstance(*part, glm::scale(glm::mat4(1.0), glm::vec3(0.04))));
		}

		const size_t LightCount = 100;
		DeferredShadowCS.getProgram().setUniform("lightCount", LightCount);
		DeferredShadowCS.getProgram().setUniform("lightRadius", 10.0f);
		DeferredShadowCS.getProgram().bindUniformBlock("LightBlock", _scene.getPointLightBuffer());

		for(size_t i = 0; i < LightCount; ++i)
		{
			_scene.getPointLights().push_back(PointLight{
				glm::vec4((float) (i % 10) * 10.0f, 2.f, (float) (i / 10) * 10.0f, 1.0f), 	// Position
				glm::vec4(1.0)		// Color
			});
		}

		// Shadow casting lights ---------------------------------------------------

		_scene.getLights().resize(2);
		
		_scene.getLights()[0].init();
		_scene.getLights()[0].setColor(glm::vec4(0.8));
		_scene.getLights()[0].setPosition(glm::vec3(0.0, 40.0, 100.0));
		_scene.getLights()[0].lookAt(glm::vec3(0.0, 10.0, 0.0));
		_scene.getLights()[0].updateMatrices();
		
		_scene.getLights()[1].init();
		_scene.getLights()[1].setColor(glm::vec4(0.6, 0.0, 0.0, 1.0));
		_scene.getLights()[1].setPosition(glm::vec3(100.0, 40.0, 100.0));
		_scene.getLights()[1].lookAt(glm::vec3(50.0, 10.0, 50.0));
		_scene.getLights()[1].updateMatrices();

		for(size_t i = 0; i < _scene.getLights().size(); ++i)
		{
			_scene.getLights()[i].drawShadowMap(_scene.getObjects());
			DeferredShadowCS.getProgram().setUniform(std::string("ShadowMaps[").append(std::to_string(i)).append("]"), (int) i + 3);
		}
		
		DeferredShadowCS.getProgram().setUniform("shadowCount", _scene.getLights().size());
	}
};

int main(int argc, char* argv[])
{
	Test _app;
	_app.init();	
	_app.run();
}
