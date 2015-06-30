#pragma once

#include <GL/gl3w.h>

#include <GLFW/glfw3.h>

#include <TimeManager.hpp>
#include <ResourcesManager.hpp>
#include <Scene.hpp>
#include <Framebuffer.hpp>
#include <Camera.hpp>
#include <Query.hpp>
#include <Buffer.hpp>
#include <Blur.hpp>

class Application
{
public:
	Application();
	virtual ~Application();

	void init(const std::string& windowName = "Default Window");
	
	void clean();
	
	virtual void run();
	
	virtual void run_init();
	
	virtual void in_loop_timing();
	
	virtual void in_loop_fps_camera();
	
	virtual void in_loop_update();
	
	virtual void in_loop_render();
	
	virtual void offscreen_render(const glm::mat4& p, const glm::mat4& v) {};

	void screen(const std::string& path) const;
	
	Scene& getScene() { return _scene; }
	
protected:
	// Window settings
	GLFWwindow*	_window;
	int				_width = 1366;
	int				_height = 720;
	bool 			_fullscreen = false;
	bool 			_msaa = false;

	// MainCamera
	Camera		_camera;
	float		_fov = 60.0;
	glm::vec3 	_resolution;
	glm::mat4 	_projection;
	glm::vec4 	_mouse = glm::vec4(0.0);

	bool 		_controlCamera = true;
	double 	_mouse_x = 0.0, 
				_mouse_y = 0.0;

	UniformBuffer _camera_buffer;

	// Time
	float		_timescale = 0.5;
	float 		_time = 0.f;
	float		_frameTime;
	float		_frameRate;
	bool		_paused = false;
	
	// Rendering
	Framebuffer<Texture2D, 3>	_offscreenRender;
	
	Scene							_scene;
	
	bool	_debug = true;

	// Callbacks (GLFW)
	void error_callback(int error, const char* description);
	void resize_callback(GLFWwindow* window, int width, int height);
	void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
	void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
	void mouse_position_callback(GLFWwindow* window, double xpos, double ypos);
	
	static Application* s_instance;
	static Application& getInstance() { return *s_instance; }
	
	static void s_error_callback(int error, const char* description)
	{ getInstance().error_callback(error, description); }
	
	static void s_resize_callback(GLFWwindow* window, int width, int height)
	{ getInstance().resize_callback(window, width, height); }
	
	static void s_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{ getInstance().key_callback(window, key, scancode, action, mods); }
	
	static void s_mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
	{ getInstance().mouse_button_callback(window, button, action, mods); }
	
	static void s_mouse_position_callback(GLFWwindow* window, double xpos, double ypos)
	{ getInstance().mouse_position_callback(window, xpos, ypos); }
};
