#pragma once

#include <array>

#include <Plane.hpp>

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
	~Frustum() =default;
	
	void setPerspective(float angle, float ratio, float znear, float zfar);
	void setLookAt(const glm::vec3& pos, const glm::vec3& dir, const glm::vec3& up);
	
	/// Testing against a sphere
	bool isIntersecting(const glm::vec3& center, float radius) const;
	
	bool changed() 
	{
		if(_changed)
		{
			_changed = false;
			return true;
		}
		return false;
	}
private:
	float	_angle,
			_ratio,
			_znear,
			_zfar,
			_nearWidth,
			_nearHeight,
			_farWidth,
			_farHeight;
			
	std::array<Plane, 6>	_planes;
	bool 							_changed = true;
};
