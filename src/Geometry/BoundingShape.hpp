#pragma once

#include <glm/glm.hpp>
#define GLM_FORCE_RADIANS
#include <glm/gtc/type_ptr.hpp> // glm::value_ptr

/**
 * Base class for all kinds of Bounding Shapes
**/
class BoundingShape
{
public:
	BoundingShape() =default;
	virtual ~BoundingShape() =default;
};

class BoundingSphere : public BoundingShape
{
public:
	BoundingSphere() =default;
	BoundingSphere(const glm::vec3& _center, float _radius) :
		center(_center),
		radius(_radius)
	{
	}
	
	glm::vec3	center;
	float		radius;
};

/**
 * BoundingBox
 * @todo Change to a general bounding box (not an AABB) : 8 Points.
**/
class BoundingBox : public BoundingShape
{
public:
	BoundingBox() =default;
	BoundingBox(const glm::vec3& _min, const glm::vec3& _max) :
		min(_min),
		max(_max)
	{
	}
	
	glm::vec3	min;
	glm::vec3	max;
};

// Forward Declarations
template<typename Vector>
class AABB;
inline bool intersect(const AABB<glm::vec2>& rhs, const AABB<glm::vec2>& lhs);
inline bool intersect(const AABB<glm::vec3>& rhs, const AABB<glm::vec3>& lhs);

/**
 * Aligned Axis Bounding Box
**/
template<typename Vector = glm::vec3>
class AABB : public BoundingShape
{
public:
	using vector_type = Vector;

	AABB() =default;
	
	AABB(const Vector& _min, const Vector& _max) :
		min(_min),
		max(_max)
	{
	}
	
	inline AABB operator*(float factor) const
	{
		return AABB(factor * min, factor * max);
	}
	
	inline bool intersect(const AABB<Vector>& other) const
	{
		return ::intersect(*this, other);
	}
	
	// Attributes
	Vector	min;
	Vector	max;
private:
};

inline bool intersect(const AABB<glm::vec2>& rhs, const AABB<glm::vec2>& lhs)
{
	return !(rhs.min.x > lhs.max.x || rhs.min.y > lhs.max.y ||
			 rhs.max.x < lhs.min.x || rhs.max.y < lhs.min.y);
}

inline bool intersect(const AABB<glm::vec3>& rhs, const AABB<glm::vec3>& lhs)
{
	return !(rhs.min.x > lhs.max.x || rhs.min.y > lhs.max.y || rhs.min.z > lhs.max.z ||
			 rhs.max.x < lhs.min.x || rhs.min.y > lhs.max.y || rhs.max.z < lhs.min.z );
}
