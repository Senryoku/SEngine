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
		
		auto& LightDraw = loadProgram("LightDraw",
			load<VertexShader>("src/GLSL/Debug/light_vs.glsl"),
			load<GeometryShader>("src/GLSL/Debug/light_gs.glsl"),
			load<FragmentShader>("src/GLSL/Debug/light_fs.glsl")
		);
		
		ComputeShader& GaussianBlur = ResourcesManager::getInstance().getShader<ComputeShader>("GaussianBlur");
		GaussianBlur.loadFromFile("src/GLSL/gaussian_blur_h_cs.glsl");
		GaussianBlur.compile();
		ComputeShader& GaussianBlurV = ResourcesManager::getInstance().getShader<ComputeShader>("GaussianBlurV");
		GaussianBlurV.loadFromFile("src/GLSL/gaussian_blur_v_cs.glsl");
		GaussianBlurV.compile();

		_camera.speed() = 15;
		_camera.setPosition(glm::vec3(0.0, 15.0, -20.0));
		_camera.lookAt(glm::vec3(0.0, 5.0, 0.0));
		
		LightDraw.bindUniformBlock("Camera", _camera_buffer); 

		auto TestMesh = Mesh::load("in/3DModels/Test/test.obj");
		for(auto part : TestMesh)
		{
			part->createVAO();
			part->getMaterial().setUniform("k", 0.8f);
			part->getMaterial().setUniform("R", 0.8f);
			part->getMaterial().setUniform("F0", 0.9f);
			_scene.add(MeshInstance(*part, glm::scale(glm::translate(glm::mat4(1.0), glm::vec3(0.0, 0.0, 0.0)), glm::vec3(2.0))));
		}

		_scene.getPointLights().push_back(PointLight{
			glm::vec3(10.0, 10.0, -10.0), 	// Position
			10.0f,
			glm::vec3(4.0), // Color
			0.0f
		});

		_scene.getPointLights().push_back(PointLight{
			glm::vec3(10.0, 10.0, 0.0), 	// Position
			15.0f,
			glm::vec3(4.0), // Color
			0.0f
		});

		_scene.getPointLights().push_back(PointLight{
			glm::vec3(10.0, 10.0, 10.0), 	// Position
			20.0f,
			glm::vec3(4.0), // Color
			0.0f
		});
		
		LightDraw.bindUniformBlock("LightBlock", _scene.getPointLightBuffer());

		// Shadow casting lights ---------------------------------------------------

		_scene.getLights().resize(1);
		
		_scene.getLights()[0].init();
		_scene.getLights()[0].setColor(glm::vec4(2.0));
		_scene.getLights()[0].setPosition(glm::vec3(25.0, 50.0, 25.0));
		_scene.getLights()[0].lookAt(glm::vec3(0.0, 0.0, 0.0));
		_scene.getLights()[0].updateMatrices();

		for(size_t i = 0; i < _scene.getLights().size(); ++i)
			_scene.getLights()[i].drawShadowMap(_scene.getObjects());
	}
	
	virtual void in_loop_update() override
	{
	}
	
	virtual void offscreen_render(const glm::mat4& p, const glm::mat4& v) override
	{
		auto& ld = ResourcesManager::getInstance().getProgram("LightDraw");
		ld.setUniform("CameraPosition", _camera.getPosition());
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
