#pragma once

#include <string>
#include <memory>

#include <glm/glm.hpp>

#include <Buffer.hpp>
#include <VertexArray.hpp>
#include <ResourcesManager.hpp>

class Font
{
public:
	struct Glyph
	{
		glm::vec2	dim = glm::vec2(0.0);
		glm::vec2	offset = glm::vec2(0.0);
		glm::vec2	uv_min = glm::vec2(0.0);
		glm::vec2	uv_max = glm::vec2(0.0);
		float		advance = 0.0;
	};
	
	std::string			Name;
	Texture2D*				Tex = nullptr;
	std::vector<Glyph>	Glyphs;
	
	Font() =default;
	Font(const std::string& path);
	
	void load(const std::string& path);
};

class Text
{
public:
	Text();
	Text(const std::string& str);
	~Text();
	
	void setText(const std::string& str);
	void setFontSize(float s) { _fontSize = s; update(); }
	
	void init();
	
	void update();
	
	void draw() const;
	
	
private:
	std::string		_text;

	glm::vec2			_position = glm::vec2(25.0);
	Font*				_font;
	float				_fontSize = 16.0;
	
	struct VertexAttributes
	{
		glm::vec2	position;
		glm::vec2	texcoord;
	};

	VertexArray		_vao;
	Buffer				_vertex_buffer;
	Buffer				_index_buffer;

	std::vector<VertexAttributes>	_vertices;
	std::vector<size_t>				_triangles;
	
	const Font::Glyph& getGlyph(char c) const;
	
	static std::unique_ptr<Font>		s_defaultFont;
};
