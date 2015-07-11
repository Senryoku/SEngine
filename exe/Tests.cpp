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
		
		_camera.speed() = 15;
		_camera.setPosition(glm::vec3(0.0, 15.0, -20.0));
		_camera.lookAt(glm::vec3(0.0, 5.0, 0.0));
		
		LightDraw.bindUniformBlock("Camera", _camera_buffer); 

		auto TestMesh = Mesh::load("in/3DModels/sponza/sponza.obj");
		for(auto part : TestMesh)
		{
			part->createVAO();
			part->getMaterial().setUniform("R", 0.95f);
			part->getMaterial().setUniform("F0", 0.15f);
			_scene.add(MeshInstance(*part, glm::scale(glm::translate(glm::mat4(1.0), glm::vec3(0.0, 0.0, 0.0)), glm::vec3(0.04))));
		}
		
		_scene.getPointLights().push_back(PointLight{
			glm::vec3(42.8, 7.1, -1.5), 	// Position
			10.0f,
			glm::vec3(2.0), // Color
			0.0f
		});

		_scene.getPointLights().push_back(PointLight{
			glm::vec3(42.0, 23.1, 16.1), 	// Position
			15.0f,
			glm::vec3(2.0), // Color
			0.0f
		});

		_scene.getPointLights().push_back(PointLight{
			glm::vec3(-50.0, 22.8, -18.6), 	// Position
			20.0f,
			glm::vec3(2.0), // Color
			0.0f
		});
		
		_scene.getPointLights().push_back(PointLight{
			glm::vec3(19.5, 5.4, 5.8), 	// Position
			5.0f,
			glm::vec3(0.8, 0.1, 0.2), // Color
			0.0f
		});
		
		_scene.getPointLights().push_back(PointLight{
			glm::vec3(-24.7, 5.4, 5.8), 	// Position
			5.0f,
			glm::vec3(0.8, 0.1, 0.2), // Color
			0.0f
		});
		
		_scene.getPointLights().push_back(PointLight{
			glm::vec3(-24.7, 5.4, -8.7), 	// Position
			5.0f,
			glm::vec3(0.8, 0.1, 0.2), // Color
			0.0f
		});
		
		_scene.getPointLights().push_back(PointLight{
			glm::vec3(19.5, 5.4, -8.7), 	// Position
			5.0f,
			glm::vec3(0.8, 0.1, 0.2), // Color
			0.0f
		});
		
		_scene.getPointLights().push_back(PointLight{
			glm::vec3(-47.0, 4.5, -1.5), 	// Position
			20.0f,
			glm::vec3(1.8), // Color
			0.0f
		});
		
		LightDraw.bindUniformBlock("LightBlock", _scene.getPointLightBuffer());

		// Shadow casting lights ---------------------------------------------------

		_scene.getLights().resize(2);
		
		_scene.getLights()[0].init();
		_scene.getLights()[0].setColor(glm::vec4(2.0));
		_scene.getLights()[0].setPosition(glm::vec3(45.0, 85.0, -20.0));
		_scene.getLights()[0].lookAt(glm::vec3(0.0, 0.0, 0.0));
		_scene.getLights()[0].setRange(150.0f);
		_scene.getLights()[0].setAngle(3.14159f * 0.5f);
		_scene.getLights()[0].updateMatrices();
		
		_scene.getLights()[1].init();
		_scene.getLights()[1].setColor(glm::vec4(1.5));
		_scene.getLights()[1].setPosition(glm::vec3(45.0, 12.0, -18.0));
		_scene.getLights()[1].lookAt(glm::vec3(45.0, 0.0, -18.1));
		_scene.getLights()[1].setRange(20.0f);
		_scene.getLights()[1].setAngle(3.14159f * 0.5f);
		_scene.getLights()[1].updateMatrices();
		

		for(size_t i = 0; i < _scene.getLights().size(); ++i)
			_scene.getLights()[i].drawShadowMap(_scene.getObjects());
		
		_scene.getSkybox().loadCubeMap({"in/Textures/skybox/posx.png",
				"in/Textures/skybox/negx.png",
				"in/Textures/skybox/posy.png",
				"in/Textures/skybox/negy.png",
				"in/Textures/skybox/posz.png",
				"in/Textures/skybox/negz.png"});
	}
	
	virtual void in_loop_update() override
	{
		_scene.getPointLights()[3].position = glm::vec3(19.5, 5.4, 5.8) +  0.2f * glm::vec3(rand<float>(), rand<float>(), rand<float>());
		_scene.getPointLights()[3].color = glm::vec3(0.8, 0.28, 0.2) * (4.0f + 0.75f * rand<float>());
		
		_scene.getPointLights()[4].position = glm::vec3(-24.7, 5.4, 5.8) +  0.2f * glm::vec3(rand<float>(), rand<float>(), rand<float>());
		_scene.getPointLights()[4].color = glm::vec3(0.8, 0.28, 0.2) * (4.0f + 0.75f * rand<float>());
		
		_scene.getPointLights()[5].position = glm::vec3(-24.7, 5.4, -8.7) +  0.2f * glm::vec3(rand<float>(), rand<float>(), rand<float>());
		_scene.getPointLights()[5].color = glm::vec3(0.8, 0.28, 0.2) * (4.0f + 0.75f * rand<float>());
		
		_scene.getPointLights()[6].position = glm::vec3(19.5, 5.4, -8.7) +  0.2f * glm::vec3(rand<float>(), rand<float>(), rand<float>());
		_scene.getPointLights()[6].color = glm::vec3(0.8, 0.28, 0.2) * (4.0f + 0.75f * rand<float>());
	}
	
	virtual void offscreen_render(const glm::mat4& p, const glm::mat4& v) override
	{
		glDisable(GL_CULL_FACE);
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
