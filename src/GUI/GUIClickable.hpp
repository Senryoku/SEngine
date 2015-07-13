#pragma once

#include <ResourcesManager.hpp>
#include <BoundingShape.hpp>
#include <GUIElement.hpp>

class GUIClickable : public GUIElement
{
public:
	virtual ~GUIClickable() =default;
	
	inline void setActive(bool b = true)
	{
		_active = b;
	}
	
	virtual bool handleKey(int key, int scancode, int action, int mods)
	{
		return false;
	}
	
	virtual bool handleTextInput(unsigned int unicode)
	{
		return false;
	}
	
	virtual bool handleClick(const glm::vec2& coords, int button)
	{
		auto c = p2c(coords);
		if(_aabb.contains(c))
			return onClick(c, button);
		return false;
	}
	
	inline virtual bool onClick(const glm::vec2& coords, int button) { return false; };
	
	AABB<glm::vec2>& getAABB() { return _aabb; }
	const AABB<glm::vec2>& getAABB() const { return _aabb; }
	
	void drawAABB(const glm::vec2& resolution, const glm::vec2& position, const glm::vec4& color) const
	{
		auto& P = ResourcesManager::getInstance().getProgram("GUIRectangle");
		if(!P)
		{
			P = loadProgram("GUIRectangle",
					load<VertexShader>("src/GLSL/GUI/rect_vs.glsl"),
					load<GeometryShader>("src/GLSL/GUI/rect_gs.glsl"),
					load<FragmentShader>("src/GLSL/GUI/rect_fs.glsl")
				);
		}
	
		P.use();
		P.setUniform("Resolution", resolution);
		P.setUniform("Position", position);
		P.setUniform("Min", _aabb.min);
		P.setUniform("Max", _aabb.max);
		P.setUniform("Color", color);
		glDrawArrays(GL_POINTS, 0, 1); // Dummy draw call
		P.useNone();
	}
	
protected:
	bool				_active = false;
	
	AABB<glm::vec2>	_aabb;
};
