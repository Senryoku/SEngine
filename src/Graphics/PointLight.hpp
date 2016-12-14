#pragma once

#include <glm/glm.hpp>

struct PointLight
{
	glm::vec3	position;
	float		range = 10.0f;
	glm::vec3	color;
	float		info = 1.0f;	// ~Lifetime
	
	PointLight(glm::vec3 p, float r,
				glm::vec3 c, float i) :
		position(p),
		range(r),
		color(c),
		info(i)
	{
	}
};
