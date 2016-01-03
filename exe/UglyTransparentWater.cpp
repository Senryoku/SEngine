#include <sstream>
#include <iomanip>

#include <Tools/CubicSpline.hpp>

#include <Query.hpp>
#include <TransformFeedback.hpp>

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
		DeferredRenderer(argc, argv),
		_cameraPath{{
				glm::dvec3(50, 70, -30), 
				glm::dvec3(28, 53, -6),
				glm::dvec3(18, 23, -2),
				glm::dvec3(3, 4, -6),
				glm::dvec3(-21, 3, -5),
				glm::dvec3(-48, 4, -2),
				glm::dvec3(-48, 9, 15),
				glm::dvec3(-9, 7, 15),
				glm::dvec3(23, 6, 15),
				glm::dvec3(42, 7, 11),
				glm::dvec3(36, 11, 0),
				glm::dvec3(26, 19, -4),
				glm::dvec3(8, 24, -15),
				glm::dvec3(-6, 25, -18),
				glm::dvec3(-40, 25, -18),
				glm::dvec3(-50, 25, -1),
				glm::dvec3(-41, 25, 15),
				glm::dvec3(-7, 24, 16.5),
				glm::dvec3(20, 24, 16.5)
			}}
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
		
		ComputeShader& WaterUpdate = ResourcesManager::getInstance().getShader<ComputeShader>("WaterUpdate");
		WaterUpdate.loadFromFile("src/GLSL/Water/update_cs.glsl");
		WaterUpdate.compile();
		if(!WaterUpdate.getProgram()) exit(1);
		
		ComputeShader& WaterComputeNormals = ResourcesManager::getInstance().getShader<ComputeShader>("WaterComputeNormals");
		WaterComputeNormals.loadFromFile("src/GLSL/normals_cs.glsl");
		WaterComputeNormals.compile();
		if(!WaterComputeNormals.getProgram()) exit(1);

		Program& WaterDrawTF = loadProgram("WaterDrawTF",
			load<VertexShader>("src/GLSL/Water/draw_tf_vs.glsl"),
			load<GeometryShader>("src/GLSL/Water/draw_tf_gs.glsl"));
		WaterDrawTF.setTransformFeedbackVaryings<2>({"water", "ground"}, false);
		WaterDrawTF.link();
		if(!WaterDrawTF) exit(1);
	
		Program& WaterDraw = loadProgram("WaterDraw",
			load<VertexShader>("src/GLSL/Water/draw_indexed_vs.glsl"),
			load<FragmentShader>("src/GLSL/Water/draw_indexed_fs.glsl"));
		if(!WaterDraw) exit(1);
				
		Program& Blend = loadProgram("Blend",
			load<VertexShader>("src/GLSL/fullscreen_vs.glsl"),
			load<FragmentShader>("src/GLSL/blend_fs.glsl"));
		if(!Blend) exit(1);
	
		for(size_t i = 0; i < water_x; ++i)
			for(size_t j = 0; j < water_z; ++j)
				water.push_back(glm::vec4{
					//water_moyheight,
					water_moyheight + 1.5 * std::cos(1.0 * std::sqrt(((double) i - water_x * 0.5)*((double) i - water_x * 0.5) + ((double) j - water_z/2.0) *((double) j - water_z/2.0))), 
					0.2 + 0.5 * std::cos(1.0 * std::sqrt(((double) i - water_x * 0.5)*((double) i - water_x * 0.5) + ((double) j - water_z/2.0) *((double) j - water_z/2.0))), 
					0.0,
					0.0}); 

		water_buffer.init();
		water_buffer.bind(4);
		water_buffer.data(water.data(), sizeof(glm::vec4) * water.size(), Buffer::Usage::DynamicDraw);

		glm::mat4 WaterModelMatrix = glm::translate(glm::mat4(1.0), - glm::vec3(water_x * water_cellsize * 0.5, 0.0, water_z * water_cellsize * 0.5));
	
		WaterUpdate.getProgram().setUniform("size_x", (int) water_x);
		WaterUpdate.getProgram().setUniform("size_y", (int) water_z);
		WaterUpdate.getProgram().setUniform("cell_size", water_cellsize);
		WaterUpdate.getProgram().setUniform("moyheight", water_moyheight);
		WaterUpdate.getProgram().setUniform("iterations", 10);
		WaterUpdate.getProgram().bindShaderStorageBlock("InBuffer", water_buffer);

		WaterComputeNormals.getProgram().setUniform("size_x", (int) water_x);
		WaterComputeNormals.getProgram().setUniform("size_y", (int) water_z);

		WaterDrawTF.setUniform("size_x", (int) water_x);
		WaterDrawTF.setUniform("size_y", (int) water_z);
		WaterDrawTF.setUniform("cell_size", water_cellsize);
		WaterDrawTF.setUniform("ModelMatrix", WaterModelMatrix);

		water_transform_feedback.init();
		water_transform_feedback.bind();

		water_vertex_buffer.init(Buffer::Target::VertexAttributes);
		water_vertex_buffer.bind();
		water_vertex_buffer.data(nullptr, sizeof(glm::vec4) * water.size(), Buffer::Usage::DynamicDraw);

		water_transform_feedback.bindBuffer(0, water_vertex_buffer);
		ground_vertex_buffer.init(Buffer::Target::VertexAttributes);
		ground_vertex_buffer.bind();
		ground_vertex_buffer.data(nullptr, sizeof(glm::vec4) * water.size(), Buffer::Usage::DynamicDraw);
		water_transform_feedback.bindBuffer(1, ground_vertex_buffer);

		water_transform_feedback.unbind();

		// Normal Buffers
		water_normal_buffer.init(Buffer::Target::ShaderStorage);
		water_normal_buffer.bind();
		water_normal_buffer.data(nullptr, sizeof(glm::vec4) * water.size(), Buffer::Usage::DynamicDraw);

		ground_normal_buffer.init(Buffer::Target::ShaderStorage);
		ground_normal_buffer.bind();
		ground_normal_buffer.data(nullptr, sizeof(glm::vec4) * water.size(), Buffer::Usage::DynamicDraw);

		water_indices.init(Buffer::Target::VertexIndices);
		water_indices.bind();
		std::vector<size_t> tmp_water_indices;
		tmp_water_indices.reserve((water_x - 1) * (water_z - 1) * 3 * 2);
		for(size_t i = 0; i < water_x - 1; ++i)
		{
			for(size_t j = 0; j < water_z - 1; ++j)
			{
				tmp_water_indices.push_back(i * water_z + j);
				tmp_water_indices.push_back(i * water_z + j + 1);
				tmp_water_indices.push_back((i + 1) * water_z + j);
				
				tmp_water_indices.push_back(i * water_z + j + 1);
				tmp_water_indices.push_back((i + 1) * water_z + j + 1);
				tmp_water_indices.push_back((i + 1) * water_z + j);
			}
		}
		water_indices.data(tmp_water_indices.data(), sizeof(size_t) * tmp_water_indices.size(), Buffer::Usage::StaticDraw);
	
		_camera.speed() = 15;
		_camera.setPosition(glm::vec3(0.0, 15.0, -20.0));
		_camera.lookAt(glm::vec3(0.0, 5.0, 0.0));
		
		LightDraw.bindUniformBlock("Camera", _camera_buffer); 

		static float R = 0.95f;
		static float F0 = 0.15f;
		auto TestMesh = Mesh::load("in/3DModels/sponza/sponza.obj");
		for(auto part : TestMesh)
		{
			part->computeNormals();
			part->createVAO();
			part->getMaterial().setUniform("R", &R);
			part->getMaterial().setUniform("F0", &F0);
			_scene.add(MeshInstance(*part, glm::scale(glm::translate(glm::mat4(1.0), glm::vec3(0.0, 0.0, 0.0)), glm::vec3(0.04))));
		}

		_scene.getPointLights().push_back(PointLight{
			glm::vec3(42.8, 7.1, -1.5), 	// Position
			10.0f,
			glm::vec3(2.0), // Color
			1.0f
		});

		_scene.getPointLights().push_back(PointLight{
			glm::vec3(42.0, 23.1, 16.1), 	// Position
			15.0f,
			glm::vec3(2.0), // Color
			1.0f
		});

		_scene.getPointLights().push_back(PointLight{
			glm::vec3(-50.0, 22.8, -18.6), 	// Position
			20.0f,
			glm::vec3(2.0), // Color
			1.0f
		});
		
		_scene.getPointLights().push_back(PointLight{
			glm::vec3(19.5, 5.4, 5.8), 	// Position
			5.0f,
			glm::vec3(0.8, 0.1, 0.2), // Color
			1.0f
		});
		
		_scene.getPointLights().push_back(PointLight{
			glm::vec3(-24.7, 5.4, 5.8), 	// Position
			5.0f,
			glm::vec3(0.8, 0.1, 0.2), // Color
			1.0f
		});
		
		_scene.getPointLights().push_back(PointLight{
			glm::vec3(-24.7, 5.4, -8.7), 	// Position
			5.0f,
			glm::vec3(0.8, 0.1, 0.2), // Color
			1.0f
		});
		
		_scene.getPointLights().push_back(PointLight{
			glm::vec3(19.5, 5.4, -8.7), 	// Position
			5.0f,
			glm::vec3(0.8, 0.1, 0.2), // Color
			1.0f
		});
		
		_scene.getPointLights().push_back(PointLight{
			glm::vec3(-47.0, 4.5, -1.5), 	// Position
			20.0f,
			glm::vec3(1.8), // Color
			1.0f
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
		
		for(size_t i = 0; i < _scene.getLights().size(); ++i)
			_scene.getLights()[i]->drawShadowMap(_scene.getObjects());

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
		w5->add(new GUIEdit<float>("FoV : ", &_fov, [&] () { 
			_fov = clamp(_fov, 40.0f, 120.0f);
			setFoV(_fov);
		}));
		
		auto w2 = _gui.add(new GUIWindow());
		w2->add(new GUIButton("Print Something.", [&] { std::cout << "Something." << std::endl; }));
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
		w2->add(new GUIEdit<float>("MinVariance : ", &_minVariance));
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
		w4->add(new GUICheckbox("Draw Debug Lights", &_debugLights));
		w4->add(new GUISeparator(w4));
		w4->add(new GUIText("Lights Test"));
		
		auto w6 = _gui.add(new GUIWindow());
		w6->add(new GUIEdit<float>("Speed: ", &_cameraSpeed));
		w6->add(new GUICheckbox("Auto", &_autoCamera));
		w6->add(new GUIText("Auto Camera"));
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
		}
		
		////////////////////////////////////////////////////////////////////////////////////////////
		// Water Update
		ComputeShader& WaterUpdate = ResourcesManager::getInstance().getShader<ComputeShader>("WaterUpdate");
		WaterUpdate.use();
	 
		const int WaterInterations = 1;
		WaterUpdate.getProgram().setUniform("time", _frameTime / WaterInterations);
		for(int i = 0; i < WaterInterations; ++i)
		{
			WaterUpdate.compute(water_x / WaterUpdate.getWorkgroupSize().x + 1, water_z / WaterUpdate.getWorkgroupSize().y + 1, 1);
			WaterUpdate.memoryBarrier();
		}
		
		// Updating vertices
		TransformFeedback::disableRasterization();
		Program& WaterDrawTF = ResourcesManager::getInstance().getProgram("WaterDrawTF");
		WaterDrawTF.use();
		
		water_buffer.bind(Buffer::Target::VertexAttributes);
		water_transform_feedback.bind();
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), 0);
		
		TransformFeedback::begin(Primitive::Points);
		glDrawArrays(GL_POINTS, 0, water.size());
		TransformFeedback::end();
		
		water_transform_feedback.unbind();
		
		water_vertex_buffer.bind(Buffer::Target::ShaderStorage, 5);
		water_normal_buffer.bind(Buffer::Target::ShaderStorage, 6);
		ComputeShader& WaterComputeNormals = ResourcesManager::getInstance().getShader<ComputeShader>("WaterComputeNormals");
		WaterComputeNormals.use();
		WaterComputeNormals.compute(water_x / WaterComputeNormals.getWorkgroupSize().x + 1, water_z / WaterComputeNormals.getWorkgroupSize().y + 1, 1);
		WaterComputeNormals.memoryBarrier();
		
		ground_vertex_buffer.bind(Buffer::Target::ShaderStorage, 5);
		ground_normal_buffer.bind(Buffer::Target::ShaderStorage, 6);
		WaterComputeNormals.use();
		WaterComputeNormals.compute(water_x / WaterComputeNormals.getWorkgroupSize().x + 1, water_z / WaterComputeNormals.getWorkgroupSize().y + 1, 1);
		WaterComputeNormals.memoryBarrier();
		TransformFeedback::enableRasterization();
		
		if(_autoCamera)
		{
			float t = mod<float>(_cameraSpeed * TimeManager::getInstance().getRuntime(), _cameraPath.getPointCount());
			float t2 = mod<float>(_cameraSpeed * TimeManager::getInstance().getRuntime() + 0.1f, _cameraPath.getPointCount());
			_camera.setPosition(glm::vec3(_cameraPath(t)));
			//_camera.setDirection(glm::vec3(_cameraPath.getSpeed(t)));
			_camera.setDirection(glm::vec3(_cameraPath(t2)) - glm::vec3(_cameraPath(t)));
		}
		
		DeferredRenderer::update();
		_updateTiming.end();
	}
	
	virtual void renderGBufferPost() override
	{
		if(_debugLights)
		{
			Context::disable(Capability::CullFace);
			auto& ld = ResourcesManager::getInstance().getProgram("LightDraw");
			ld.setUniform("CameraPosition", _camera.getPosition());
			ld.use();
			glDrawArrays(GL_POINTS, 0, _scene.getPointLights().size());
			ld.useNone();
		}
	}
	
	virtual void render() override
	{
		_GBufferPassTiming.begin(Query::Target::TimeElapsed);
		renderGBuffer();
		_GBufferPassTiming.end();
		
		_lightPassTiming.begin(Query::Target::TimeElapsed);
		renderLightPass();
		_lightPassTiming.end();
		
		_GBufferTransparencyPassTiming.begin(Query::Target::TimeElapsed);
		renderTransparencyGBuffer();
		_GBufferTransparencyPassTiming.end();
		
		renderTransparencyLightPass();
		_offscreenRender.unbind();
		_offscreenRender.getColor(0).bind(0);
		_offscreenRenderTransparency.getColor(0).bind(1);
		ResourcesManager::getInstance().getProgram("Blend").use();
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		
		/// @todo Problem: Bloom won't work on water this way.
		/// @todo Opaque version of the water.
		
		_postProcessTiming.begin(Query::Target::TimeElapsed);
		renderPostProcess();
		_postProcessTiming.end();
		
		_GUITiming.begin(Query::Target::TimeElapsed);
		renderGUI();
		_GUITiming.end();
	}
	
	virtual void resize_callback(GLFWwindow* window, int width, int height) override
	{
		DeferredRenderer::resize_callback(window, width, height);
		
		_offscreenRenderTransparency = Framebuffer<Texture2D, 3>(_width, _height);
		_offscreenRenderTransparency.getColor(0).create(nullptr, _width, _height, GL_RGBA32F, GL_RGBA, false);
		_offscreenRenderTransparency.getColor(1).create(nullptr, _width, _height, GL_RGBA32F, GL_RGBA, false);
		_offscreenRenderTransparency.getColor(2).create(nullptr, _width, _height, GL_RGBA32F, GL_RGBA, false);
		_offscreenRenderTransparency.getDepth() = _offscreenRender.getDepth();
		_offscreenRenderTransparency.init();
	}
	
	void renderTransparencyGBuffer()
	{
		_offscreenRenderTransparency.bind();
		glClearColor(0.0, 0.0, 0.0, 0.0);
		_offscreenRenderTransparency.clear(BufferBit::Color); // Keep depth buffer from opaque pass
		// Water
		_waterEnvCubemap.bind(0);
		Program& WaterDraw = ResourcesManager::getInstance().getProgram("WaterDraw");
		WaterDraw.setUniform("EnvMap", 0);
		WaterDraw.setUniform("cameraPosition", _camera.getPosition());
		WaterDraw.use();
		
		water_vertex_buffer.bind();
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (const GLvoid *) 0);
		
		water_normal_buffer.bind(Buffer::Target::VertexAttributes);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (const GLvoid *) 0);
		
		water_indices.bind();
		glDrawElements(GL_TRIANGLES, (water_x - 1) * (water_z - 1) * 3 * 2, GL_UNSIGNED_INT, 0);
		_offscreenRenderTransparency.unbind();
	}
	
	void renderTransparencyLightPass()
	{
		// Light pass (Compute Shader)
		Context::viewport(0, 0, _width, _height);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		Context::clear(BufferBit::All);
		
		_offscreenRenderTransparency.getColor(0).bindImage(0, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
		_offscreenRenderTransparency.getColor(1).bindImage(1, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
		_offscreenRenderTransparency.getColor(2).bindImage(2, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
		
		size_t lc = 0;
		for(const auto& l : _scene.getLights())
			l->getShadowMap().bind(lc++ + 3);
		
		lc = 0;
		for(const auto& l : _scene.getOmniLights())
			l.getShadowMap().bind(lc++ + 13);
		
		ComputeShader& DeferredShadowCS = ResourcesManager::getInstance().getShader<ComputeShader>("DeferredShadowCS");
		DeferredShadowCS.getProgram().setUniform("ColorMaterial", (int) 0);
		DeferredShadowCS.getProgram().setUniform("PositionDepth", (int) 1);
		DeferredShadowCS.getProgram().setUniform("Normal", (int) 2);	
		
		/// @todo Move this to getLights, or something like that ?
		for(size_t i = 0; i < _scene.getLights().size(); ++i)
			DeferredShadowCS.getProgram().setUniform(std::string("ShadowMaps[").append(std::to_string(i)).append("]"), (int) i + 3);
		for(size_t i = 0; i < _scene.getOmniLights().size(); ++i)
			DeferredShadowCS.getProgram().setUniform(std::string("CubeShadowMaps[").append(std::to_string(i)).append("]"), (int) i + 13);
		DeferredShadowCS.getProgram().setUniform("ShadowCount", _scene.getLights().size());
		DeferredShadowCS.getProgram().setUniform("CubeShadowCount", _scene.getOmniLights().size());
		DeferredShadowCS.getProgram().setUniform("LightCount", _scene.getPointLights().size());
		
		DeferredShadowCS.getProgram().setUniform("CameraPosition", _camera.getPosition());
		DeferredShadowCS.getProgram().setUniform("Exposure", _exposure);
		DeferredShadowCS.getProgram().setUniform("Bloom", _bloom);
		DeferredShadowCS.getProgram().setUniform("Ambiant", _ambiant);
		DeferredShadowCS.getProgram().setUniform("MinVariance", _minVariance);
		DeferredShadowCS.getProgram().setUniform("AOSamples", _aoSamples);
		DeferredShadowCS.getProgram().setUniform("AOThreshold", _aoThreshold);
		DeferredShadowCS.getProgram().setUniform("AORadius", _aoRadius);

		DeferredShadowCS.compute(getInternalWidth() / DeferredShadowCS.getWorkgroupSize().x + 1, 
									getInternalHeight() / DeferredShadowCS.getWorkgroupSize().y + 1, 1);
		DeferredShadowCS.memoryBarrier();
	}

protected:
	Framebuffer<Texture2D, 3> _offscreenRenderTransparency;
	
	size_t water_x = 250;
	size_t water_z = 250;
	float water_cellsize = 0.5;
	float water_moyheight = 3.0;
	
		
	std::vector<glm::vec4>	water;
	ShaderStorage 			water_buffer;
	TransformFeedback 		water_transform_feedback;
	Buffer 					water_vertex_buffer;
	Buffer 					water_normal_buffer;
	Buffer 					ground_vertex_buffer;
	Buffer 					ground_normal_buffer;
	Buffer 					water_indices;
	CubeMap					_waterEnvCubemap;
	
	Query	_updateTiming;
	Query	_GBufferPassTiming;
	Query	_GBufferTransparencyPassTiming;
	Query	_lightPassTiming;
	Query	_postProcessTiming;
	Query	_GUITiming;
	
	bool	_debugLights = false;
	
	CubicSpline<glm::dvec3, double>	_cameraPath;
	bool							_autoCamera = false;
	float							_cameraSpeed = 1.0f;
};

int main(int argc, char* argv[])
{
	Test _app(argc, argv);
	_app.init();	
	_app.run();
}
