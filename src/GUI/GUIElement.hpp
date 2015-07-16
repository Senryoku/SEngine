#pragma once

#include <glm/glm.hpp>

#include <BoundingShape.hpp>

class GUIElement
{
public:
	// Public Attributes
	glm::vec2			Position = glm::vec2(0.0); ///< Position relative to parent
	
	GUIElement() =default;
	virtual ~GUIElement() =default;
	
	inline AABB<glm::vec2>& getAABB() { return _aabb; }
	inline const AABB<glm::vec2>& getAABB() const { return _aabb; }
	
	/**
	 * Every GUIElement should override this method.
	 *
	 * @param resolution	Screen resolution (pixels)
	 * @param position	Origin (position of the parent)
	**/
	virtual void draw(const glm::vec2& resolution, const glm::vec2& position = glm::vec2(0.0)) =0;

	inline void setActive(bool b = true);
	
	virtual bool handleKey(int key, int scancode, int action, int mods);
	virtual bool handleTextInput(unsigned int unicode);
	virtual bool handleClick(const glm::vec2& coords, int button);

	virtual bool onClick(const glm::vec2& coords, int button);

	void drawAABB(const glm::vec2& resolution, const glm::vec2& position, const glm::vec4& color) const;

	/// From Parent Space to Child Space
	inline glm::vec2 p2c(const glm::vec2& v) const { return v - Position; }
	inline AABB<glm::vec2> p2c(const AABB<glm::vec2>& aabb) const { return aabb - Position; }
	/// From Child Space to Parent Space
	inline glm::vec2 c2p(const glm::vec2& v) const { return v + Position; }
	inline AABB<glm::vec2> c2p(const AABB<glm::vec2>& aabb) const { return aabb + Position; }
	
	
protected:
	bool				_active = false;
	AABB<glm::vec2>	_aabb;
};

// Inlined functions

inline void GUIElement::setActive(bool b)
{
	_active = b;
}