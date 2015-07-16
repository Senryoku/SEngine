#pragma once

#include <functional>

#include <GUIElement.hpp>
#include <GUIText.hpp>

class GUIButton : public GUIElement
{
public:
	using ClickFunc = std::function<void(void)>;
	
	GUIButton(const std::string& str, ClickFunc func) :
		_text(str),
		_func(func)
	{
		_aabb = _text.getAABB();
		_aabb.min -= glm::vec2(4.0);
		_aabb.max += glm::vec2(4.0);
	}
	
	bool onClick(const glm::vec2& coords, int button) override
	{
		if(_func)
			_func();
		else
			return false;
		
		return true;
	}
	
	void draw(const glm::vec2& resolution, const glm::vec2& position = glm::vec2(0.0)) override
	{
		auto p = c2p(position);
		drawAABB(resolution, p,  glm::vec4(0.0, 0.3, 1.0, 0.25));
		
		_text.draw(resolution, p);
	}
	
private:
	GUIText		_text;
	ClickFunc		_func;
};
