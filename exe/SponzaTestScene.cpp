#include <sstream>
#include <iomanip>

#include <Query.hpp>

#include <SpotLight.hpp>
#include <OrthographicLight.hpp>
#include <DeferredRenderer.hpp>

#include <MathTools.hpp>
#include <GUIText.hpp>
#include <GUIButton.hpp>
#include <GUICheckbox.hpp>
#include <GUIEdit.hpp>
#include <GUIGraph.hpp>
#include <GUISeparator.hpp>

#include <glm/gtx/transform.hpp>

template <typename T>
std::string to_string(const T a_value, const int n = 6)
{
    std::ostringstream out;
	out << std::fixed;
    out << std::setprecision(n) << a_value;
    return out.str();
}

class Test : public DeferredRenderer
{
public:
	Test(int argc, char* argv[]) : 
		DeferredRenderer(argc, argv)
	{
	}
	
	virtual void run_init() override
	{
		DeferredRenderer::run_init();
		
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
		
		OrthographicLight* o = _scene.add(new OrthographicLight());
		o->init();
		o->Dynamic = false;
		o->setColor(glm::vec3(2.0));
		o->setDirection(glm::normalize(glm::vec3{58.8467 - 63.273, 161.167 - 173.158, -34.2005 - -37.1856}));
		o->_position = glm::vec3{63.273, 173.158, -37.1856};
		o->updateMatrices();
		
		SpotLight* s = _scene.add(new SpotLight());
		s->init();
		s->setColor(glm::vec3(1.5));
		s->setPosition(glm::vec3(45.0, 12.0, -18.0));
		s->lookAt(glm::vec3(45.0, 0.0, -18.0));
		s->setRange(20.0f);
		s->setAngle(3.14159f * 0.5f);
		s->updateMatrices();
		
		s = _scene.add(new SpotLight());
		s->init();
		s->setColor(glm::vec3(1.5));
		s->setPosition(glm::vec3(0.0, 20.0, 00.0));
		s->lookAt(glm::vec3(0.0, 0.0, 0.0));
		s->setRange(50.0f);
		s->setAngle(3.14159f * 0.5f);
		s->updateMatrices();
		
		_scene.getOmniLights().resize(1);
		_scene.getOmniLights()[0].setResolution(2048);
		_scene.getOmniLights()[0].dynamic = true; /// @todo Doesn't work if not dynamic...
		_scene.getOmniLights()[0].init();
		_scene.getOmniLights()[0].setPosition(glm::vec3(-20.0, 25.0, -2.0));
		_scene.getOmniLights()[0].setColor(glm::vec3(1.5));
		_scene.getOmniLights()[0].setRange(40.0f);
		_scene.getOmniLights()[0].updateMatrices();
		
		for(size_t i = 0; i < _scene.getLights().size(); ++i)
			_scene.getLights()[i]->drawShadowMap(_scene.getObjects());
		
		for(size_t i = 0; i < _scene.getOmniLights().size(); ++i)
			_scene.getOmniLights()[i].drawShadowMap(_scene.getObjects());

		_scene.getSkybox().loadCubeMap({"in/Textures/skybox/posx.png",
				"in/Textures/skybox/negx.png",
				"in/Textures/skybox/posy.png",
				"in/Textures/skybox/negy.png",
				"in/Textures/skybox/posz.png",
				"in/Textures/skybox/negz.png"});
		
		auto w = _gui.add(new GUIWindow());
		w->add(new GUIGraph<float>("GUIPass (ms): ", [&]() -> float { return _GUITiming.get<GLuint64>() / 1000000.0; }, 0.0, 3.0, 7.5));
		w->add(new GUIGraph<float>("PostProcessPass (ms): ", [&]() -> float { return _postProcessTiming.get<GLuint64>() / 1000000.0; }, 0.0, 3.0, 7.5));
		w->add(new GUIGraph<float>("LightPass (ms): ", [&]() -> float { return _lightPassTiming.get<GLuint64>() / 1000000.0; }, 0.0, 3.0, 7.5));
		w->add(new GUIGraph<float>("GBufferPass (ms): ", [&]() -> float { return _GBufferPassTiming.get<GLuint64>() / 1000000.0; }, 0.0, 3.0, 7.5));
		w->add(new GUIGraph<float>("Update (ms): ", [&]() -> float { return _updateTiming.get<GLuint64>() / 1000000.0; }, 0.0, 15.0, 7.5));
		w->add(new GUISeparator(w));
		w->add(new GUIGraph<float>("Frame Time (ms): ", [&]() -> float { return 1000.f * TimeManager::getInstance().getRealDeltaTime(); }, 0.0, 20.0, 7.5));
		w->add(new GUIGraph<float>("FPS: ", [&]() -> float { return TimeManager::getInstance().getInstantFrameRate(); }, 0.0, 450.0, 7.5));
		w->add(new GUIText([&]() -> std::string {
			return to_string(1000.f * TimeManager::getInstance().getRealDeltaTime(), 1) + "ms - " + 
						to_string(1.0f/TimeManager::getInstance().getRealDeltaTime(), 0) + " FPS";
		}));
		w->add(new GUISeparator(w));
		w->add(new GUIText("Stats"));
		
		auto w5 = _gui.add(new GUIWindow());
		w5->add(new GUIButton("3840*2160", [&] {setInternalResolution(3840, 2160);}));
		w5->add(new GUIButton("2715*1527", [&] {setInternalResolution(2715, 1527);}));
		w5->add(new GUIButton("1920*1080", [&] {setInternalResolution(1920, 1080);}));
		w5->add(new GUIButton("Windows resolution", [&] {setInternalResolution(0, 0);}));
		w5->add(new GUISeparator(w5));
		w5->add(new GUIText("Internal Resolution"));
		
		auto w2 = _gui.add(new GUIWindow());
		w2->add(new GUIButton("Print Something.", [&] { std::cout << "Something." << std::endl; }));
		/// @todo Come back here when GLFW 3.2 will be released :)
		//w2->add(new GUICheckbox("Vsync", [&] { static int i = 0; i = (i + 1) % 2; glfwSwapInterval(i); return i == 1; }));
		//w2->add(new GUICheckbox("Fullscreen", [&] { ... }));
		w2->add(new GUIEdit<float>("AtmosphericDensity : ", &_atmosphericDensity));
		w2->add(new GUIEdit<int>("VolumeSamples : ", &_volumeSamples));
		w2->add(new GUIEdit<float>("AORadius : ", &_aoRadius));
		w2->add(new GUIEdit<float>("AOThresold : ", &_aoThreshold));
		w2->add(new GUIEdit<int>("AOSamples : ", &_aoSamples));
		w2->add(new GUIEdit<float>("MinVariance : ", &_minVariance));
		w2->add(new GUIEdit<float>("Bloom : ", &_bloom));
		w2->add(new GUIEdit<float>("Exposure : ", &_exposure));
		w2->add(new GUICheckbox("Toggle Bloom", [&]() -> bool { _bloom = -_bloom; return _bloom > 0.0; }));
		w2->add(new GUICheckbox("Pause", &_paused));
		w2->add(new GUISeparator(w2));
		w2->add(new GUIText("Controls"));
		
		auto w3 = _gui.add(new GUIWindow());
		w3->add(new GUIEdit<float>("Fresnel Reflectance : ", &F0));
		w3->add(new GUIEdit<float>("Roughness : ", &R));
		w3->add(new GUISeparator(w3));
		w3->add(new GUIText("Material Test"));
		
		auto w4 = _gui.add(new GUIWindow());
		if(!_scene.getLights().empty())
		{
			w4->add(new GUIEdit<float>("L0 Color B: ", &(_scene.getLights()[0]->getColor().b)));
			w4->add(new GUIEdit<float>("L0 Color G: ", &(_scene.getLights()[0]->getColor().g)));
			w4->add(new GUIEdit<float>("L0 Color R: ", &(_scene.getLights()[0]->getColor().r)));
		}
		w4->add(new GUIEdit<float>("Ambiant Color B: ", &_ambiant.b));
		w4->add(new GUIEdit<float>("Ambiant Color G: ", &_ambiant.g));
		w4->add(new GUIEdit<float>("Ambiant Color R: ", &_ambiant.r));
		w4->add(new GUISeparator(w4));
		w4->add(new GUIText("Lights Test"));
	}
	
	virtual void update() override
	{
		_updateTiming.begin(Query::Target::TimeElapsed);
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
		
			if(!_scene.getOmniLights().empty() && _scene.getOmniLights()[0].dynamic)
				_scene.getOmniLights()[0].setPosition(glm::vec3(-20.0 + 15.0 * cos(_time), 25.0, -2.0));
		}
	
		DeferredRenderer::update();
		_updateTiming.end();
	}
	
	virtual void renderGBufferPost() override
	{
		Context::disable(Capability::CullFace);
		auto& ld = ResourcesManager::getInstance().getProgram("LightDraw");
		ld.setUniform("CameraPosition", _camera.getPosition());
		ld.use();
		glDrawArrays(GL_POINTS, 0, _scene.getPointLights().size());
		ld.useNone();
	}
	
	virtual void render() override
	{
		_GBufferPassTiming.begin(Query::Target::TimeElapsed);
		renderGBuffer();
		_GBufferPassTiming.end();
		
		_lightPassTiming.begin(Query::Target::TimeElapsed);
		renderLightPass();
		_lightPassTiming.end();
		
		_postProcessTiming.begin(Query::Target::TimeElapsed);
		renderPostProcess();
		_postProcessTiming.end();
		
		_GUITiming.begin(Query::Target::TimeElapsed);
		renderGUI();
		_GUITiming.end();
	}
	
protected:
	Query	_updateTiming;
	Query	_GBufferPassTiming;
	Query	_lightPassTiming;
	Query	_postProcessTiming;
	Query	_GUITiming;
};

int main(int argc, char* argv[])
{
	Test _app(argc, argv);
	_app.init();	
	_app.run();
}
