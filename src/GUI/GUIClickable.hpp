#pragma once

#include <BoundingShape.hpp>
#include <GUIElement.hpp>

class GUIClickable : public GUIElement
{
public:
	virtual ~GUIClickable() =default;
	
	inline bool handleClick(const glm::vec2& coords, int button)
	{
		auto c = p2c(coords);
		if(_aabb.contains(c))
			return onClick(c, button);
		return false;
	}
	
	inline virtual bool onClick(const glm::vec2& coords, int button) { return false; };
	
	const AABB<glm::vec2>& getAABB() const { return _aabb; }
	
protected:
	AABB<glm::vec2>	_aabb;
};
