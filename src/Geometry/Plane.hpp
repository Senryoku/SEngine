#pragma once

#include <glm/glm.hpp>

class Plane
{
public:
	Plane() =default;

	Plane(const glm::vec3& point, const glm::vec3& normal) :
		_point(point),
		_normal(normal)
	{
	}
	
	inline const glm::vec3& getPoint() const { return _point; }
	inline const glm::vec3& getNormal() const { return _normal; }
	
	inline void set(const glm::vec3& p, const glm::vec3& n) { _point = p; _normal = n; }
	inline void setPoint(const glm::vec3& v) { _point = v; }
	inline void setNormal(const glm::vec3& v) { _normal = v; }
	
private:
	glm::vec3	_point;
	glm::vec3	_normal;
};
