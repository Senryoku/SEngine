#pragma once

#include <functional>

#include <GUIElement.hpp>
#include <GUIText.hpp>

class GUICheckbox : public GUIElement
{
public:
	using ToggleFunc = std::function<bool(void)>;

	GUICheckbox(const std::string& str, bool* b) :
		_text(str),
		_bool(b)
	{
		_aabb = _text.getAABB();
		_aabb.min -= glm::vec2(4.0);
		_aabb.max += glm::vec2(4.0);
	}
	
	GUICheckbox(const std::string& str, ToggleFunc func) :
		_text(str),
		_func(func)
	{
		_aabb = _text.getAABB();
		_aabb.min -= glm::vec2(4.0);
		_aabb.max += glm::vec2(4.0);
		
		// Toggle twice, so _lastCall has the good value (yes, it's dirty.)
		/// @todo Change
		_lastCall = _func();
		_lastCall = _func();
	}
	
	bool onClick(const glm::vec2& coords, int button) override
	{
		if(_bool != nullptr)
			*_bool = !(*_bool);
		else if(_func)
			_lastCall = _func();
		else
			return false;
		
		return true;
	}
	
	void draw(const glm::vec2& resolution, const glm::vec2& position = glm::vec2(0.0)) override
	{
		auto p = c2p(position);
		
		glm::vec4 c;
		if((_bool != nullptr && *_bool) || (_func && _lastCall)) 
		{
			c = glm::vec4(0.1, 1.0, 0.0, 0.25);
		} else {
			c = glm::vec4(1.0, 0.1, 0.0, 0.25);
		}
		drawAABB(resolution, p, c);
		
		_text.draw(resolution, p);
	}
	
private:
	GUIText		_text;
	bool*			_bool = nullptr;
	
	ToggleFunc		_func;
	bool			_lastCall = false;
};
