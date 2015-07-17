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
		w->getAABB() = {{0.0, 0.0}, {10.0, 10.0}};
		w->Position = {10.0 + 20.0 * _windows.size(), 10.0};
		_windows.push_back(w);
		return w;
	}

	bool handleKey(int key, int scancode, int action, int mods)
	{
		if(_activeWindow != nullptr)
		{
			if(_activeWindow->handleKey(key, scancode, action, mods))
				return true;
			
			if(key == 256) // Escape, GLFW_KEY_ESCAPE
			{
				closeActiveWindow();
				return true;
			}
		}
		
		return false;
	}

	bool handleClick(glm::vec2 coords, int button)
	{		
		if(_activeWindow != nullptr)
		{
			if(!_activeWindow->getAABB().contains(_activeWindow->p2c(coords)))
			{
				closeActiveWindow();
			} else {
				return _activeWindow->handleClick(coords, button);
			}
		}
		
		for(auto p : _windows)
		{
			if(p->handleClick(coords, button))
			{
				_activeWindow = p;
				_activeWindow->setActive();
				_activeWindow->Position = {25.0, 35.0};
				break;
			}
		}
		
		return false;
	}

	bool handleTextInput(unsigned int unicode)
	{
		if(_activeWindow != nullptr)
			return _activeWindow->handleTextInput(unicode);
		
		return false;
	}
	
	void draw(const glm::vec2& resolution, const glm::vec2& position = glm::vec2(0.0)) const
	{
		for(auto p : _windows)
			p->draw(resolution, position);
	}
	
private:
	std::vector<GUIWindow*>	_windows;
	
	GUIWindow*					_activeWindow = nullptr;
	
	size_t getIndex(GUIWindow* w) const
	{
		return std::distance(_windows.begin(), std::find(_windows.begin(), _windows.end(), w));
	}
	
	void closeActiveWindow()
	{
		_activeWindow->setActive(false);
		_activeWindow->getAABB() = {{0.0, 0.0}, {10.0, 10.0}};
		_activeWindow->Position = {10.0 + 20.0 * getIndex(_activeWindow), 10.0};
		_activeWindow = nullptr;
	}
};
