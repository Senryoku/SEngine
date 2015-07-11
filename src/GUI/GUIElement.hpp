#pragma once

#include <glm/glm.hpp>

class GUIElement
{
public:
	virtual ~GUIElement() =default;
	
	const glm::vec2& getPosition() const { return _position; }
	void setPosition(const glm::vec2& p) { _position = p; }
	
	/**
	 * @param resolution	Screen resolution (pixels)
	 * @param position	Origin (position of the parent)
	**/
	virtual void draw(const glm::vec2& resolution, const glm::vec2& position = glm::vec2(0.0)) const =0;
	
	/// From Parent Space to Child Space
	inline glm::vec2 p2c(const glm::vec2& v) const { return v - _position; }
	inline AABB<glm::vec2> p2c(const AABB<glm::vec2>& aabb) const { return aabb - _position; }
	/// From Child Space to Parent Space
	inline glm::vec2 c2p(const glm::vec2& v) const { return v + _position; }
	inline AABB<glm::vec2> c2p(const AABB<glm::vec2>& aabb) const { return aabb + _position; }

public:
	glm::vec2			_position = glm::vec2(0.0); ///< Position relative to parent
};
