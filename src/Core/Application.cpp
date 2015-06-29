#include <Application.hpp>

#include <functional>

#include <stb_image_write.hpp>
#include <glm/gtx/transform.hpp>

// TEMP
struct CameraStruct
{
	glm::mat4	view;
	glm::mat4	projection;
};

Application::Application() :
	_resolution(_width, _height, 0.0)
{
}

Application::~Application()
{
	clean();
}

void Application::init(const std::string& windowName)
{
	// Window and Context creation
	if (glfwInit() == false)
	{
		std::cerr << "Error: couldn't initialize GLFW." << std::endl;
		exit(EXIT_FAILURE);
	}
	glfwWindowHint(GLFW_SAMPLES, 4);

	_window = glfwCreateWindow(_width, _height, windowName.c_str(), nullptr, nullptr);
	
	if (!_window)
	{
		std::cerr << "Error: couldn't create _window." << std::endl;
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(_window);
	glfwSwapInterval(0);
	
	if(gl3wInit())
	{
		std::cerr << "Error: couldn't initialize gl3w." << std::endl;
		exit(EXIT_FAILURE);
	}
	
	// Callback setting
	glfwSetErrorCallback(s_error_callback);
	glfwSetKeyCallback(_window, s_key_callback);
	glfwSetMouseButtonCallback(_window, s_mouse_button_callback);
	glfwSetCursorPosCallback(_window, s_mouse_position_callback);
	glfwSetWindowSizeCallback(_window, s_resize_callback);
	
	glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	
	glEnable(GL_DEPTH_TEST);
	
	_scene.init();
	
	_camera_buffer.init();
	_camera_buffer.bind(0);
}

void Application::clean()
{
	glfwDestroyWindow(_window);
}

void Application::run_init()
{
	ComputeShader& DeferredShadowCS = ResourcesManager::getInstance().getShader<ComputeShader>("DeferredShadowCS");
	DeferredShadowCS.loadFromFile("src/GLSL/Deferred/tiled_deferred_shadow_cs.glsl");
	DeferredShadowCS.compile();
}

void Application::in_loop_update()
{
}

void Application::in_loop_timing()
{
	TimeManager::getInstance().update();
	_frameTime = TimeManager::getInstance().getRealDeltaTime();
	_frameRate = TimeManager::getInstance().getInstantFrameRate();
	if(!_paused)
	{
		_time += _timescale * _frameTime;
		_frameTime *= _timescale;
		if(_frameTime > 1.0/60.0) _frameTime = 1.0/60.0; // In case the _window is moved
	} else _frameTime = 0.0;
}

void Application::in_loop_fps_camera()
{
	if(_controlCamera)
	{
		float _frameTime = TimeManager::getInstance().getRealDeltaTime(); // Should move even on pause :)
		if(glfwGetKey(_window, GLFW_KEY_W) == GLFW_PRESS)
			_camera.moveForward(_frameTime);
			
		if(glfwGetKey(_window, GLFW_KEY_A) == GLFW_PRESS)
			_camera.strafeLeft(_frameTime);
				
		if(glfwGetKey(_window, GLFW_KEY_S) == GLFW_PRESS)
			_camera.moveBackward(_frameTime);
				
		if(glfwGetKey(_window, GLFW_KEY_D) == GLFW_PRESS)
			_camera.strafeRight(_frameTime);
				
		if(glfwGetKey(_window, GLFW_KEY_Q) == GLFW_PRESS)
			_camera.moveDown(_frameTime);
				
		if(glfwGetKey(_window, GLFW_KEY_E) == GLFW_PRESS)
			_camera.moveUp(_frameTime);
			
		double mx = _mouse_x, my = _mouse_y;
		glfwGetCursorPos(_window, &_mouse_x, &_mouse_y);
		if(_mouse_x != mx || _mouse_y != my)
			_camera.look(glm::vec2(_mouse_x - mx, my - _mouse_y));
	}
	_camera.updateView();
	CameraStruct CamS = {_camera.getMatrix(), _projection};
	_camera_buffer.data(&CamS, sizeof(CameraStruct), Buffer::Usage::DynamicDraw);
}

void Application::in_loop_render()
{
	_offscreenRender.bind();
	_offscreenRender.clear();
	
	_scene.draw(_projection, _camera.getMatrix());

	_offscreenRender.unbind();
	
	glViewport(0, 0, _width, _height);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	_offscreenRender.getColor(0).bindImage(0, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	_offscreenRender.getColor(1).bindImage(1, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
	_offscreenRender.getColor(2).bindImage(2, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
	
	size_t lc = 0;
	for(const auto& l : _scene.getLights())
		l.getShadowMap().bind(lc++ + 3);
	
	ComputeShader& DeferredShadowCS = ResourcesManager::getInstance().getShader<ComputeShader>("DeferredShadowCS");
	DeferredShadowCS.getProgram().setUniform("ColorMaterial", (int) 0);
	DeferredShadowCS.getProgram().setUniform("PositionDepth", (int) 1);
	DeferredShadowCS.getProgram().setUniform("Normal", (int) 2);	
	
	DeferredShadowCS.getProgram().setUniform("cameraPosition", _camera.getPosition());

	DeferredShadowCS.compute(_resolution.x / DeferredShadowCS.getWorkgroupSize().x + 1, _resolution.y / DeferredShadowCS.getWorkgroupSize().y + 1, 1);
	DeferredShadowCS.memoryBarrier();

		
	Framebuffer<>::unbind(FramebufferTarget::Draw);
	_offscreenRender.bind(FramebufferTarget::Read);
	glBlitFramebuffer(0, 0, _resolution.x, _resolution.y, 0, 0, _resolution.x, _resolution.y, GL_COLOR_BUFFER_BIT, GL_LINEAR);
}

void Application::run()
{
	run_init();
	
	resize_callback(_window, _width, _height);
	
	while(!glfwWindowShouldClose(_window))
	{ 
		in_loop_timing();
		glfwSetWindowTitle(_window, std::string("NamelessEngine2 - ")
										.append(std::to_string(1000.f * TimeManager::getInstance().getRealDeltaTime()))
										.append("ms - FPS: ")
										.append(std::to_string(1.0f/TimeManager::getInstance().getRealDeltaTime()))
									.c_str());
		
		in_loop_fps_camera();
		
		in_loop_update();
		
		in_loop_render();

		glfwSwapBuffers(_window);
		glfwPollEvents();
	}
}

void Application::screen(const std::string& path) const
{
	GLubyte* pixels = new GLubyte[4 * _width * _height];

	glReadPixels(0, 0, _width, _height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	
	stbi_write_png(path.c_str(), _width, _height, 4, pixels, 0);
	
	delete[] pixels;
}

void Application::error_callback(int error, const char* description)
{
	std::cerr << "GLFW Error (" << error << "): " << description << std::endl;
}

void Application::resize_callback(GLFWwindow* _window, int width, int height)
{
	_width = width;
	_height = height;

	glViewport(0, 0, _width, _height);
	_resolution = glm::vec3(_width, _height, 0.0);
	
	float inRad = _fov * glm::pi<float>()/180.f;
	_projection = glm::perspective(inRad, (float) _width/_height, 0.1f, 1000.0f);
	
	_offscreenRender = Framebuffer<Texture2D, 3>(_width, _height);
	_offscreenRender.getColor(0).create(nullptr, _width, _height, GL_RGBA32F, GL_RGBA, false);
	_offscreenRender.getColor(1).create(nullptr, _width, _height, GL_RGBA32F, GL_RGBA, false);
	_offscreenRender.getColor(2).create(nullptr, _width, _height, GL_RGBA32F, GL_RGBA, false);
	_offscreenRender.init();
	
	std::cout << "Reshaped to " << width << "*" << height  << " (" << ((GLfloat) _width)/_height << ")" << std::endl;
}

void Application::key_callback(GLFWwindow* _window, int key, int scancode, int action, int mods)
{
	if(action == GLFW_PRESS)
	{
		switch(key)
		{
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

void Application::mouse_button_callback(GLFWwindow* _window, int button, int action, int mods)
{	
	float z = _mouse.z;
	float w = _mouse.w;
	if(button == GLFW_MOUSE_BUTTON_1)
	{
		if(action == GLFW_PRESS)
		{
			z = 1.0;
		} else {
			z = 0.0;
		}
	} else if(button == GLFW_MOUSE_BUTTON_2) {
		if(action == GLFW_PRESS)
		{
			w = 1.0;
		} else {
			w = 0.0;
		}
	}
	
	_mouse = glm::vec4(_mouse.x, _mouse.y, z, w);
}

void Application::mouse_position_callback(GLFWwindow* _window, double xpos, double ypos)
{
	_mouse = glm::vec4(xpos, ypos, _mouse.z, _mouse.w);
}
