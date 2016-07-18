#include <GUIText.hpp>

#include <fstream>
#include <sstream>

std::unique_ptr<Font>		GUIText::s_defaultFont = nullptr;

Font::Font(const std::string& path)
{
	load(path);
}

void Font::load(const std::string& path)
{
	// Load the distance filed texture
	Tex = &Resources::getTexture<Texture2D>(Name);
	if(!(*Tex))
		Tex->load(path + std::string(".png"));
	
	// Load the descriptive file for the font
	std::ifstream file(path + std::string(".txt"));
	std::string line;
	std::getline(file, line);
	auto first = line.find('\"') + 1;
	auto second = line.find('\"', first);
	Name = line.substr(first, second - first);
	//std::cout << "Loading " << Name << " from " << path << std::endl;
	
	size_t pos = 0;
	
	// Search for glyph size
	size_t size = 221;
	
	pos = line.find('=', second + 1) + 1;
	if(pos != line.size())
	{
		std::stringstream ss;
		ss.str(line.substr(pos)),
		ss >> size;
	} else std::cerr << "Warning: Couldn't extract glyph size from font descriptive file." << std::endl;
	
	std::getline(file, line);
	size_t glyph_count = 256;
	pos = line.find('=') + 1;
	if(pos != line.size())
	{
		std::stringstream ss;
		ss.str(line.substr(pos)),
		ss >> glyph_count;
	}
	
	/// @tod Optimize this shit.
	glm::vec2 dim = {
		static_cast<float>(Tex->getSize().x), 
		static_cast<float>(Tex->getSize().y)
	};
	Glyphs.resize(glyph_count);
	Glyph tmp;
	std::stringstream ss;
		
	auto get_next = [&](auto& v) {
		pos = line.find('=', pos) + 1;
		ss.str(line.substr(pos));
		ss >> v;
	};
		
	while(std::getline(file, line))
	{
		size_t c;
		pos = 0;
		get_next(c);
		
		if(Glyphs.size() - 1 < c)
			Glyphs.resize(c + 1);
		
		get_next(tmp.uv_min.x);
		get_next(tmp.uv_min.y);
		get_next(tmp.dim.x);
		get_next(tmp.dim.y);
		get_next(tmp.offset.x);
		get_next(tmp.offset.y);
		get_next(tmp.advance);
		
		tmp.uv_max = tmp.uv_min + tmp.dim;
		
		tmp.dim /= static_cast<float>(size);
		tmp.uv_min /= dim;
		tmp.uv_max /= dim;
		tmp.offset /= static_cast<float>(size);
		tmp.offset.y -= tmp.dim.y;
		tmp.advance /= static_cast<float>(size);
		/*
		std::cout << "Add Glyph " << c << "(" << (char) c << ") : " 
			<< tmp.dim.x << " " << tmp.dim.y << " ; " 
			<< tmp.uv_min.x << " " << tmp.uv_min.y << " ; "  
			<< tmp.uv_max.x << " " << tmp.uv_max.y << " ; "
			<< tmp.offset.x << " " << tmp.offset.y << " ; " 
			<< tmp.advance << " " << std::endl;
		*/
		Glyphs[c] = tmp;
	}
	file.close();
}

GUIText::GUIText() :
	_vao(),
	_vertex_buffer(Buffer::Target::VertexAttributes),
	_index_buffer(Buffer::Target::VertexIndices)
{
	if(!s_defaultFont)
		s_defaultFont.reset(new Font("in/Fonts/default_unicode"));
	_font = &*s_defaultFont;
}

GUIText::~GUIText()
{
}

GUIText::GUIText(const std::string& str) :
	_text(str),
	_vao(),
	_vertex_buffer(Buffer::Target::VertexAttributes),
	_index_buffer(Buffer::Target::VertexIndices)
{
	if(!s_defaultFont)
		s_defaultFont.reset(new Font("in/Fonts/default_unicode"));
	_font = &*s_defaultFont;

	init();
}

GUIText::GUIText(const TextFunc& func) :
	_func(func),
	_text(func()),
	_vao(),
	_vertex_buffer(Buffer::Target::VertexAttributes),
	_index_buffer(Buffer::Target::VertexIndices)
{
	if(!s_defaultFont)
		s_defaultFont.reset(new Font("in/Fonts/default_unicode"));
	_font = &*s_defaultFont;

	init();
}

void GUIText::setText(const std::string& str)
{
	if(str != _text)
	{
		_text = str;
		update();
	}
}

void GUIText::init()
{
	//std::cout << "Initializing text... ";
	_vao.init();
	_vao.bind();
	
	_vertex_buffer.init();
	_vertex_buffer.bind();
	
    _vao.attribute(0, 2, GL_FLOAT, GL_FALSE, sizeof(VertexAttributes), (GLvoid *) offsetof(struct VertexAttributes, position));
    _vao.attribute(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexAttributes), (GLvoid *) offsetof(struct VertexAttributes, texcoord));

	_index_buffer.init();
	_index_buffer.bind();
	
	_vao.unbind();
	_index_buffer.unbind();
	_vertex_buffer.unbind();
	//std::cout << "Done." << std::endl;
	
	update();
}

void GUIText::update()
{
	//std::cout << "Updating text... ";
	_vertices.clear();
	_vertices.reserve(_text.size() * 4);
	_triangles.clear();
	_triangles.reserve(_text.size() * 3 * 2);
	glm::vec2 position = glm::vec2(0.0);

	_aabb.min = position;
	_aabb.max = position;
	
	for(auto& c : _text)
	{
		if(c == '\0') continue;
		
		auto g = getGlyph(c);
		size_t idx = _vertices.size();
		
		auto p = position + _fontSize * g.offset;
		
		_vertices.push_back({
							p, 
							glm::vec2(g.uv_min.x, g.uv_max.y)
						});
							
		_vertices.push_back({
							p + glm::vec2(g.dim.x, 0.0) * _fontSize, 
							glm::vec2(g.uv_max.x, g.uv_max.y)
						});
							
		_vertices.push_back({
							p + g.dim * _fontSize, 
							glm::vec2(g.uv_max.x, g.uv_min.y)
						});
		
		_vertices.push_back({
							p + glm::vec2(0.0, g.dim.y) * _fontSize, 
							glm::vec2(g.uv_min.x, g.uv_min.y)
						});
							
		_triangles.push_back(idx);
		_triangles.push_back(idx + 1);
		_triangles.push_back(idx + 2);
		_triangles.push_back(idx + 2);
		_triangles.push_back(idx + 3);
		_triangles.push_back(idx);
		
		//_aabb.max = glm::max(_aabb.max, p + g.dim * _fontSize);
		
		position.x += g.advance * _fontSize;
		_aabb.max = glm::max(_aabb.max, {position.x, (p + g.dim * _fontSize).y});
	}
	
	_vertex_buffer.bind();
	_vertex_buffer.data(_vertices.data(), sizeof(VertexAttributes) * _vertices.size(), Buffer::Usage::DynamicDraw);
	_index_buffer.bind();
	_index_buffer.data(_triangles.data(), sizeof(size_t) * _triangles.size(), Buffer::Usage::DynamicDraw);
	//std::cout << "Done." << std::endl;
}

const Font::Glyph& GUIText::getGlyph(char c) const
{
	auto idx = static_cast<size_t>(c);
	if(idx >= _font->Glyphs.size())
	{
		std::cerr << "Error: Glyph " << c << " (" << idx <<  ") not in font." << std::endl;
		return _font->Glyphs[0];
	}
	return _font->Glyphs[idx];
}

void GUIText::draw(const glm::vec2& resolution, const glm::vec2& position)
{
	assert(_vao);
	
	auto p = c2p(position);
	
	// If we are using a function, check if the text has changed
	if(_func)
		setText(_func());
	
	Program& P = Resources::getProgram("TextRendering");
	if(!P)
	{
		P = Resources::loadProgram("TextRendering",
			Resources::load<VertexShader>("src/GLSL/Text/text_vs.glsl"),
			Resources::load<FragmentShader>("src/GLSL/Text/text_fs.glsl")
		);
	}
	P.use();
	P.setUniform("Resolution", resolution);
	P.setUniform("Position", p);
	
	_font->Tex->bind();
	_vao.bind();
	glDrawElements(GL_TRIANGLES, _triangles.size(),  GL_UNSIGNED_INT, 0);
	_vao.unbind();
	
	P.useNone();
}
