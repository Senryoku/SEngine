#pragma once

#include <string>
#include <vector>
#include <array>

#include <glm/glm.hpp>

#include <Buffer.hpp>
#include <VertexArray.hpp>

class Axes
{
public:
	Axes()
	{
		if(!_vao)
		{
			_vao.init();
			_vao.bind();
			
			_vertex_buffer.init();
			_vertex_buffer.bind();
			
			std::array<glm::vec3, 6> _vertices = {glm::vec3(0.f),
														glm::vec3(1.f, 0.f, 0.f),
														glm::vec3(0.f),
														glm::vec3(0.f, 1.f, 0.f),
														glm::vec3(0.f),
														glm::vec3(0.f, 0.f, 1.f)
													};

			_vertex_buffer.data(&_vertices[0], sizeof(glm::vec3)*_vertices.size(), Buffer::Usage::StaticDraw);

			_vao.attribute(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid *) 0);

			_vao.unbind();
			_vertex_buffer.unbind();
		}
		
		if(!_vao_marks)
		{
			_vao_marks.init();
			_vao_marks.bind();
			
			_vertex_buffer_marks.init();
			_vertex_buffer_marks.bind();
			
			std::vector<glm::vec3> _vertices;
			float s = 0.05f;
			for(int i = -5; i < 5; ++i)
				for(int j = -5; j < 5; ++j)
					for(int k = -5; k < 5; ++k)
					{
						_vertices.push_back(10.0f * glm::vec3(i - s, j, k));
						_vertices.push_back(10.0f * glm::vec3(i + s, j, k));
						_vertices.push_back(10.0f * glm::vec3(i, j - s, k));
						_vertices.push_back(10.0f * glm::vec3(i, j + s, k));
						_vertices.push_back(10.0f * glm::vec3(i, j, k - s));
						_vertices.push_back(10.0f * glm::vec3(i, j, k + s));
						
						_vertices.push_back(10.0f * glm::vec3(i - s + 0.5f, j, k));
						_vertices.push_back(10.0f * glm::vec3(i + s + 0.5f, j, k));
						_vertices.push_back(10.0f * glm::vec3(i, j - s + 0.5f, k));
						_vertices.push_back(10.0f * glm::vec3(i, j + s + 0.5f, k));
						_vertices.push_back(10.0f * glm::vec3(i, j, k - s + 0.5f));
						_vertices.push_back(10.0f * glm::vec3(i, j, k + s + 0.5f));
					}
					
			_vertex_buffer_marks.data(&_vertices[0], sizeof(glm::vec3)*_vertices.size(), Buffer::Usage::StaticDraw);

			_vao_marks.attribute(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid *) 0);

			_vao_marks.unbind();
			_vertex_buffer_marks.unbind();
		}
	}
	
	void draw() const
	{
		_vao.bind();
		glDrawArrays(GL_LINES, 0, 6);
		_vao.unbind();
	}
	void drawMarks() const
	{
		_vao_marks.bind();
		glDrawArrays(GL_LINES, 0, 10 * 10 * 10 * 12);
		_vao_marks.unbind();
	}

private:
	static VertexArray		_vao;
	static Buffer				_vertex_buffer;
	
	static VertexArray		_vao_marks;
	static Buffer				_vertex_buffer_marks;
};