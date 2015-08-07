#include <DeferredRenderer.hpp>

#include <stb_image_write.hpp>

DeferredRenderer::DeferredRenderer(int argc, char* argv[]) :
	Application(argc, argv)
{
}

void DeferredRenderer::screen(const std::string& path) const
{
	_offscreenRender.bind(FramebufferTarget::Read);
	GLubyte* pixels = new GLubyte[4 * getInternalWidth() * getInternalHeight()];
	glReadPixels(0, 0, getInternalWidth(), getInternalHeight(), GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	stbi_write_png(path.c_str(), getInternalWidth(), getInternalHeight(), 4, pixels, 0);
	delete[] pixels;
}

void DeferredRenderer::run_init()
{
	ComputeShader& DeferredShadowCS = ResourcesManager::getInstance().getShader<ComputeShader>("DeferredShadowCS");
	DeferredShadowCS.loadFromFile("src/GLSL/Deferred/tiled_deferred_shadow_cs.glsl");
	DeferredShadowCS.compile();
	
	DeferredShadowCS.getProgram().bindUniformBlock("LightBlock", _scene.getPointLightBuffer());
		
	loadProgram("BloomBlend",
				load<VertexShader>("src/GLSL/fullscreen_vs.glsl"),
				load<FragmentShader>("src/GLSL/bloom_blend_fs.glsl"));
	
	auto& Deferred = loadProgram("Deferred",
		load<VertexShader>("src/GLSL/Deferred/deferred_vs.glsl"),
		load<FragmentShader>("src/GLSL/Deferred/deferred_fs.glsl")
	);
	
	Deferred.bindUniformBlock("Camera", _camera_buffer); 
}

void DeferredRenderer::renderGBuffer()
{
	// Fill G-Buffer
	_offscreenRender.bind();
	_offscreenRender.clear();
	
	_scene.draw(_projection, _camera.getMatrix());
	
	renderGBufferPost();

	_offscreenRender.unbind();
}

void DeferredRenderer::renderLightPass()
{
	// Light pass (Compute Shader)
	Context::viewport(0, 0, _width, _height);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	Context::clear(BufferBit::All);
	
	_offscreenRender.getColor(0).bindImage(0, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	_offscreenRender.getColor(1).bindImage(1, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
	_offscreenRender.getColor(2).bindImage(2, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	
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

void DeferredRenderer::renderPostProcess()
{
	Framebuffer<>::unbind(FramebufferTarget::Draw);
	
	// This looks really good with downsampling (but is obviously really expensive)
	/// @todo The amount of blur (i.e. its kernel) should be customizable.
	if(_postProcessBlur)
		blur(_offscreenRender.getColor(0), getInternalWidth(), getInternalHeight(), 0);
	
	if(_bloom > 0.0)
	{
		// Downsampling and blur
		_offscreenRender.getColor(2).generateMipmaps();
		_offscreenRender.getColor(2).set(Texture::Parameter::BaseLevel, _bloomDownsampling);
		for(int i = 0; i < _bloomBlur; ++i)
			blur(_offscreenRender.getColor(2), getInternalWidth(), getInternalHeight(), _bloomDownsampling);
		_offscreenRender.getColor(2).generateMipmaps();
		
		// Blend
		_offscreenRender.getColor(0).bind(0);
		_offscreenRender.getColor(2).bind(1);
		Program& BloomBlend = ResourcesManager::getInstance().getProgram("BloomBlend");
		BloomBlend.use();
		BloomBlend.setUniform("Exposure", _exposure);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); // Dummy draw call
		BloomBlend.useNone();
		
		_offscreenRender.getColor(2).set(Texture::Parameter::BaseLevel, 0);
	} else {
		// No post process, just blit.
		_offscreenRender.bind(FramebufferTarget::Read);
		glBlitFramebuffer(0, 0, getInternalWidth(), getInternalHeight(), 
							0, 0, _width, _height, 
							GL_COLOR_BUFFER_BIT, GL_LINEAR);
	}
}

void DeferredRenderer::render()
{
	renderGBuffer();
	renderLightPass();
	renderPostProcess();
	renderGUI();
}

void DeferredRenderer::setInternalResolution(size_t width, size_t height)
{
	_internalWidth = width;
	_internalHeight = height;
	if(_internalWidth == 0 || _internalHeight == 0)
	{
		initGBuffer(_width, _height);
	} else {
		initGBuffer(_internalWidth, _internalHeight);
	}
}

void DeferredRenderer::initGBuffer(size_t width, size_t height)
{
	_offscreenRender = Framebuffer<Texture2D, 3>(width, height);
	_offscreenRender.getColor(0).setPixelType(Texture::PixelType::Float);
	_offscreenRender.getColor(0).create(nullptr, width, height, GL_RGBA32F, GL_RGBA, false);
	_offscreenRender.getColor(0).set(Texture::Parameter::WrapS, GL_CLAMP_TO_EDGE);
	_offscreenRender.getColor(0).set(Texture::Parameter::WrapT, GL_CLAMP_TO_EDGE);
	_offscreenRender.getColor(1).setPixelType(Texture::PixelType::Float);
	_offscreenRender.getColor(1).create(nullptr, width, height, GL_RGBA32F, GL_RGBA, false);
	_offscreenRender.getColor(1).set(Texture::Parameter::WrapS, GL_CLAMP_TO_EDGE);
	_offscreenRender.getColor(1).set(Texture::Parameter::WrapT, GL_CLAMP_TO_EDGE);
	_offscreenRender.getColor(2).setPixelType(Texture::PixelType::Float);
	_offscreenRender.getColor(2).create(nullptr, width, height, GL_RGBA32F, GL_RGBA, false);
	_offscreenRender.getColor(2).set(Texture::Parameter::WrapS, GL_CLAMP_TO_EDGE);
	_offscreenRender.getColor(2).set(Texture::Parameter::WrapT, GL_CLAMP_TO_EDGE);
	_offscreenRender.init();
}
	
void DeferredRenderer::resize_callback(GLFWwindow* _window, int width, int height)
{
	Application::resize_callback(_window, width, height);
	
	if(_internalWidth == 0 || _internalHeight == 0)
	{
		initGBuffer(_width, _height);
	}
}

void DeferredRenderer::key_callback(GLFWwindow* _window, int key, int scancode, int action, int mods)
{
	if(action == GLFW_PRESS && !_gui.handleKey(key, scancode, action, mods))
	{
		switch(key)
		{
			case GLFW_KEY_N:
			{				
				std::cout << _camera.getPosition().x << "\t" << _camera.getPosition().y << "\t" << _camera.getPosition().z << std::endl;
				break;
			}
			case GLFW_KEY_KP_4:
			{
				if(_bloomDownsampling > 0)
					_bloomDownsampling -= 1;
				std::cout << "BloomDownsampling: " << _bloomDownsampling << std::endl;
				break;
			}
			case GLFW_KEY_KP_5:
			{
				_bloomDownsampling += 1;
				std::cout << "BloomDownsampling: " << _bloomDownsampling << std::endl;
				break;
			}
			case GLFW_KEY_ESCAPE:
			{
				glfwSetWindowShouldClose(_window, GL_TRUE);
				break;
			}
			case GLFW_KEY_R:
			{
				std::cout << "Reloading shaders..." << std::endl;
				ResourcesManager::getInstance().reloadShaders();
				std::cout << "Reloading shaders... Done !" << std::endl;
				break;
			}
			case GLFW_KEY_SPACE:
			{
				if(!_controlCamera)
				{
					glfwGetCursorPos(_window, &_mouse_x, &_mouse_y); // Avoid camera jumps
					glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				} else {
					glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
				}
				_controlCamera = !_controlCamera;
				break;
			}
			case GLFW_KEY_X:
			{
				_msaa = ! _msaa;
				if(_msaa)
				{
					glEnable(GL_MULTISAMPLE);
					
					GLint iMultiSample = 0;
					GLint iNumSamples = 0;
					glGetIntegerv(GL_SAMPLE_BUFFERS, &iMultiSample);
					glGetIntegerv(GL_SAMPLES, &iNumSamples);
					
					glfwWindowHint(GLFW_SAMPLES, iMultiSample);
					
					std::cout << "Enabled MSAA (GL_SAMPLES : " << iNumSamples << ", GL_SAMPLE_BUFFERS : " << iMultiSample << ")" << std::endl;
				} else {
					glDisable(GL_MULTISAMPLE);
					
					GLint  iMultiSample = 0;
					GLint  iNumSamples = 0;
					glGetIntegerv(GL_SAMPLE_BUFFERS, &iMultiSample);
					glGetIntegerv(GL_SAMPLES, &iNumSamples);
					std::cout << "Disabled MSAA (GL_SAMPLES : " << iNumSamples << ", GL_SAMPLE_BUFFERS : " << iMultiSample << ")" << std::endl;
				}
				break;
			}
			case GLFW_KEY_V:
			{
				_fullscreen = !_fullscreen;
				if(_fullscreen)
				{
					std::cout << "TODO: Add fullscreen :p (Sorry...)" << std::endl;
				} else {
					std::cout << "TODO: Add fullscreen :p (Sorry...)" << std::endl;
				}
				break;
			}
			case GLFW_KEY_P:
			{
				_paused = !_paused;
				break;
			}
			case GLFW_KEY_L:
			{
				const std::string ScreenPath("out/screenshot.png");
				std::cout << "Saving a screenshot to " << ScreenPath << "..." << std::endl;
				screen(ScreenPath);
				break;
			}
			case GLFW_KEY_KP_ADD:
			{
				if(_camera.speed() < 1)
					_camera.speed() += .1;
				else
					_camera.speed() += 1;
				std::cout << "Camera Speed: " << _camera.speed() << std::endl;
				break;
			}
			case GLFW_KEY_KP_SUBTRACT:
			{
				if(_camera.speed() <= 1)
					_camera.speed() -= .1;
				else
					_camera.speed() -= 1;
				std::cout << "Camera Speed: " << _camera.speed() << std::endl;
				break;
			}
		}
	}
}