#include <Skybox.hpp>

#include <cmath>

#include <Context.hpp>

#include <Resources.hpp>

VertexArray	Skybox::s_vao;
Buffer			Skybox::s_vertex_buffer(Buffer::Target::VertexAttributes);
Buffer			Skybox::s_index_buffer(Buffer::Target::VertexIndices);

Skybox::Skybox()
{
}

Skybox::Skybox(const std::array<std::string, 6>& Paths)
{
	loadCubeMap(Paths);
}

Skybox::Skybox(const CubeMap& cubemap) :
	_cubeMap(cubemap)
{
	if(!s_vao)
		init();
}

void Skybox::loadCubeMap(const std::array<std::string, 6>& Paths)
{
	if(!s_vao)
		init();
		
	_cubeMap.load(Paths);
}
	
void Skybox::draw(const glm::mat4& p, const glm::mat4& mv) const
{
	Program& P = Resources::getProgram("SkyboxProgram");
	if(!P)
	{
		Resources::loadProgram("SkyboxProgram",
			Resources::load<VertexShader>("src/GLSL/Skybox/skybox_vs.glsl"),
			Resources::load<FragmentShader>("src/GLSL/Skybox/skybox_fs.glsl")
		);
	}
	
	Context::disable(Capability::DepthTest);
	Context::disable(Capability::CullFace);
	
	P.setUniform("ModelViewMatrix", mv);
	P.setUniform("ProjectionMatrix", p);
	P.setUniform("SkyBox", 0);
	P.use();
	_cubeMap.bind();
	s_vao.bind();
	glDrawElements(GL_QUADS, 24, GL_UNSIGNED_BYTE, 0);	/// @todo Remove raw OpenGL
	s_vao.unbind();
	_cubeMap.unbind();
	
	Context::enable(Capability::CullFace);
	Context::enable(Capability::DepthTest);
}

void Skybox::cubedraw() const
{
	Program& P = Resources::getProgram("CubeSkyboxProgram");
	if(!P)
	{
		Resources::loadProgram("CubeSkyboxProgram",
			Resources::load<VertexShader>("src/GLSL/vs.glsl"),
			Resources::load<GeometryShader>("src/GLSL/Skybox/skybox_cube_gs.glsl"),
			Resources::load<FragmentShader>("src/GLSL/Skybox/skybox_cube_fs.glsl")
		);
	}

	Context::disable(Capability::DepthTest);
	Context::disable(Capability::CullFace);
	
	P.setUniform("SkyBox", 0);
	P.use();
	_cubeMap.bind();
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);	/// @todo Remove raw OpenGL
	_cubeMap.unbind();
	
	Context::enable(Capability::CullFace);
	Context::enable(Capability::DepthTest);
}

void Skybox::init()
{
	float boxsize = std::sqrt(3)/3.f;

	float vertices[24] = {
		-boxsize,	-boxsize,	-boxsize,
		boxsize,	-boxsize,	-boxsize,
		-boxsize,	boxsize,	-boxsize,
		boxsize,	boxsize,	-boxsize,
		-boxsize,	-boxsize,	boxsize,
		boxsize,	-boxsize,	boxsize,
		-boxsize,	boxsize,	boxsize,
		boxsize,	boxsize,	boxsize
	};

	GLubyte indices[24] = {
		1, 5, 7, 3,
		2, 0, 4, 6,
		4, 5, 7, 6,
		0, 1, 3, 2,
		0, 1, 5, 4,
		3, 2, 6, 7
	};
	
	s_vao.init();
	s_vao.bind();
	
	s_vertex_buffer.init();
	s_vertex_buffer.bind();
	
	s_vertex_buffer.data(vertices, sizeof(float) * 24.0, Buffer::Usage::StaticDraw);

	s_vao.attribute(0, 3, GL_FLOAT, GL_FALSE, 3.0 * sizeof(float), (GLvoid *) 0);

	s_index_buffer.init();
	s_index_buffer.bind();
	s_index_buffer.data(indices, sizeof(GLubyte) * 24, Buffer::Usage::StaticDraw);
	
	s_vao.unbind(); // Unbind first on purpose :)
	s_index_buffer.unbind();
	s_vertex_buffer.unbind();
}
