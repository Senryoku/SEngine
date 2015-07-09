#include <Application.hpp>
#include <NoisyTerrain.hpp>
#include <MathTools.hpp>

#include <glm/gtx/transform.hpp>

class Test : public Application
{
public:
	Test(int argc, char* argv[]) : 
		Application(argc, argv)
	{
	}
	
	virtual void run_init() override
	{
		Application::run_init();
		
		auto& Deferred = loadProgram("Deferred",
			load<VertexShader>("src/GLSL/Deferred/deferred_vs.glsl"),
			load<FragmentShader>("src/GLSL/Deferred/deferred_normal_map_fs.glsl")
		);
		
		auto& LightDraw = loadProgram("LightDraw",
			load<VertexShader>("src/GLSL/Debug/light_vs.glsl"),
			load<GeometryShader>("src/GLSL/Debug/light_gs.glsl"),
			load<FragmentShader>("src/GLSL/Debug/light_fs.glsl")
		);

		ComputeShader& DeferredShadowCS = ResourcesManager::getInstance().getShader<ComputeShader>("DeferredShadowCS");
		
		ComputeShader& GaussianBlur = ResourcesManager::getInstance().getShader<ComputeShader>("GaussianBlur");
		GaussianBlur.loadFromFile("src/GLSL/gaussian_blur_h_cs.glsl");
		GaussianBlur.compile();
		ComputeShader& GaussianBlurV = ResourcesManager::getInstance().getShader<ComputeShader>("GaussianBlurV");
		GaussianBlurV.loadFromFile("src/GLSL/gaussian_blur_v_cs.glsl");
		GaussianBlurV.compile();

		_camera.speed() = 15;
		_camera.setPosition(glm::vec3(0.0, 15.0, -20.0));
		_camera.lookAt(glm::vec3(0.0, 5.0, 0.0));
		
		Deferred.bindUniformBlock("Camera", _camera_buffer); 
		LightDraw.bindUniformBlock("Camera", _camera_buffer); 

		auto TestMesh = Mesh::load("in/3DModels/sponza/sponza.obj");
		for(auto part : TestMesh)
		{
			part->createVAO();
			part->getMaterial().setUniform("k", 0.2f);
			part->getMaterial().setUniform("R", 0.4f);
			part->getMaterial().setUniform("F0", 0.2f);
			_scene.add(MeshInstance(*part, glm::scale(glm::translate(glm::mat4(1.0), glm::vec3(0.0, 0.0, 0.0)), glm::vec3(0.04))));
		}

		_scene.getPointLights().push_back(PointLight{
			glm::vec4(0.0, 10.0, 10.0, 1.0), 	// Position
			glm::vec4(4.0) // Color
		});
		
		DeferredShadowCS.getProgram().setUniform("lightCount", _scene.getPointLights().size());
		DeferredShadowCS.getProgram().setUniform("lightRadius", 10.0f);
		DeferredShadowCS.getProgram().bindUniformBlock("LightBlock", _scene.getPointLightBuffer());
		LightDraw.bindUniformBlock("LightBlock", _scene.getPointLightBuffer());

		// Shadow casting lights ---------------------------------------------------

		_scene.getLights().resize(1);
		
		_scene.getLights()[0].init();
		_scene.getLights()[0].setColor(glm::vec4(2.0));
		_scene.getLights()[0].setPosition(glm::vec3(25.0, 50.0, 25.0));
		_scene.getLights()[0].lookAt(glm::vec3(0.0, 0.0, 0.0));
		_scene.getLights()[0].updateMatrices();

		for(size_t i = 0; i < _scene.getLights().size(); ++i)
		{
			_scene.getLights()[i].drawShadowMap(_scene.getObjects());
			blur(_scene.getLights()[i].getShadowMap(), _scene.getLights()[i].getResolution());
			DeferredShadowCS.getProgram().setUniform(std::string("ShadowMaps[").append(std::to_string(i)).append("]"), (int) i + 3);
		}
		
		DeferredShadowCS.getProgram().setUniform("shadowCount", _scene.getLights().size());
	}
	
	virtual void in_loop_update() override
	{
		/*
		_scene.getLights()[4].setPosition(glm::vec3(150.0, 150.0, 150.0)
												+ 20.0f * glm::vec3(std::cos(0.1f * TimeManager::getInstance().getRuntime()), 0.0,
																	std::sin(0.1f * TimeManager::getInstance().getRuntime())));
		_scene.getLights()[4].lookAt(glm::vec3(150.0, 0.0, 150.0));
		_scene.getLights()[4].drawShadowMap(_scene.getObjects());
		
		blur(_scene.getLights()[4].getShadowMap(), _scene.getLights()[4].getResolution());
		*/
		for(auto& l :_scene.getPointLights())
		{
			l.position.y = 8.0 + 4.0 * std::sin(TimeManager::getInstance().getRuntime() + l.position.x * l.position.z);
		}
		
		/// @todo Fix bug (black dots) when light position == camera position
		//_scene.getPointLights()[0].position = glm::vec4(_camera.getPosition(), 1.0);
	}
	
	virtual void offscreen_render(const glm::mat4& p, const glm::mat4& v) override
	{
		auto& ld = ResourcesManager::getInstance().getProgram("LightDraw");
		ld.setUniform("cameraPosition", _camera.getPosition());
		ld.use();
		glDrawArrays(GL_POINTS, 0, _scene.getPointLights().size());
		ld.useNone();
	}
};

int main(int argc, char* argv[])
{
	Test _app(argc, argv);
	_app.init();	
	_app.run();
}
