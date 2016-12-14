#include <DeferredRenderer.hpp>

#include <stb_image_write.hpp>

#include <Component.hpp>
#include <SpotLight.hpp>

DeferredRenderer::DeferredRenderer(int argc, char* argv[]) :
	Application(argc, argv)
{
}

void DeferredRenderer::init(const std::string& windowName)
{
	_multisampling = 0; // Can't use multisampling in deferred mode (glBlitFramebuffer)
	Application::init(windowName);
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
	using Resources::load;
	ComputeShader& DeferredShadowCS = load<ComputeShader>(
		"DeferredShadowCS",
		"src/GLSL/Deferred/tiled_deferred_shadow_cs.glsl"
	);
	DeferredShadowCS.getProgram().bindUniformBlock("LightBlock", _scene.getPointLightBuffer());
		
	Resources::loadProgram("BloomBlend",
		load<VertexShader>("src/GLSL/fullscreen_vs.glsl"),
		load<FragmentShader>("src/GLSL/bloom_blend_fs.glsl")
	);
	
	auto& Deferred = Resources::loadProgram("Deferred",
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
	for(auto& it : ComponentIterator<SpotLight>{})
		it.getShadowMap().bind(lc++ + 3);
	
	ComputeShader& DeferredShadowCS = Resources::getShader<ComputeShader>("DeferredShadowCS");
	DeferredShadowCS.getProgram().setUniform("ColorMaterial", (int) 0);
	DeferredShadowCS.getProgram().setUniform("PositionDepth", (int) 1);
	DeferredShadowCS.getProgram().setUniform("Normal", (int) 2);	
	
	/// @todo Move this to getLights, or something like that ?
	for(size_t i = 0; i < impl::components<SpotLight>.count(); ++i)
		DeferredShadowCS.getProgram().setUniform(std::string("ShadowMaps[").append(std::to_string(i)).append("]"), (int) i + 3);
	/// @todo No more omnidirectional lights. Add them again? Or maybe its better not to?
	DeferredShadowCS.getProgram().setUniform("ShadowCount", impl::components<SpotLight>.count());
	DeferredShadowCS.getProgram().setUniform("CubeShadowCount", 0);
	DeferredShadowCS.getProgram().setUniform("LightCount", _scene.getPointLights().size());
	
	DeferredShadowCS.getProgram().setUniform("CameraPosition", _camera.getPosition());
	DeferredShadowCS.getProgram().setUniform("Exposure", _exposure);
	DeferredShadowCS.getProgram().setUniform("Bloom", _bloom);
	DeferredShadowCS.getProgram().setUniform("Ambiant", _ambiant);
	DeferredShadowCS.getProgram().setUniform("MinVariance", _minVariance);
	DeferredShadowCS.getProgram().setUniform("AOSamples", _aoSamples);
	DeferredShadowCS.getProgram().setUniform("AOThreshold", _aoThreshold);
	DeferredShadowCS.getProgram().setUniform("AORadius", _aoRadius);
	DeferredShadowCS.getProgram().setUniform("AORadius", _aoRadius);
	DeferredShadowCS.getProgram().setUniform("VolumeSamples", _volumeSamples);
	DeferredShadowCS.getProgram().setUniform("AtmosphericDensity", _atmosphericDensity);

	DeferredShadowCS.compute(getInternalWidth() / DeferredShadowCS.getWorkgroupSize().x + 1, 
							getInternalHeight() / DeferredShadowCS.getWorkgroupSize().y + 1, 1);
	DeferredShadowCS.memoryBarrier();
}

void DeferredRenderer::renderPostProcess()
{
	Framebuffer<>::unbind(FramebufferTarget::Draw);
	
	if(_debug_buffers) // Blit offscreen buffers.
	{
		_offscreenRender.bind(FramebufferTarget::Read, _framebufferToBlit);
		glBlitFramebuffer(0, 0, getInternalWidth(), getInternalHeight(), 
							0, 0, _width, _height, 
							GL_COLOR_BUFFER_BIT, GL_LINEAR);
		return;
	}
	
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
		
		// Blend and display (writes directly on main framebuffer)
		_offscreenRender.getColor(0).bind(0);
		_offscreenRender.getColor(2).bind(1);
		Program& BloomBlend = Resources::getProgram("BloomBlend");
		BloomBlend.use();
		BloomBlend.setUniform("Exposure", _exposure);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); // Dummy draw call
		BloomBlend.useNone();
		
		_offscreenRender.getColor(2).set(Texture::Parameter::BaseLevel, 0);
	} else {
		// No post process, just blit the result of the light pass.
		_offscreenRender.bind(FramebufferTarget::Read);
		glBlitFramebuffer(0, 0, getInternalWidth(), getInternalHeight(), 
							0, 0, _width, _height, 
							GL_COLOR_BUFFER_BIT, GL_LINEAR);
	}
}

void DeferredRenderer::render()
{
	_GBufferPassTiming.begin(Query::Target::TimeElapsed);
	renderGBuffer();
	_GBufferPassTiming.end();
	_lastGBufferPassTiming = _GBufferPassTiming.get<GLuint64>();
	
	_lightPassTiming.begin(Query::Target::TimeElapsed);
	if(!_debug_buffers)
		renderLightPass();
	_lightPassTiming.end();
	_lastLightPassTiming = _lightPassTiming.get<GLuint64>();
	
	_postProcessTiming.begin(Query::Target::TimeElapsed);
	renderPostProcess();
	_postProcessTiming.end();
	_lastPostProcessTiming = _postProcessTiming.get<GLuint64>();
	
	_GUITiming.begin(Query::Target::TimeElapsed);
	renderGUI();
	_GUITiming.end();
	_lastGUITiming = _GUITiming.get<GLuint64>();
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
