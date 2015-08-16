#pragma once

#include <glm/glm.hpp>

struct Ray
{
	glm::vec3	origin;
	glm::vec3	direction;
};

struct Sphere
{
	glm::vec3	center;
	float		radius;
};

inline bool traceSphere(glm::vec3 origin, glm::vec3 dir, glm::vec3 center, float radius)
{	
    glm::vec3 d = origin - center;
	
	float a = glm::dot(dir, dir);
	float b = glm::dot(dir, d);
	float c = glm::dot(d, d) - radius * radius;
	
	float g = b * b - a * c;
	
	if(g > 0.0)
	{
		float dis = (-std::sqrt(g) - b) / a;
		if(dis > 0.0 && dis < 1000.0)
			return true;
	}
	return false;
}

inline bool traceSphere(glm::vec3 origin, glm::vec3 dir, glm::vec3 center, float radius, float& depth)
{	
    glm::vec3 d = origin - center;
	
	float a = glm::dot(dir, dir);
	float b = glm::dot(dir, d);
	float c = glm::dot(d, d) - radius * radius;
	
	float g = b * b - a * c;
	
	if(g > 0.0)
	{
		float dis = (-std::sqrt(g) - b) / a;
		if(dis > 0.0 && dis < depth)
		{
			depth = dis;
			return true;
		}
	}
	return false;
}

inline bool trace(Ray r, Sphere s)
{
	return traceSphere(r.origin, r.direction, s.center, s.radius);
}

inline bool trace(Ray r, Sphere s, float& depth)
{
	return traceSphere(r.origin, r.direction, s.center, s.radius, depth);
}
