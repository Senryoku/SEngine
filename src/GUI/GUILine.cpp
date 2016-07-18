#include <GUIGraph.hpp>

GUILine::GUILine(glm::vec4 color) :
	Color(color)
{
	init();
}

void GUILine::draw(const glm::vec2& resolution, const glm::vec2& position)
{
	auto p = c2p(position);
	
	auto& P = Resources::getProgram("Line");
	if(!P)
	{
		P = Resources::loadProgram("Line",
			Resources::load<VertexShader>("src/GLSL/GUI/line_vs.glsl"),
			Resources::load<FragmentShader>("src/GLSL/GUI/line_fs.glsl")
		);
	}
	P.use();
	P.setUniform("Resolution", resolution);
	P.setUniform("Position", p);
	P.setUniform("Color", Color);

	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT,  GL_NICEST);
	_vao.bind();
	glDrawArrays(GL_LINE_STRIP, 0, _vertices.size());
	_vao.unbind();
}

void GUILine::init()
{
	_vao.init();
	_vao.bind();
	
	_vertex_buffer.init();
	_vertex_buffer.bind();
	
	_vao.attribute(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	
	_vao.unbind();
	_vertex_buffer.unbind();
}

void GUILine::update(Buffer::Usage hint)
{
	_vertex_buffer.data(_vertices.data(), sizeof(glm::vec2) * _vertices.size(), hint);
}
