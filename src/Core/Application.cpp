#include <Application.hpp>

#include <functional>

#include <stb_image_write.hpp>
#include <glm/gtx/transform.hpp>

Application* Application::s_instance = nullptr;

Application::Application() :
	_resolution(_width, _height, 0.0)
{
	if(s_instance != nullptr)
		std::cerr << "Warning: You should have only one Application running at a time! Some callbacks will not work properly." << std::endl;
	s_instance = this;
}

Application::Application(int argc, char* argv[])
{
	if(s_instance != nullptr)
		std::cerr << "Warning: You should have only one Application running at a time! Some callbacks will not work properly." << std::endl;
	s_instance = this;
	
	if(argc > 1)
		_fullscreen = true;
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

	if(_fullscreen)
	{
		auto monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		_width = mode->width;
		_height = mode->height;
		_window = glfwCreateWindow(_width, _height, windowName.c_str(), monitor, nullptr);
	} else {
		_window = glfwCreateWindow(_width, _height, windowName.c_str(), nullptr, nullptr);
	}
	
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
	glfwSetCharCallback(_window, s_char_callback);
	glfwSetMouseButtonCallback(_window, s_mouse_button_callback);
	glfwSetCursorPosCallback(_window, s_mouse_position_callback);
	glfwSetWindowSizeCallback(_window, s_resize_callback);
	glfwSetDropCallback(_window, s_drop_callback);
	
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
}

void Application::update()
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

	glfwSetWindowTitle(_window, std::string("NamelessEngine2 - ")
									.append(std::to_string(1000.f * TimeManager::getInstance().getRealDeltaTime()))
									.append("ms - FPS: ")
									.append(std::to_string(1.0f/TimeManager::getInstance().getRealDeltaTime()))
								.c_str());

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
	_gpuCameraData = {_camera.getMatrix(), _projection};
	_camera_buffer.data(&_gpuCameraData, sizeof(GPUViewProjection), Buffer::Usage::DynamicDraw);
}

void Application::renderGUI()
{
	Context::disable(Capability::DepthTest);
	Context::enable(Capability::Blend);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	_gui.draw(glm::vec2(_resolution));
	
	Context::disable(Capability::Blend);
	Context::enable(Capability::DepthTest);
}

void Application::run()
{
	run_init();
	
	resize_callback(_window, _width, _height);
	
	while(!glfwWindowShouldClose(_window))
	{ 
		update();
		
		render();

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

// GLFW Callbacks

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
}

void Application::key_callback(GLFWwindow* _window, int key, int scancode, int action, int mods)
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

void Application::char_callback(GLFWwindow* window, unsigned int codepoint)
{
	//std::cout << "Received:" << codepoint << std::endl;
	_gui.handleTextInput(codepoint);
}

void Application::mouse_button_callback(GLFWwindow* _window, int button, int action, int mods)
{	
	float z = _mouse.z;
	float w = _mouse.w;

	if(action == GLFW_PRESS || !_gui.handleClick({_mouse.x, _resolution.y - _mouse.y}, button))
	{
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
	}
		
	_mouse = glm::vec4(_mouse.x, _mouse.y, z, w);
}

void Application::mouse_position_callback(GLFWwindow* _window, double xpos, double ypos)
{
	_mouse = glm::vec4(xpos, ypos, _mouse.z, _mouse.w);
}

void Application::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
}

void Application::drop_callback(GLFWwindow* window, int count, const char ** paths)
{
	for(int i = 0; i < count; ++i)
		std::cout << "Dropped: " << paths[i] << std::endl;
}