#pragma once

#include <glm/glm.hpp>

struct PointLight
{
	glm::vec3	position;
	float		range = 10.0f;
	glm::vec3	color;
	float		info = 1.0f;	// ~Lifetime
	
	PointLight(glm::vec3 p = glm::vec3{0.0f}, float r = 100.0f,
				glm::vec3 c = glm::vec3{1.0f}, float i = 0.0f) :
		position(p),
		range(r),
		color(c),
		info(i)
	{
	}
};
