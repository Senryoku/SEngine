#include <Application.hpp>

#include <functional>

#include <stb_image_write.hpp>
#include <glm/gtx/transform.hpp>

#include <imgui.h>
#include <imgui_impl_glfw_gl3.h>

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
	glfwWindowHint(GLFW_SAMPLES, _multisampling);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

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
	
	if(!Context::init())
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
	glfwSetScrollCallback(_window, s_scroll_callback);
	glfwSetDropCallback(_window, s_drop_callback);
	
	if(_controlCamera)
		glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	
	ImGui_ImplGlfwGL3_Init(_window, false);
	ImGui::GetIO().MouseDrawCursor = false;
	
	Context::enable(Capability::DepthTest);
	
	_scene.init();
	
	_camera_buffer.init();
	_camera_buffer.bind(0);
}

void Application::clean()
{
	ImGui_ImplGlfwGL3_Shutdown();
	glfwDestroyWindow(_window);
	glfwTerminate();
}

void Application::run_init()
{
}

void Application::update()
{
	glfwSetWindowTitle(_window,
		std::string("SEngine - ")
			.append(std::to_string(1000.f * TimeManager::getRealDeltaTime()))
			.append("ms - FPS: ")
			.append(std::to_string(TimeManager::getInstantFrameRate()))
			.c_str());
	
	_cameraMoved = false;
	if(_controlCamera)
	{
		float _frameTime = TimeManager::getRealDeltaTime(); // Should move even on pause :)
		if(glfwGetKey(_window, GLFW_KEY_W) == GLFW_PRESS)
		{
			_cameraMoved = true;
			_camera.moveForward(_frameTime);
		}
			
		if(glfwGetKey(_window, GLFW_KEY_A) == GLFW_PRESS)
		{
			_cameraMoved = true;
			_camera.strafeLeft(_frameTime);
		}
				
		if(glfwGetKey(_window, GLFW_KEY_S) == GLFW_PRESS)
		{
			_cameraMoved = true;
			_camera.moveBackward(_frameTime);
		}
				
		if(glfwGetKey(_window, GLFW_KEY_D) == GLFW_PRESS)
		{
			_cameraMoved = true;
			_camera.strafeRight(_frameTime);
		}
				
		if(glfwGetKey(_window, GLFW_KEY_Q) == GLFW_PRESS)
		{
			_cameraMoved = true;
			_camera.moveDown(_frameTime);
		}
				
		if(glfwGetKey(_window, GLFW_KEY_E) == GLFW_PRESS)
		{
			_cameraMoved = true;
			_camera.moveUp(_frameTime);
		}
			
		double mx = _mouse_x, my = _mouse_y;
		glfwGetCursorPos(_window, &_mouse_x, &_mouse_y);
		_cameraMoved = _cameraMoved || (_mouse_x != mx || _mouse_y != my);
		if(_mouse_x != mx || _mouse_y != my)
			_camera.look(glm::vec2(_mouse_x - mx, my - _mouse_y));
	}
	_camera.updateView();
	_invViewMatrix = glm::inverse(_camera.getMatrix());
	_invViewProjection = _invViewMatrix * _invProjection;
	
	_gpuCameraData = {_camera.getMatrix(), _projection};
	_camera_buffer.data(&_gpuCameraData, sizeof(GPUViewProjection), Buffer::Usage::DynamicDraw);
	_camera_buffer.unbind();

	if(!_paused || _time == 0.0f)
		_scene.update();
}

void Application::renderGUI()
{
	ImGui::Render();
}

void Application::run()
{
	run_init();
	
	resize_callback(_window, _width, _height);
	
	while(!glfwWindowShouldClose(_window))
	{
		// Time Management
		TimeManager::update();
		_frameTime = TimeManager::getRealDeltaTime();
		_frameRate = TimeManager::getInstantFrameRate();
		if(!_paused)
		{
			_time += _timescale * _frameTime;
			_frameTime *= _timescale;
			if(_frameTime > 1.0/60.0) _frameTime = 1.0/60.0; // In case the _window is moved
		} else _frameTime = 0.0;
		
		ImGui_ImplGlfwGL3_NewFrame();
	
		update();
		
		render();

		glfwSwapBuffers(_window);
		glfwPollEvents();
	}
}

void Application::setFullscreen(bool val)
{
	_fullscreen = val;
	if(_fullscreen)
	{
		int monitor_count;
		const auto monitor = glfwGetMonitors(&monitor_count)[0];
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		glfwSetWindowMonitor(_window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
	} else {
		glfwSetWindowMonitor(_window, nullptr, 100, 100, 1280, 720, 0);
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

	Context::viewport(0, 0, _width, _height);
	_resolution = glm::vec3(_width, _height, 0.0);
	
	update_projection();
}

void Application::setMSAA(bool val)
{
	if(val)
		Context::enable(Capability::Multisample);
	else
		Context::disable(Capability::Multisample);
	
	GLint iMultiSample = 0;
	GLint iNumSamples = 0;
	glGetIntegerv(GL_SAMPLE_BUFFERS, &iMultiSample);
	glGetIntegerv(GL_SAMPLES, &iNumSamples);
	
	glfwWindowHint(GLFW_SAMPLES, iMultiSample);
	
	Log::info(val ? "Enabled" : "Disabled", " MSAA (GL_SAMPLES : ", iNumSamples, ", GL_SAMPLE_BUFFERS : ", iMultiSample,")");
}

void Application::key_callback(GLFWwindow* _window, int key, int scancode, int action, int mods)
{
	if(!_controlCamera)
		ImGui_ImplGlfwGL3_KeyCallback(_window, key, scancode, action, mods);
	if(ImGui::GetIO().WantCaptureKeyboard)
		return;
	
	if(action == GLFW_PRESS)
	{
		switch(key)
		{
			case GLFW_KEY_N:
			{				
				Log::info(_camera.getPosition().x, "\t", _camera.getPosition().y, "\t", _camera.getPosition().z);
				break;
			}
			case GLFW_KEY_ESCAPE:
			{
				_menu = !_menu;
				break;
			}
			case GLFW_KEY_R:
			{
				Log::info("Reloading shaders...");
				Resources::reloadShaders();
				Log::info("Reloading shaders... Done !");
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
				setMSAA(_msaa);
				break;
			}
			case GLFW_KEY_ENTER:
			{
				if(mods & GLFW_MOD_ALT)
					setFullscreen(!_fullscreen);
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
				Log::info("Saving a screenshot to ", ScreenPath, "...");
				screen(ScreenPath);
				break;
			}
			case GLFW_KEY_KP_ADD:
			{
				if(_camera.speed() < 1)
					_camera.speed() += .1;
				else
					_camera.speed() += 1;
				Log::info("Camera Speed: ", _camera.speed());
				break;
			}
			case GLFW_KEY_KP_SUBTRACT:
			{
				if(_camera.speed() <= 1)
					_camera.speed() -= .1;
				else
					_camera.speed() -= 1;
				Log::info("Camera Speed: ", _camera.speed());
				break;
			}
		}
	}
}

void Application::char_callback(GLFWwindow* window, unsigned int codepoint)
{
	if(!_controlCamera)
		ImGui_ImplGlfwGL3_CharCallback(window, codepoint);
	if(ImGui::GetIO().WantCaptureKeyboard)
		return;
}

Ray Application::getScreenRay(size_t x, size_t y) const
{
	auto ratio = _resolution.y / _resolution.x;
	auto d = glm::normalize(((2.0f * x) / _resolution.x - 1.0f) * _camera.getRight() + -ratio * ((2.0f * y) / _resolution.y - 1.0f) * glm::cross(_camera.getRight(), _camera.getDirection()) + _camera.getDirection());

	return Ray{_camera.getPosition(), d};
}

Ray Application::getMouseRay() const
{
	return getScreenRay(_mouse.x, _mouse.y);
}

glm::vec3 Application::getMouseProjection(float depth) const
{
	auto o = _invProjection * glm::vec4(2.0 * _mouse.x / _resolution.x - 1.0, -(2.0 * _mouse.y / _resolution.y - 1.0), depth, 1.0);
	o /= o.w;
	o = _invViewMatrix * o;
	return glm::vec3(o);
}

glm::vec2 Application::project(const glm::vec4& v) const
{
	auto t = _camera.getMatrix() * v;
	if(t.z > 0.0) // Truncate is point is behind camera
		t.z = 0.0;
	t = _projection * t;
	auto r = glm::vec2{t.x, -t.y} / t.w;
	r = 0.5f * (r + 1.0f);
	r.x *= _width;
	r.y *= _height;
	return r;
}

void Application::update_projection()
{
	float inRad = _fov * glm::pi<float>()/180.f;
	_projection = glm::perspective(inRad, (float) _width/_height, _near, _far);
	_invProjection = glm::inverse(_projection);
}

void Application::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{	
	float z = _mouse.z;
	float w = _mouse.w;

	if(!_controlCamera)
		ImGui_ImplGlfwGL3_MouseButtonCallback(_window, button, action, mods);
	if(ImGui::GetIO().WantCaptureMouse)
		return;
	
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

void Application::mouse_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	_mouse = glm::vec4(xpos, ypos, _mouse.z, _mouse.w);
}

void Application::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	if(!_controlCamera)
		ImGui_ImplGlfwGL3_ScrollCallback(window, xoffset, yoffset);
}

void Application::drop_callback(GLFWwindow* window, int count, const char ** paths)
{
	for(int i = 0; i < count; ++i)
		Log::info("Dropped: ", paths[i]);
}
