#pragma once

#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/random.hpp>
#include <random>

#include <Raytracing.hpp>
#include <TimeManager.hpp>
#include <Resources.hpp>
#include <Scene.hpp>
#include <Framebuffer.hpp>
#include <Camera.hpp>
#include <Query.hpp>
#include <Blur.hpp>

struct KeyboardShortcut
{
	int key;
	int action = GLFW_PRESS;
	int mods = 0;
	
	bool operator==(const KeyboardShortcut& o) const
	{
		return key == o.key && action == o.action && (mods == o.mods || (mods & o.mods));
	}
};

namespace std
{
	template <class T>
	inline void hash_combine(std::size_t & seed, const T & v)
	{
		std::hash<T> hasher;
		seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}
	
	template <> struct hash<KeyboardShortcut>
	{
		size_t operator()(const KeyboardShortcut & x) const
		{
			std::size_t seed = 0;
			hash_combine(seed, x.key);
			hash_combine(seed, x.action);
			hash_combine(seed, x.mods);
			return seed;
		}
	};
}

class Application
{
public:
	Application();
	Application(int argc, char* argv[]);
	virtual ~Application();

	virtual void init(const std::string& windowName = "Default Window");
	
	void clean();
	
	virtual void run();
	
	virtual void run_init();
	
	virtual void update();
	
	virtual void render() =0;
	
	virtual void renderGUI();
	
	void setFullscreen(bool val = true);
	void setMSAA(bool val = true);

	virtual void screen(const std::string& path) const;
	
	inline Scene& getScene() { return _scene; }
	
	inline bool mouseLeft() const { return _mouse.x > 0.0; }
	inline bool mouseRight() const { return _mouse.w > 0.0; }
	Ray getMouseRay() const;
	Ray getScreenRay(size_t x, size_t y) const;
	glm::vec3 getMouseProjection(float depth) const;
	
	glm::vec2 project(const glm::vec4&) const;
	inline glm::vec2 project(const glm::vec3& v) const { return project(glm::vec4{v, 1.0}); };
	
protected:	
	// Window settings
	GLFWwindow*		_window;
	int				_width = 1366;
	int				_height = 720;
	glm::vec3 		_resolution;  // Get rid of this eventually?
	bool 			_fullscreen = false;
	bool 			_vsync = false;
	bool 			_msaa = false;
	size_t			_multisampling = 4;
	
	Scene			_scene;

	// MainCamera
	bool			_cameraMoved = true;
	Camera			_camera;

	glm::vec4 		_mouse = glm::vec4(0.0);

	bool 	_menu = false;
	bool 	_controlCamera = false;
	double 	_mouse_x = 0.0, 
			_mouse_y = 0.0;

	std::unordered_map<KeyboardShortcut, std::function<void()>> _shortcuts;
	
	// Time Management
	float	_timescale = 1.0;
	float 	_time = 0.f;
	float	_frameTime;
	float	_frameRate;
	bool	_paused = false;

	// Callbacks (GLFW)
	virtual void error_callback(int error, const char* description);
	virtual void resize_callback(GLFWwindow* window, int width, int height);
	virtual void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
	virtual void char_callback(GLFWwindow* window, unsigned int codepoint);
	virtual void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
	virtual void mouse_position_callback(GLFWwindow* window, double xpos, double ypos);
	virtual void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
	virtual void drop_callback(GLFWwindow* window, int count, const char ** paths);
	
	// Static
	
	// Singleton
	static Application* s_instance;
	static Application& getInstance() { return *s_instance; }
	
	// Static callbacks (C-like callbacks)
	static void s_error_callback(int error, const char* description)
	{ getInstance().error_callback(error, description); }
	
	static void s_resize_callback(GLFWwindow* window, int width, int height)
	{ getInstance().resize_callback(window, width, height); }
	
	static void s_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{ getInstance().key_callback(window, key, scancode, action, mods); }
	
	static void s_char_callback(GLFWwindow* window, unsigned int codepoint)
	{ getInstance().char_callback(window, codepoint); }
	
	static void s_mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
	{ getInstance().mouse_button_callback(window, button, action, mods); }
	
	static void s_mouse_position_callback(GLFWwindow* window, double xpos, double ypos)
	{ getInstance().mouse_position_callback(window, xpos, ypos); }
	
	static void s_scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
	{ getInstance().scroll_callback(window, xoffset, yoffset); }
	
	static void s_drop_callback(GLFWwindow* window, int count, const char ** paths)
	{ getInstance().drop_callback(window, count, paths); }
};
