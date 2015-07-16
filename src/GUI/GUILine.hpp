#pragma once

#include <GUIElement.hpp>

class GUILine : public GUIElement
{
public:
	glm::vec4	Color = glm::vec4(1.0);
	
	GUILine(glm::vec4 color = glm::vec4(1.0));

	void draw(const glm::vec2& resolution, const glm::vec2& position = glm::vec2(0.0)) override;

	/**
	 * Initialize internal data and rendering components.
	**/
	void init();
	
	void update(Buffer::Usage hint = Buffer::Usage::StaticDraw);
	
	inline std::vector<glm::vec2>& getVertices() { return _vertices; }

private:
	std::vector<glm::vec2>	_vertices;
	
	VertexArray				_vao;
	Buffer						_vertex_buffer;
};