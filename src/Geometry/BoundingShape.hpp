#pragma once

#include <glm/glm.hpp>
#define GLM_FORCE_RADIANS
#include <glm/gtc/type_ptr.hpp> // glm::value_ptr

#include <MathTools.hpp>

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
inline bool contains(const AABB<glm::vec2>& rhs, const glm::vec2& lhs);
inline bool contains(const AABB<glm::vec3>& rhs, const glm::vec3& lhs);

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
	
	inline bool contains(const Vector& v) const
	{
		return ::contains(*this, v);
	}
	
	inline AABB<Vector>& operator+=(const Vector& rhs)
	{
		min += rhs;
		max += rhs;
		return *this;
	}
	
	inline AABB<Vector>& operator+=(const AABB<Vector>& rhs)
	{
		min = glm::min(min, rhs.min);
		max = glm::max(max, rhs.max);
		return *this;
	}
	
	inline AABB<Vector>& operator*=(float factor)
	{
		Vector tmp = 0.5f * (max + min);
		min = tmp + (min - tmp) * factor;
		max = tmp + (max - tmp) * factor;
		return *this;
	}
	
	inline AABB<Vector>& operator*=(const Vector& factors)
	{
		Vector tmp = 0.5f * (max + min);
		min = tmp + (min - tmp) * factors;
		max = tmp + (max - tmp) * factors;
		return *this;
	}

	// Attributes
	Vector	min;
	Vector	max;
private:
};

inline bool intersect(const AABB<glm::vec2>& lhs, const AABB<glm::vec2>& rhs)
{
	return !(lhs.min.x > rhs.max.x || lhs.min.y > rhs.max.y ||
			 lhs.max.x < rhs.min.x || lhs.max.y < rhs.min.y);
}

inline bool intersect(const AABB<glm::vec3>& lhs, const AABB<glm::vec3>& rhs)
{
	return !(lhs.min.x > rhs.max.x || lhs.min.y > rhs.max.y || lhs.min.z > rhs.max.z ||
			 lhs.max.x < rhs.min.x || lhs.min.y > rhs.max.y || lhs.max.z < rhs.min.z );
}

inline bool contains(const AABB<glm::vec2>& lhs, const glm::vec2& rhs)
{
	return !(lhs.min.x > rhs.x || lhs.min.y > rhs.y ||
			 lhs.max.x < rhs.x || lhs.max.y < rhs.y);
}

inline bool contains(const AABB<glm::vec3>& lhs, const glm::vec3& rhs)
{
	return !(lhs.min.x > rhs.x || lhs.min.y > rhs.y || lhs.min.z > rhs.z ||
			 lhs.max.x < rhs.x || lhs.min.y > rhs.y || lhs.max.z < rhs.z );
}

template<typename Vector>
inline AABB<Vector> operator+(const AABB<Vector>& lhs, const Vector& rhs)
{
	return AABB<Vector>(lhs.min + rhs, lhs.max + rhs);
}

template<typename Vector>
inline AABB<Vector> operator-(const AABB<Vector>& lhs, const Vector& rhs)
{
	return AABB<Vector>(lhs.min - rhs, lhs.max - rhs);
}

template<typename Vector>
inline AABB<Vector> operator+(const AABB<Vector>& lhs, const AABB<Vector>& rhs)
{
	return AABB<Vector>(glm::min(lhs.min, rhs.min), glm::max(lhs.max, rhs.max));
}

inline bool intersect(const AABB<glm::vec3>& rhs, const BoundingSphere& lhs)
{
	float dist_squared = lhs.radius * lhs.radius;
	if (lhs.center.x < rhs.min.x) dist_squared -= sqr(lhs.center.x - rhs.min.x);
	else if (lhs.center.x > rhs.max.x) dist_squared -= sqr(lhs.center.x - rhs.max.x);
	if (lhs.center.y < rhs.min.y) dist_squared -= sqr(lhs.center.y - rhs.min.y);
	else if (lhs.center.y > rhs.max.y) dist_squared -= sqr(lhs.center.y - rhs.max.y);
	if (lhs.center.z < rhs.min.z) dist_squared -= sqr(lhs.center.z - rhs.min.z);
	else if (lhs.center.z > rhs.max.z) dist_squared -= sqr(lhs.center.z - rhs.max.z);
	return dist_squared > 0;
}
