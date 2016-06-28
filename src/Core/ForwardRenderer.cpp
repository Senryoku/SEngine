#include <ForwardRenderer.hpp>

#include <stb_image_write.hpp>
#include <glm/gtc/matrix_transform.hpp>

ForwardRenderer::ForwardRenderer(int argc, char* argv[]) :
	Application{argc, argv},
	_shadow_maps{
		ShadowBuffer{CascadeResolution},
		ShadowBuffer{CascadeResolution},
		ShadowBuffer{CascadeResolution}
	}
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
	
	// Cascade Initialization
	auto& DepthProgram = ResourcesManager::getInstance().getProgram("Light_Depth");
	if(!DepthProgram)
	{
		DepthProgram = loadProgram("Light_Depth",
			load<VertexShader>("src/GLSL/depth_vs.glsl"),
			load<FragmentShader>("src/GLSL/depth_fs.glsl")
		);
	}
	
	for(size_t i = 0; i < CascadeCount ; i++)
	{
		_shadow_maps[i].getColor().init();
		_shadow_maps[i].getColor().bind();
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, CascadeResolution, CascadeResolution, 0, GL_RGBA, GL_FLOAT, nullptr);
		_shadow_maps[i].getColor().set(Texture::Parameter::WrapS, GL_CLAMP_TO_EDGE);
		_shadow_maps[i].getColor().set(Texture::Parameter::WrapT, GL_CLAMP_TO_EDGE);
		_shadow_maps[i].getColor().set(Texture::Parameter::MinFilter, GL_LINEAR_MIPMAP_LINEAR);
		_shadow_maps[i].getColor().set(Texture::Parameter::MagFilter, GL_LINEAR);
		_shadow_maps[i].getColor().unbind();
		_shadow_maps[i].init();
	}
}

void ForwardRenderer::render()
{
	Context::viewport(0, 0, _width, _height);
	Context::clear();

	auto& Forward = ResourcesManager::getInstance().getProgram("Forward");
	Forward.setUniform("CameraPosition", _camera.getPosition());
	for(size_t i = 0; i < CascadeCount; ++i)
	{
		Forward.setUniform("LightMatrix[" + std::to_string(i) + "]", _cascade_matrices_biased[i]);
		Forward.setUniform("ShadowMaps[" + std::to_string(i) + "]", _shadow_maps[i].getColor().getName());
		Forward.setUniform("CascadeFar[" + std::to_string(i) + "]", _cascade_far[i]);
	}
	_scene.draw(_projection, _camera.getMatrix());
	
	renderGUI();
}

void ForwardRenderer::update()
{
	Application::update();
	
	update_cascade_matrices();
	auto& DepthProgram = ResourcesManager::getInstance().getProgram("Light_Depth");
	Context::enable(Capability::CullFace);
	Context::enable(Capability::DepthClamp);
	Context::depthFunc(DepthFunction::LEqual);
	for(size_t i = 0; i < CascadeCount; i++)
	{
		_shadow_maps[i].bind();
		_shadow_maps[i].clear(BufferBit::All);
		DepthProgram.setUniform("DepthVP", _cascade_matrices[i]);
		DepthProgram.use();
		for(auto& b : _scene.getObjects())
			//if(b.isVisible(_cascade_proj[i], _cascade_light_view))
			{
				DepthProgram.setUniform("ModelMatrix", b.getModelMatrix());
				b.getMesh().draw();
			}
	}
	Context::disable(Capability::DepthClamp);
	Context::disable(Capability::CullFace);
	Program::useNone();
	_shadow_maps[0].unbind();
}

void ForwardRenderer::update_cascade_matrices()
{
	auto cam = _projection * _camera.getMatrix();
	auto cam_inv = glm::inverse(cam);

	//_cascade_light_view = glm::lookAt(glm::vec3{0.0}, _light_direction, glm::vec3{0.0, 1.0, 0.0});
	_cascade_light_view = glm::lookAt(-_light_direction, glm::vec3{0.0}, glm::vec3{0.0, 1.0, 0.0});
	//auto inv_light_view = glm::inverse(_cascade_light_view); //glm::lookAt(_light_direction, glm::vec3{0.0}, glm::vec3{0.0, 1.0, 0.0});
	
	auto view2light = _cascade_light_view * cam_inv;
	
	float z_step = 2.0 / CascadeCount;

	for(size_t i = 0; i < CascadeCount; i++)
	{
		float tnear = -1.0 + i * z_step;
		float tfar = -1.0 + (i + 1) * z_step;
		const glm::vec4 tmp[8] = {
			view2light * glm::vec4{-1.0f, -1.0f, tnear, 1.0f},
			view2light * glm::vec4{ 1.0f, -1.0f, tnear, 1.0f},
			view2light * glm::vec4{-1.0f,  1.0f, tnear, 1.0f},
			view2light * glm::vec4{ 1.0f,  1.0f, tnear, 1.0f},
			view2light * glm::vec4{-1.0f, -1.0f, tfar,  1.0f},
			view2light * glm::vec4{ 1.0f, -1.0f, tfar,  1.0f},
			view2light * glm::vec4{-1.0f,  1.0f, tfar,  1.0f},
			view2light * glm::vec4{ 1.0f,  1.0f, tfar,  1.0f}
		};
		
		glm::vec3 frustum[8];
		glm::vec3 min{std::numeric_limits<float>::max()}, max{std::numeric_limits<float>::min()};
		for(size_t i = 0; i < 8 ; i++)
		{
			frustum[i] = glm::vec3{tmp[i]} / tmp[i].w;
			min = glm::min(min, frustum[i]);
			max = glm::max(max, frustum[i]);
		}
		_cascade_far[i] = (i + 1) * _far / CascadeCount;
		_cascade_proj[i] = glm::ortho(min.x, max.x, min.y, max.y, min.z, max.z);
		_cascade_matrices[i] = _cascade_proj[i] * _cascade_light_view;
		_cascade_matrices_biased[i] = 
			glm::mat4{
				0.5, 0.0, 0.0, 0.0,
				0.0, 0.5, 0.0, 0.0,
				0.0, 0.0, 0.5, 0.0,
				0.5, 0.5, 0.5, 1.0
			} * _cascade_matrices[i];
	}
}
