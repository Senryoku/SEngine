#include <sstream>

#include <GL/gl3w.h>

#include <GLFW/glfw3.h>

#include <Context.hpp>
#include <Shaders.hpp>
#include <ResourcesManager.hpp>
#include <TimeManager.hpp>
#include <Material.hpp>

int			_width = 1366;
int			_height = 720;

glm::vec3 	_resolution(_width, _height, 0.0);
glm::vec4 	_mouse(0.0);

float		_time = 0.f;
float		_frameTime;
float		_frameRate;
	
void error_callback(int error, const char* description);
void resize_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void mouse_pos_callback(GLFWwindow* window, double xpos, double ypos);

int main(int argc, char* argv[])
{
	if (glfwInit() == false)
	{
		std::cerr << "Error: couldn't initialize GLFW." << std::endl;
		exit(EXIT_FAILURE);
	}
	glfwSetErrorCallback(error_callback);
    GLFWwindow* window = nullptr;
	
	if(argc > 1)
	{
		auto monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		_width = mode->width;
		_height = mode->height;
		window = glfwCreateWindow(_width, _height, "ShaderToy Native", monitor, nullptr);
	} else {
		window = glfwCreateWindow(_width, _height, "ShaderToy Native", nullptr, nullptr);
	}
	
	if(!window)
	{
		std::cerr << "Error: couldn't create window." << std::endl;
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	
	glfwMakeContextCurrent(window);
	if(gl3wInit())
	{
		std::cerr << "Error: couldn't initialize gl3w." << std::endl;
		exit(EXIT_FAILURE);
	}
	
	// Callback Setting
	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, (GLFWmousebuttonfun) mouse_button_callback);
	glfwSetCursorPosCallback(window, (GLFWcursorposfun) mouse_pos_callback);
	glfwSetWindowSizeCallback(window, resize_callback);

	auto& RayTracer = loadProgram("RayTracer",
		load<VertexShader>("src/GLSL/ShaderToy/vs.glsl"),
		load<FragmentShader>("src/GLSL/ShaderToy/fs.glsl")
	);

	Material RayTraced;
	RayTraced.setShadingProgram(RayTracer);
	
	// Todo: Manage texture inputs!
	//RayTraced.setAttributeRef("iChannel0", CM);
	//RayTraced.setAttributeRef("iChannel2", Tex);
	// ...
	
	RayTraced.setUniform("iGlobalTime",	&_time);
	RayTraced.setUniform("iResolution",	&_resolution);
	RayTraced.setUniform("iMouse",		&_mouse);
	
	while(!glfwWindowShouldClose(window))
	{	
		TimeManager::getInstance().update();
		_frameTime = TimeManager::getInstance().getRealDeltaTime();
		_time += _frameTime;
		_frameRate = TimeManager::getInstance().getInstantFrameRate();
		
		std::ostringstream oss;
		oss << "ShaderToy Native - FPS: " << _frameRate;
		glfwSetWindowTitle(window, oss.str().c_str());
		
		Context::clear();
		
		RayTraced.use();
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	
	glfwDestroyWindow(window);
}

// Callbacks definitions

void error_callback(int error, const char* description)
{
	std::cerr << "GLFW Error (" << error << "): " << description << std::endl;
}

void resize_callback(GLFWwindow* window, int width, int height)
{
	_width = width;
	_height = height;

	glViewport(0, 0, _width, _height);
	_resolution = glm::vec3(_width, _height, 0.0);
	
	std::cout << "Reshaped to " << width << "*" << height  << " (" << ((GLfloat) _width)/_height << ")" << std::endl;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
		
	if(key == GLFW_KEY_R && action == GLFW_PRESS)
	{
		ResourcesManager::getInstance().reloadShaders();
		std::cout << "Shaders reloaded." << std::endl;
	}
}

inline void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if(button == GLFW_MOUSE_BUTTON_1)
		_mouse.z = (action == GLFW_PRESS) ? 1.0 : 0.0;
	else if(button == GLFW_MOUSE_BUTTON_2)
		_mouse.w = (action == GLFW_PRESS) ? 1.0 : 0.0;
}

inline void mouse_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
	if(_mouse.z)
		_mouse = glm::vec4(xpos, ypos, _mouse.z, _mouse.w);
}
