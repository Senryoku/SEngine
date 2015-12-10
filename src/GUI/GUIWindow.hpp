#pragma once

#include <vector>
#include <algorithm>

#include <ResourcesManager.hpp>
#include <GUIElement.hpp>

class GUIWindow : public GUIElement
{
public:
	GUIWindow() =default;
	
	~GUIWindow()
	{
		for(auto p : _elements)
			delete p;
	}
	
	template<typename Element>
	Element* add(Element* e)
	{
		if(!_elements.empty())
			e->Position = glm::vec2(0.0, 
					_elements.back()->Position.y + 
					_elements.back()->getAABB().max.y +
					-e->getAABB().min.y +
					_padding);
		_elements.push_back(e);
		//_aabb += e->c2p(e->getAABB());
		return e;
	}
	
	virtual bool handleKey(int key, int scancode, int action, int mods) override
	{
		for(auto p : _elements)
			if(p->handleKey(key, scancode, action, mods))
				return true;

		return false;
	}
	
	virtual bool handleTextInput(unsigned int unicode) override
	{
		for(auto p : _elements)
			if(p->handleTextInput(unicode))
				return true;

		return false;
	}
	
	virtual bool onClick(const glm::vec2& coords, int button) override
	{
		updateAABB(); /// @todo (TEMP) Move elsewhere

		if(_active)
		{		
			for(auto p : _elements)
				if(p->handleClick(coords, button))
				{
					if(_activeElement != nullptr)
						_activeElement->setActive(false);
					p->setActive(true);
					_activeElement = p;
					return true;
				}
			return false;
		} else {
			_active = true;
		}
		
		return true;
	}
	
	void updateAABB()
	{
		if(!_elements.empty())
			_aabb = _elements[0]->c2p(_elements[0]->getAABB());
		
		for(auto p : _elements)
			_aabb += p->c2p(p->getAABB());
		
		_aabb.min -= _padding;
		_aabb.max += _padding;
	}

	void draw(const glm::vec2& resolution, const glm::vec2& position = glm::vec2(0.0)) override
	{
		auto pos = c2p(position);
		
		drawAABB(resolution, pos, _color);
		
		if(_active)
			for(auto p : _elements)
				p->draw(resolution, pos);
	}
	
	float getPadding() const { return _padding; }
	
private:
	std::vector<GUIElement*>	_elements;
	
	GUIElement*		_activeElement = nullptr;
	
	float			_padding = 5.0;
	glm::vec4		_color = glm::vec4(1.0, 1.0, 1.0, 0.1);
};
