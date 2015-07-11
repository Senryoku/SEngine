#pragma once

#include <string>
#include <memory>

#include <glm/glm.hpp>

#include <Buffer.hpp>
#include <VertexArray.hpp>
#include <ResourcesManager.hpp>
#include <GUIClickable.hpp>

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

/**
 * Handles the rendering of distance field fonts.
**/
class GUIText : public GUIClickable
{
public:
	GUIText();
	GUIText(const std::string& str);
	virtual ~GUIText();
	
	void setText(const std::string& str);
	void setFontSize(float s) { _fontSize = s; update(); }
	
	void init();
	
	void update();
	
	void draw(const glm::vec2& resolution, const glm::vec2& position = glm::vec2(0.0)) const override;
	
private:
	std::string		_text;
	
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
