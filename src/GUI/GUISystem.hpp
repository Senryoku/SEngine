#pragma once

#include <vector>
#include <iostream>

#include <GUIWindow.hpp>

class GUISystem
{
public:
	GUISystem() =default;
	
	~GUISystem()
	{
		for(auto p : _windows)
			delete p;
	}
	
	GUIWindow* add(GUIWindow* w)
	{
		_windows.push_back(w);
		return w;
	}

	/**
	 * Will probably remain useless for a long time.
	**/
	bool handleKey(int key, int scancode, int action, int mods)
	{
		return false;
	}

	bool handleClick(glm::vec2 coords, int button)
	{
		std::cout << coords.x << " " << coords.y << std::endl;
		
		if(_activeWindow != nullptr)
		{
			if(_activeWindow->getAABB().contains(coords))
			{
				return _activeWindow->onClick(coords, button);
			} else {
				std::cout << "Lost Focus." << std::endl;
				_activeWindow->setActive(false);
				_activeWindow = nullptr;
			}
		}
		
		for(auto p : _windows)
		{
			if(p->getAABB().contains(coords))
			{
				std::cout << "Clicked on a window !" << std::endl;
				_activeWindow = p;
				_activeWindow->setActive();
				break;
			}
		}
		
		return false;
	}

	void handleTextInput(unsigned int unicode) {}
	
	void draw(const glm::vec2& resolution, const glm::vec2& position = glm::vec2(0.0)) const
	{
		for(auto p : _windows)
			p->draw(resolution, position);
	}
	
private:
	std::vector<GUIWindow*>	_windows;
	
	GUIWindow*					_activeWindow = nullptr;
};
