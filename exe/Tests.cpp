#include <sstream>
#include <iomanip>
#include <deque>

#include <glm/gtx/transform.hpp>
#include <imgui.h>

#include <Query.hpp>

#include <SpotLight.hpp>
#include <OrthographicLight.hpp>
#include <DeferredRenderer.hpp>

#include <MathTools.hpp>

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
		
		auto& LightDraw = Resources::loadProgram("LightDraw",
			Resources::load<VertexShader>("src/GLSL/Debug/light_vs.glsl"),
			Resources::load<GeometryShader>("src/GLSL/Debug/light_gs.glsl"),
			Resources::load<FragmentShader>("src/GLSL/Debug/light_fs.glsl")
		);
		
		_camera.speed() = 15;
		_camera.setPosition(glm::vec3(0.0, 15.0, -20.0));
		_camera.lookAt(glm::vec3(0.0, 5.0, 0.0));
		
		LightDraw.bindUniformBlock("Camera", _camera_buffer); 

		static float R = 0.95f;
		static float F0 = 0.15f;
		const auto Paths = {
			"in/3DModels/sponza/sponza.obj",
			"in/3DModels/sibenik/sibenik.obj"
		};
		const auto Matrices = {
			glm::scale(glm::translate(glm::mat4(1.0), glm::vec3(0.0, 0.0, 0.0)), glm::vec3(0.04)),
			glm::scale(glm::translate(glm::mat4(1.0), glm::vec3(200.0, 0.0, 0.0)), glm::vec3(3.0))
		};
		for(size_t i = 0; i < Paths.size(); ++i)
		{
			auto m = Mesh::load(Paths.begin()[i]);
			for(auto& part : m)
			{
				part->createVAO();
				part->getMaterial().setUniform("R", &R);
				part->getMaterial().setUniform("F0", &F0);
				_scene.add(MeshInstance(*part, Matrices.begin()[i]));
			}
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

		_volumeSamples = 16;
		// Shadow casting lights ---------------------------------------------------
		
		OrthographicLight* o = _scene.add(new OrthographicLight());
		o->init();
		o->dynamic = false;
		o->setColor(glm::vec3(2.0));
		o->setDirection(glm::normalize(glm::vec3{58.8467 - 63.273, 161.167 - 173.158, -34.2005 - -37.1856}));
		o->_position = glm::vec3{63.273, 173.158, -37.1856};
		o->updateMatrices();
		
		OrthographicLight* o2 = _scene.add(new OrthographicLight());
		o2->init();
		o2->dynamic = false;
		o2->setColor(glm::vec3(2.0));         
		o2->setDirection(glm::normalize(glm::vec3{220.472 - 63.273, -34.6538 - 0.0, 0.789395 - 0.0}));
		o2->_position = glm::vec3{127.27, 0.0, 0.0};
		o2->updateMatrices();
		
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
		/*
		_scene.getOmniLights().resize(1);
		_scene.getOmniLights()[0].setResolution(2048);
		_scene.getOmniLights()[0].dynamic = true; /// @todo Doesn't work if not dynamic...
		_scene.getOmniLights()[0].init();
		_scene.getOmniLights()[0].setPosition(glm::vec3(-20.0, 25.0, -2.0));
		_scene.getOmniLights()[0].setColor(glm::vec3(1.5));
		_scene.getOmniLights()[0].setRange(40.0f);
		_scene.getOmniLights()[0].updateMatrices();
		*/
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
		
		/*
		auto w = _gui.add(new GUIWindow());
		w->add(new GUIGraph<float>("GUIPass (ms): ", [&]() -> float { 
			return _GUITiming.get<GLuint64>() / 1000000.0;
		}, 0.0, 3.0, 7.5));
		w->add(new GUIGraph<float>("PostProcessPass (ms): ", [&]() -> float {
			return _postProcessTiming.get<GLuint64>() / 1000000.0;
		}, 0.0, 3.0, 7.5));
		w->add(new GUIGraph<float>("LightPass (ms): ", [&]() -> float {
			return _lightPassTiming.get<GLuint64>() / 1000000.0;
		}, 0.0, 3.0, 7.5));
		w->add(new GUIGraph<float>("GBufferPass (ms): ", [&]() -> float {
			return _GBufferPassTiming.get<GLuint64>() / 1000000.0;
		}, 0.0, 3.0, 7.5));
		w->add(new GUIGraph<float>("Update (ms): ", [&]() -> float {
			return _updateTiming.get<GLuint64>() / 1000000.0;
		}, 0.0, 15.0, 7.5));
		w->add(new GUISeparator(w));
		w->add(new GUIGraph<float>("Frame Time (ms): ", [&]() -> float {
			return 1000.f * TimeManager::getInstance().getRealDeltaTime();
		}, 0.0, 20.0, 7.5));
		w->add(new GUIGraph<float>("FPS: ", [&]() -> float {
			return TimeManager::getInstance().getInstantFrameRate();
		}, 0.0, 450.0, 7.5));
		w->add(new GUIText([&]() -> std::string {
			return to_string(1000.f * TimeManager::getInstance().getRealDeltaTime(), 1) + "ms - " + 
						to_string(1.0f/TimeManager::getInstance().getRealDeltaTime(), 0) + " FPS";
		}));
		w->add(new GUISeparator(w));
		w->add(new GUIText("Stats"));
		
		auto debug_buffers = _gui.add(new GUIWindow());
		debug_buffers->add(new GUIButton("Normal", [&] { _framebufferToBlit = Attachment::Color2; }));
		debug_buffers->add(new GUIButton("Position", [&] { _framebufferToBlit = Attachment::Color1; }));
		debug_buffers->add(new GUIButton("Color", [&] { _framebufferToBlit = Attachment::Color0; }));
		debug_buffers->add(new GUICheckbox("Toggle Debug", &_debug_buffers));
		debug_buffers->add(new GUISeparator(debug_buffers));
		debug_buffers->add(new GUIText("Debug render buffers"));
		
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
		}, _vsync));
		w2->add(new GUICheckbox("Fullscreen", [&]() -> bool {
			setFullscreen(!_fullscreen);
			return _fullscreen;
		}, _fullscreen));
		w2->add(new GUIEdit<float>("AtmosphericDensity : ", &_atmosphericDensity));
		w2->add(new GUIEdit<int>("VolumeSamples : ", &_volumeSamples));
		w2->add(new GUIEdit<float>("AORadius : ", &_aoRadius));
		w2->add(new GUIEdit<float>("AOThresold : ", &_aoThreshold));
		w2->add(new GUIEdit<int>("AOSamples : ", &_aoSamples));
		w2->add(new GUIEdit<float>("MinVariance : ", &_minVariance));
		w2->add(new GUIEdit<float>("Bloom : ", &_bloom));
		w2->add(new GUIEdit<float>("Exposure : ", &_exposure));
		w2->add(new GUICheckbox("Toggle Bloom", [&]() -> bool { 
			_bloom = -_bloom; return _bloom > 0.0;
		}, _bloom > 0.0));
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
		*/
	}
	
	virtual void update() override
	{
		_updateTiming.begin(Query::Target::TimeElapsed);
		if(!_paused)
		{
			_scene.getPointLights()[3].position = glm::vec3(19.5, 5.4, 5.8) +  glm::ballRand(0.25f);
			_scene.getPointLights()[3].color = glm::vec3(0.8, 0.28, 0.2) * (4.0f + 0.75f * rand<float>());
			
			_scene.getPointLights()[4].position = glm::vec3(-24.7, 5.4, 5.8) +  glm::ballRand(0.25f);
			_scene.getPointLights()[4].color = glm::vec3(0.8, 0.28, 0.2) * (4.0f + 0.75f * rand<float>());
			
			_scene.getPointLights()[5].position = glm::vec3(-24.7, 5.4, -8.7) +  glm::ballRand(0.25f);
			_scene.getPointLights()[5].color = glm::vec3(0.8, 0.28, 0.2) * (4.0f + 0.75f * rand<float>());
			
			_scene.getPointLights()[6].position = glm::vec3(19.5, 5.4, -8.7) +  glm::ballRand(0.25f);
			_scene.getPointLights()[6].color = glm::vec3(0.8, 0.28, 0.2) * (4.0f + 0.75f * rand<float>());
		
			if(!_scene.getOmniLights().empty() && _scene.getOmniLights()[0].dynamic)
				_scene.getOmniLights()[0].setPosition(glm::vec3(-20.0 + 15.0 * cos(_time), 25.0, -2.0));
		}
	
		DeferredRenderer::update();
		_updateTiming.end();
		
		{
			// Plots
			static float last_update = 2.0;
			last_update += TimeManager::getInstance().getRealDeltaTime();
			static std::deque<float> frametimes;
			static std::deque<float> updatetimes;
			static std::deque<float> gbuffertimes;
			static std::deque<float> lighttimes;
			static std::deque<float> postprocesstimes;
			static std::deque<float> guitimes;
			float ms = TimeManager::getInstance().getRealDeltaTime() * 1000;
			if(last_update > 0.1 || frametimes.empty())
			{
				if(frametimes.size() > 50) frametimes.pop_front();
				frametimes.push_back(ms);
				if(updatetimes.size() > 50) updatetimes.pop_front();
				updatetimes.push_back(_updateTiming.get<GLuint64>() / 1000000.0);
				if(gbuffertimes.size() > 50) gbuffertimes.pop_front();
				gbuffertimes.push_back(_GBufferPassTiming.get<GLuint64>() / 1000000.0);
				if(lighttimes.size() > 50) lighttimes.pop_front();
				lighttimes.push_back(_lightPassTiming.get<GLuint64>() / 1000000.0);
				if(postprocesstimes.size() > 50) postprocesstimes.pop_front();
				postprocesstimes.push_back(_postProcessTiming.get<GLuint64>() / 1000000.0);
				if(guitimes.size() > 50) guitimes.pop_front();
				guitimes.push_back(_GUITiming.get<GLuint64>() / 1000000.0);
				last_update = 0.0;
			}
			
			ImGui::Begin("Statistics");
			ImGui::Text("%.5f ms/frame (%.1f FPS)", 
				ms, 
				1.0 / TimeManager::getInstance().getRealDeltaTime()
			);
			auto lamba_data = [](void* data, int idx) {
				return static_cast<std::deque<float>*>(data)->at(idx);
			};
			ImGui::PlotLines("FrameTime", lamba_data, &frametimes, frametimes.size(), 0, std::to_string(frametimes.back()).c_str(), 0.0, 20.0); 
			ImGui::PlotLines("Update", lamba_data, &updatetimes, updatetimes.size(), 0, std::to_string(updatetimes.back()).c_str(), 0.0, 10.0);    
			ImGui::PlotLines("GBuffer", lamba_data, &gbuffertimes, gbuffertimes.size(), 0, std::to_string(gbuffertimes.back()).c_str(), 0.0, 10.0);    
			ImGui::PlotLines("Lights", lamba_data, &lighttimes, lighttimes.size(), 0, std::to_string(lighttimes.back()).c_str(), 0.0, 10.0);    
			ImGui::PlotLines("Post Process",lamba_data, &postprocesstimes, postprocesstimes.size(), 0, std::to_string(postprocesstimes.back()).c_str(), 0.0, 10.0);    
			ImGui::PlotLines("GUI", lamba_data, &guitimes, guitimes.size(), 0, std::to_string(guitimes.back()).c_str(), 0.0, 10.0);         
			ImGui::End();
			
			ImGui::Begin("Debug");
			ImGui::Text("Hey!");
			ImGui::Button("Test Button");
			ImGui::SliderFloat("Time Scale", &_timescale, 0.0f, 5.0f);
			ImGui::End();
		}
	}
	
	virtual void renderGBufferPost() override
	{
		Context::disable(Capability::CullFace);
		auto& ld = Resources::getProgram("LightDraw");
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
