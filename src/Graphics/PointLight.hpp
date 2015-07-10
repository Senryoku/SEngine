#pragma once

struct PointLight
{
	glm::vec3	position;
	float		range = 10.0f;
	glm::vec3	color;
	float		dummy = 0.0f;
	
	PointLight(glm::vec3 p, float r,
				glm::vec3 c, float d) :
		position(p),
		range(r),
		color(c),
		dummy(d)
	{
	}
};
