#include <sstream>
#include <iomanip>

#include <SpotLight.hpp>
#include <DeferredRenderer.hpp>

#include <Query.hpp>

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
		
		auto& LightDraw = loadProgram("LightDraw",
			load<VertexShader>("src/GLSL/Debug/light_vs.glsl"),
			load<GeometryShader>("src/GLSL/Debug/light_gs.glsl"),
			load<FragmentShader>("src/GLSL/Debug/light_fs.glsl")
		);

		_camera.speed() = 15;
		_camera.setPosition(glm::vec3(0.0, 15.0, -20.0));
		_camera.lookAt(glm::vec3(0.0, 5.0, 0.0));
		
		LightDraw.bindUniformBlock("Camera", _camera_buffer); 

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
		Plane.getMaterial().setUniform("R", 0.9f);
		Plane.getMaterial().setUniform("F0", 0.5f);

		_scene.add(MeshInstance(Plane));

		auto Model = Mesh::load("in/3DModels/dragon/Figurine Dragon N170112.3DS");
		for(auto part : Model)
		{
			part->createVAO();
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
			part->getMaterial().setUniform("R", 0.8f);
			part->getMaterial().setUniform("F0", 0.5f);
			_scene.add(MeshInstance(*part, glm::scale(glm::translate(glm::mat4(1.0), glm::vec3(150.0, 0.0, 150.0)), glm::vec3(0.04))));
		}
		
		auto Droid = Mesh::load("in/3DModels/Attack_Droid_2011/attack_droid.obj");
		for(auto part : Droid)
		{
			part->createVAO();
			part->getMaterial().setUniform("R", 0.8f);
			part->getMaterial().setUniform("F0", 0.9f);
			_scene.add(MeshInstance(*part, glm::scale(glm::translate(glm::mat4(1.0), glm::vec3(30.0, 0.0, 150.0)), glm::vec3(0.4))));
		}
		
		auto Dragon = Mesh::load("in/3DModels/alduin/alduin.obj");
		for(auto part : Dragon)
		{
			part->createVAO();
			part->getMaterial().setUniform("R", 0.8f);
			part->getMaterial().setUniform("F0", 0.5f);
			_scene.add(MeshInstance(*part, glm::scale(glm::translate(glm::mat4(1.0), glm::vec3(150.0, 0.0, 30.0)), glm::vec3(0.04))));
		}
		
		const size_t LightCount = 1000;
		LightDraw.bindUniformBlock("LightBlock", _scene.getPointLightBuffer());

		for(size_t i = 0; i < LightCount; ++i)
		{
			_scene.getPointLights().push_back(PointLight{
				glm::vec3((float) (i % 25) * 10.0f, 3.f, (float) (i / 25) * 10.0f - 5.0f), // Position
				10.0f, // Range
				2.0f * glm::vec3((i % 13) / 13.0, (i % 27) / 27.0, (i % 57) / 57.0), // Color
				1.0f		
			});
		}

		// Shadow casting lights ---------------------------------------------------

		SpotLight* spot = _scene.add(new SpotLight());
		spot->init();
		spot->setColor(glm::vec3(2.0));
		spot->setPosition(glm::vec3(50.0, 50.0, 120.0));
		spot->lookAt(glm::vec3(50.0, 0.0, 50.0));
		spot->updateMatrices();
		
		spot = _scene.add(new SpotLight());
		spot->init();
		spot->setColor(glm::vec3(0.9, 0.6, 0.6));
		spot->setPosition(glm::vec3(120.0, 50.0, 50.0));
		spot->lookAt(glm::vec3(50.0, 0.0, 50.0));
		spot->updateMatrices();
		
		spot = _scene.add(new SpotLight());
		spot->init();
		spot->setColor(glm::vec3(1.0));
		spot->setPosition(glm::vec3(30.0, 50.0, 140.0));
		spot->lookAt(glm::vec3(30.0, 0.0, 150.0));
		spot->updateMatrices();
		
		spot = _scene.add(new SpotLight());
		spot->init();
		spot->setColor(glm::vec3(1.0));
		spot->setPosition(glm::vec3(140.0, 75.0, 30.0));
		spot->lookAt(glm::vec3(150.0, 0.0, 30.0));
		spot->updateMatrices();
		
		spot = _scene.add(new SpotLight());
		spot->init();
		spot->setColor(glm::vec3(1.0));
		spot->setPosition(glm::vec3(170.0, 150.0, 170.0));
		spot->lookAt(glm::vec3(150.0, 0.0, 150.0));
		spot->updateMatrices();

		for(size_t i = 0; i < _scene.getLights().size(); ++i)
		{
			_scene.getLights()[i]->drawShadowMap(_scene.getObjects());
		}
		
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
		
		auto w2 = _gui.add(new GUIWindow());
		//w2->add(new GUIButton("Print Something.", [&] { std::cout << "Something." << std::endl; }));
		/// @todo Come back here when GLFW 3.2 will be released :)
		//w2->add(new GUICheckbox("Vsync", [&] { static int i = 0; i = (i + 1) % 2; glfwSwapInterval(i); return i == 1; }));
		//w2->add(new GUICheckbox("Fullscreen", [&] { ... }));
		w2->add(new GUIEdit<float>("AORadius : ", &_aoRadius));
		w2->add(new GUIEdit<float>("AOThresold : ", &_aoThreshold));
		w2->add(new GUIEdit<int>("AOSamples : ", &_aoSamples));
		w2->add(new GUISeparator(w2));
		w2->add(new GUIEdit<int>("BloomDownsampling : ", &_bloomDownsampling));
		w2->add(new GUIEdit<int>("BloomBlur : ", &_bloomBlur));
		w2->add(new GUIEdit<float>("Bloom : ", &_bloom));
		w2->add(new GUICheckbox("Toggle Bloom", [&]() -> bool { _bloom = -_bloom; return _bloom > 0.0; }));
		w2->add(new GUISeparator(w2));
		w2->add(new GUIEdit<float>("Exposure : ", &_exposure));
		w2->add(new GUIEdit<float>("MinVariance : ", &_minVariance));
		w2->add(new GUICheckbox("Pause", &_paused));
		w2->add(new GUISeparator(w2));
		w2->add(new GUIText("Controls"));
	}
	
	virtual void update() override
	{
		/*
		spot->setPosition(glm::vec3(150.0, 150.0, 150.0)
												+ 20.0f * glm::vec3(std::cos(0.1f * TimeManager::getInstance().getRuntime()), 0.0,
																	std::sin(0.1f * TimeManager::getInstance().getRuntime())));
		spot->lookAt(glm::vec3(150.0, 0.0, 150.0));
		spot->drawShadowMap(_scene.getObjects());
		
		blur(spot->getShadowMap(), spot->getResolution());
		*/
		for(auto& l :_scene.getPointLights())
		{
			l.position.y = 8.0 + 4.0 * std::sin(TimeManager::getInstance().getRuntime() + l.position.x * l.position.z);
		}
		
		_scene.getLights()[0]->setColor(glm::vec3(5.0f * 
			triangleWave(TimeManager::getInstance().getRuntime(), 5.0)));
		
		/// @todo Fix bug (black dots) when light position == camera position
		//_scene.getPointLights()[0].position = glm::vec4(_camera.getPosition(), 1.0);
		
		DeferredRenderer::update();
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
