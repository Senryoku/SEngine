#include <ForwardRenderer.hpp>

#include <stb_image_write.hpp>

ForwardRenderer::ForwardRenderer(int argc, char* argv[]) :
	Application(argc, argv)
{
}

void ForwardRenderer::run_init()
{
	auto& Simple = loadProgram("Simple",
		load<VertexShader>("src/GLSL/vs.glsl"),
		load<FragmentShader>("src/GLSL/fs.glsl")
	);
	Simple.bindUniformBlock("Camera", _camera_buffer); 
	
	auto& Forward = loadProgram("Forward",
		load<VertexShader>("src/GLSL/Forward/forward_vs.glsl"),
		load<FragmentShader>("src/GLSL/Forward/forward_fs.glsl")
	);
	Forward.bindUniformBlock("Camera", _camera_buffer); 
}

void ForwardRenderer::render()
{
	Context::clear();

	_scene.draw(_projection, _camera.getMatrix());
	
	renderGUI();
}
