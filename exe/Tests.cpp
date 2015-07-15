#include <sstream>
#include <iomanip>

#include <Application.hpp>
#include <NoisyTerrain.hpp>
#include <MathTools.hpp>
#include <GUIText.hpp>
#include <GUIButton.hpp>
#include <GUICheckbox.hpp>
#include <GUIEdit.hpp>

#include <glm/gtx/transform.hpp>

template <typename T>
std::string to_string(const T a_value, const int n = 6)
{
    std::ostringstream out;
	out << std::fixed;
    out << std::setprecision(n) << a_value;
    return out.str();
}

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

		static float R = 0.95f;
		static float F0 = 0.15f;
		auto TestMesh = Mesh::load("in/3DModels/sponza/sponza.obj");
		for(auto part : TestMesh)
		{
			part->createVAO();
			part->getMaterial().setUniform("R", &R);
			part->getMaterial().setUniform("F0", &F0);
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

		_scene.getLights().resize(3 + 6);
		
		_scene.getLights()[0].init();
		_scene.getLights()[0].setColor(glm::vec3(2.0));
		_scene.getLights()[0].setPosition(glm::vec3(45.0, 85.0, -20.0));
		_scene.getLights()[0].lookAt(glm::vec3(0.0, 0.0, 0.0));
		_scene.getLights()[0].setRange(150.0f);
		_scene.getLights()[0].setAngle(3.14159f * 0.5f);
		_scene.getLights()[0].updateMatrices();
		
		_scene.getLights()[1].init();
		_scene.getLights()[1].setColor(glm::vec3(1.5));
		_scene.getLights()[1].setPosition(glm::vec3(45.0, 12.0, -18.0));
		_scene.getLights()[1].lookAt(glm::vec3(45.0, 0.0, -18.0));
		_scene.getLights()[1].setRange(20.0f);
		_scene.getLights()[1].setAngle(3.14159f * 0.5f);
		_scene.getLights()[1].updateMatrices();
		
		_scene.getLights()[2].init();
		_scene.getLights()[2].setColor(glm::vec3(1.5));
		_scene.getLights()[2].setPosition(glm::vec3(0.0, 20.0, 00.0));
		_scene.getLights()[2].lookAt(glm::vec3(0.0, 0.0, 0.0));
		_scene.getLights()[2].setRange(50.0f);
		_scene.getLights()[2].setAngle(3.14159f * 0.5f);
		_scene.getLights()[2].updateMatrices();
		
		_scene.getOmniLights().resize(1);
		_scene.getOmniLights()[0].setResolution(2048);
		_scene.getOmniLights()[0].dynamic = true;
		_scene.getOmniLights()[0].init();
		_scene.getOmniLights()[0].setColor(glm::vec3(1.5));
		_scene.getOmniLights()[0].setRange(40.0f);
		
		for(size_t i = 0; i < _scene.getLights().size(); ++i)
			_scene.getLights()[i].drawShadowMap(_scene.getObjects());
		
		for(size_t i = 0; i < _scene.getOmniLights().size(); ++i)
			_scene.getOmniLights()[i].drawShadowMap(_scene.getObjects());

		_scene.getSkybox().loadCubeMap({"in/Textures/skybox/posx.png",
				"in/Textures/skybox/negx.png",
				"in/Textures/skybox/posy.png",
				"in/Textures/skybox/negy.png",
				"in/Textures/skybox/posz.png",
				"in/Textures/skybox/negz.png"});
		
		auto w = _gui.add(new GUIWindow());
		w->add(new GUIText([&]() -> std::string {
			return to_string(1000.f * TimeManager::getInstance().getRealDeltaTime(), 1) + "ms - " + 
						to_string(1.0f/TimeManager::getInstance().getRealDeltaTime(), 0) + " FPS";
		}));
		w->add(new GUIText("=================="));
		w->add(new GUIText("Stats"));
		
		auto w2 = _gui.add(new GUIWindow());
		w2->add(new GUIButton("Print Something.", [&] { std::cout << "Something." << std::endl; }));
		/// @todo Come back here when GLFW 3.2 will be released :)
		//w2->add(new GUICheckbox("Vsync", [&] { static int i = 0; i = (i + 1) % 2; glfwSwapInterval(i); return i == 1; }));
		//w2->add(new GUICheckbox("Fullscreen", [&] { ... }));
		w2->add(new GUIEdit<float>("AORadius : ", &_aoRadius));
		w2->add(new GUIEdit<float>("AOThresold : ", &_aoThreshold));
		w2->add(new GUIEdit<int>("AOSamples : ", &_aoSamples));
		w2->add(new GUIEdit<float>("MinVariance : ", &_minVariance));
		w2->add(new GUIEdit<float>("Bloom : ", &_bloom));
		w2->add(new GUICheckbox("Toggle Bloom", [&]() -> bool { _bloom = -_bloom; return _bloom > 0.0; }));
		w2->add(new GUICheckbox("Pause", &_paused));
		w2->add(new GUIText("=================="));
		w2->add(new GUIText("Controls"));
		
		auto w3 = _gui.add(new GUIWindow());
		w3->add(new GUIEdit<float>("Fresnel Reflectance : ", &F0));
		w3->add(new GUIEdit<float>("Roughness : ", &R));
		w3->add(new GUIText("=================="));
		w3->add(new GUIText("Material Test"));
		
		auto w4 = _gui.add(new GUIWindow());
		w4->add(new GUIEdit<float>("L0 Color B: ", &(_scene.getLights()[0].getColor().b)));
		w4->add(new GUIEdit<float>("L0 Color G: ", &(_scene.getLights()[0].getColor().g)));
		w4->add(new GUIEdit<float>("L0 Color R: ", &(_scene.getLights()[0].getColor().r)));
		w4->add(new GUIEdit<float>("Ambiant Color B: ", &_ambiant.b));
		w4->add(new GUIEdit<float>("Ambiant Color G: ", &_ambiant.g));
		w4->add(new GUIEdit<float>("Ambiant Color R: ", &_ambiant.r));
		w4->add(new GUIText("=================="));
		w4->add(new GUIText("Lights Test"));
	}
	
	virtual void in_loop_update() override
	{
		if(!_paused)
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
		
		_scene.getOmniLights()[0].setPosition(glm::vec3(-20.0 + 15.0 * cos(_time), 25.0, -2.0));
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
