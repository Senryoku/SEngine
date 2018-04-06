#include <ForwardRenderer.hpp>

#include <Component.hpp>

void ForwardRenderer::run_init()
{
	Application::run_init();
	
	using Resources::load;
	
	auto& Forward = Resources::loadProgram("Default",
		load<VertexShader>("src/GLSL/vs.glsl"),
		load<FragmentShader>("src/GLSL/fs.glsl")
	);
	
	Forward.bindUniformBlock("Camera", _camera.getGPUBuffer()); 
}

void ForwardRenderer::render()
{
}
