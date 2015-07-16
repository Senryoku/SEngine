#pragma once

#include <GUIElement.hpp>
#include <GUILine.hpp>
#include <GUIWindow.hpp>

class GUISeparator : public GUIElement
{
public:
	GUISeparator(GUIWindow* window) :
		_window(window)
	{
	}

	void draw(const glm::vec2& resolution, const glm::vec2& position = glm::vec2(0.0)) override
	{
		if(_window && (_window->getAABB().max.x - _window->getPadding() != _size))
			setSize(_window->getAABB().max.x - _window->getPadding());
		
		auto p = c2p(position);
		
		_line.draw(resolution, p);
	}

private:
	float						_size = 100.0f;
	GUIWindow*					_window = nullptr; ///< If not nullptr, used to scale the line.
	
	GUILine					_line;
	
	void setSize(float size)
	{
		_size = size;
		updateAABB();
		_line.getVertices().resize(2);
		_line.getVertices()[0] = {0.0f, 0.0f};
		_line.getVertices()[1] = {_size, 0.0f};
		_line.update(Buffer::Usage::StaticDraw);
	}

	void updateAABB()
	{
		_aabb = {{0.0f, -5.0f},
				{_size, 5.0f}};
	}
};