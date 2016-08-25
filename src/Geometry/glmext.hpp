#pragma once

#include <ostream>
#include <glm/glm.hpp>

template<typename T>
inline float point_line_distance(const T& p, const T& l0, const T& l1)
{
	auto l = glm::normalize(l1 - l0);
	auto proj = l0 + glm::dot(l, p - l0) * l;
	return glm::distance(p, proj);
}

inline std::ostream& operator<<(std::ostream& out, const glm::vec2& v)
{
	out << "[" << v.x << ", " << v.y << "]";
	return out;
}

inline std::ostream& operator<<(std::ostream& out, const glm::vec3& v)
{
	out << "[" << v.x << ", " << v.y << ", " << v.z << "]";
	return out;
}
