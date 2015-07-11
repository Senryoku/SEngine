#pragma once

#include <vector>

#include <ResourcesManager.hpp>
#include <GUIClickable.hpp>
#include <GUIElement.hpp>

class GUIWindow : public GUIClickable
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
		_elements.push_back(e);
		_aabb += e->getAABB();
		return e;
	}
	
	bool onClick(const glm::vec2& coords, int button) override
	{
		updateAABB();
		
		for(auto p : _elements)
			if(p->handleClick(coords, button))
				return true;
		
		return false;
	}
	
	void setActive(bool b = true)
	{
		_active = b;
	}
	
	void updateAABB()
	{
		if(!_elements.empty())
			_aabb = _elements[0]->c2p(_elements[0]->getAABB());
		
		for(auto p : _elements)
			_aabb += p->c2p(p->getAABB());
	}

	void draw(const glm::vec2& resolution, const glm::vec2& position = glm::vec2(0.0)) const override
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
		P.setUniform("Position", position + _position);
		P.setUniform("Min", _aabb.min);
		P.setUniform("Max", _aabb.max);
		glDrawArrays(GL_POINTS, 0, 1); // Dummy draw call
		P.useNone();
		
		if(_active)
			for(auto p : _elements)
				p->draw(resolution, _position + position);
	}
	
private:
	std::vector<GUIClickable*>	_elements;
	
	bool							_active = false;
};
