#pragma once

#include <functionnal>

#include <Clickable.hpp>
#include <GUIText.hpp>

class GUIButton : public GUIClickable
{
public:
	using ToggleFunc = std::function<void(void)>;

	GUIButton(const std::string& str, bool* b) :
		_text(str),
		_bool(b)
	{
	}
	
	GUIButton(const std::string& str, ToggleFunc func) :
		_text(str),
		_func(func)
	{
	}
	
	void onClick(const glm::vec2& coords, int button) override
	{
		if(_bool != nullptr)
			*_bool = !(*_bool);
		else
			_func()
	}
	
	void draw(const glm::vec2& resolution, const glm::vec2& position = glm::vec2(0.0)) const override
	{
		_text.draw(resolution, position + _position);
	}
	
private:
	GUIText		_text;
	bool*			_bool = nullptr;
	ToggleFunc		_func;
};
