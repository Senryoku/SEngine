#pragma once

#include <array>

#include <Plane.hpp>
#include <BoundingShape.hpp>

class Frustum
{
public:
	enum Face
	{
		Top,
		Bottom,
		Left,
		Right,
		Near,
		Far
	};
	
	Frustum() =default;
	Frustum(const glm::mat4& projmat);
	
	bool isIntersecting(const AABB<glm::vec3>& aabb) const;

	std::array<Plane, 6>	planes;
};
