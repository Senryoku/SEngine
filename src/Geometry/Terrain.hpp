#pragma once

#include <Mesh.hpp>

class Terrain
{
public:
	virtual double operator()(double x, double y) const =0;
};

Mesh create(const Terrain& t, const glm::vec2& start, const glm::vec2& end, const glm::ivec2& precision);
