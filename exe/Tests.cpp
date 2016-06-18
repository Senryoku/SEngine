#include <sstream>
#include <iomanip>
#include <Tools/range/range.hpp>

#include <Query.hpp>

#include <SpotLight.hpp>
#include <OrthographicLight.hpp>
#include <DeferredRenderer.hpp>
#include <NoisyTerrain.hpp>

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
				
		_camera.speed() = 15;
		_camera.setPosition(glm::vec3(0.0, 15.0, 0.0));
		_camera.lookAt(glm::vec3(0.0, 5.0, 0.0));

		static float R = 0.95f;
		static float F0 = 0.15f;
		Mesh& Plane = ResourcesManager::getInstance().getMesh("Plane");
		float s = 400.f;
		auto T = NoisyTerrain({5.f, 1.0f, 0.5f, 0.1f, 0.05f},
								{15.f, 4.f, 3.f, 2.f, 1.f},
								{0.125f/2.f, 0.125f, 0.25f, 0.5f, 1.f});
		Plane = create(T, glm::vec2(0.0), glm::vec2(s), glm::vec2(1200));
		for(auto& v : Plane.getVertices())
		{
			v.texcoord *= 0.3f;
		}
		Plane.setBoundingBox({glm::vec3(-s, 0.f, -s), glm::vec3(s, 100.f, s)});
		Plane.computeNormals();
		Plane.createVAO();
		Plane.getMaterial().setShadingProgram(
			ResourcesManager::getInstance().getProgram("Deferred")
		);
		Plane.getMaterial().setUniform("R", &R);
		Plane.getMaterial().setUniform("F0", &F0);
		Plane.getMaterial().setUniform("Color", glm::vec3{0.5, 0.5, 0.5});
		Plane.getMaterial().setSubroutine(ShaderType::Fragment, "colorFunction", "uniform_color");
		Plane.getMaterial().setSubroutine(ShaderType::Fragment, "normalFunction", "basic_normal");
		_scene.add(MeshInstance(Plane));
		
		auto Meshes = {
			Mesh::load("in/3DModels/sponza/sponza.obj"),
			Mesh::load("in/3DModels/dragon/Figurine Dragon N170112.3DS"),
			Mesh::load("in/3DModels/alduin/alduin.obj")
		};
		auto MeshesMat = {
			glm::scale(glm::translate(glm::mat4(1.0), glm::vec3(300.0, 0.0, 300.0)), glm::vec3(0.04)),
			glm::scale(glm::translate(glm::mat4(1.0), glm::vec3(100.0, 0.0, 100.0)), glm::vec3(0.08)),
			glm::scale(glm::translate(glm::mat4(1.0), glm::vec3(200.0, 0.0, 100.0)), glm::vec3(0.08))
		};
		for(auto m : util::lang::indices(Meshes))
			for(auto& part : Meshes.begin()[m])
			{
				part->createVAO();
				part->getMaterial().setUniform("R", &R);
				part->getMaterial().setUniform("F0", &F0);
				_scene.add(MeshInstance(*part, MeshesMat.begin()[m]));
			}
		
		// Shadow casting lights ---------------------------------------------------
		
		OrthographicLight* o = _scene.add(new OrthographicLight());
		o->init();
		o->dynamic = true;
		o->setColor(glm::vec3(1.0));
		o->setDirection(glm::normalize(glm::vec3{0.05, -1.0, 0.05}));
		o->_position = glm::vec3{0.0};
		o->updateMatrices();

		_scene.getSkybox().loadCubeMap({
			"in/Textures/skybox/posx.png",
			"in/Textures/skybox/negx.png",
			"in/Textures/skybox/posy.png",
			"in/Textures/skybox/negy.png",
			"in/Textures/skybox/posz.png",
			"in/Textures/skybox/negz.png"
		});

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
		w2->add(new GUICheckbox("Vsync", [&] {
			_vsync = !_vsync; 
			glfwSwapInterval(_vsync); 
			return _vsync;
		}));
		w2->add(new GUICheckbox("Fullscreen", [&]() -> bool {
			setFullscreen(!_fullscreen);
			return _fullscreen;
		}));
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
		w4->add(new GUIText("Lights Colors"));
	}
	
	virtual void update() override
	{
		_updateTiming.begin(Query::Target::TimeElapsed);
		if(!_paused)
		{
			if(!_scene.getLights().empty() && _scene.getLights()[0]->dynamic)
				static_cast<OrthographicLight*>(_scene.getLights()[0])->_position = 
					glm::vec3{_camera.getPosition().x, 150.0, _camera.getPosition().z};
		}
	
		DeferredRenderer::update();
		_updateTiming.end();
	}
	
	virtual void renderGBufferPost() override
	{
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
