#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/intersect.hpp>

#include <Mesh.hpp>
#include <Plane.hpp>

struct Ray
{
	glm::vec3	origin;
	glm::vec3	direction;
	
	inline glm::vec3 operator()(float depth) const
	{
		return origin + depth * direction;
	}
};

struct Sphere
{
	glm::vec3	center;
	float			radius;
};

inline bool traceSphere(const glm::vec3& origin, const glm::vec3& dir, const glm::vec3& center, float radius)
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

inline bool trace(const Ray& r, const Sphere& s, glm::vec3& p, glm::vec3& n)
{
	return glm::intersectRaySphere(r.origin, r.direction, s.center, s.radius, p, n);
}

inline bool trace(Ray r, Plane p, float& d)
{
	return glm::intersectRayPlane(r.origin, r.direction, p.getPoint(), p.getNormal(), d);
}

inline bool trace(const Ray& r, const Plane& pl, float& d, glm::vec3& p, glm::vec3& n)
{
	float tmpD = d;
	if(glm::intersectRayPlane(r.origin, r.direction, pl.getPoint(), pl.getNormal(), tmpD) 
		&& tmpD > 0.0 && tmpD < d)
	{
		d = tmpD;
		p = r(d);
		n = pl.getNormal();
		return true;
	}
	return false;
}

inline bool traceSphere(const glm::vec3& origin, const glm::vec3& dir, const glm::vec3& center, float radius, float& depth)
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

inline bool trace(const Ray& r, const Sphere& s)
{
	return traceSphere(r.origin, r.direction, s.center, s.radius);
}

inline bool trace(const Ray& r, const Sphere& s, float& depth)
{
	return traceSphere(r.origin, r.direction, s.center, s.radius, depth);
}

inline bool trace(const Ray& r, const Sphere& s, float& depth, glm::vec3& p, glm::vec3& n)
{
	if(trace(r, s, depth))
	{
		p = r(depth);
		n = (p - s.center)/s.radius;
		return true;
	}
	return false;
}

inline bool trace(const Ray& r, const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, glm::vec3& p)
{
	glm::vec3 tmp;
	if(glm::intersectRayTriangle(r.origin, r.direction, p0, p1, p2, tmp))
	{
		p = r(tmp.z);
		return true;
	}
	return false;
}

inline bool trace(const Ray& r, const AABB<glm::vec3>& b)
{
	const glm::vec3 dir_inv = glm::vec3(1.0) / r.direction;
    float t1 = (b.min[0] - r.origin[0]) * dir_inv[0];
    float t2 = (b.max[0] - r.origin[0]) * dir_inv[0];
 
    float tmin = glm::min(t1, t2);
    float tmax = glm::max(t1, t2);
 
    for (int i = 1; i < 3; ++i)
    {
        t1 = (b.min[i] - r.origin[i]) * dir_inv[i];
        t2 = (b.max[i] - r.origin[i]) * dir_inv[i];
 
        tmin = glm::max(tmin, glm::min(t1, t2));
        tmax = glm::min(tmax, glm::max(t1, t2));
    }
 
    return tmax > glm::max(tmin, 0.0f);
}

inline bool trace(const Ray& r, const Mesh& m, glm::vec3& p, glm::vec3& n)
{
	if(!trace(r, m.getBoundingBox()))
		return false;
	
	for(auto& t : m.getTriangles())
	{
		if(glm::intersectRayTriangle(r.origin,
						r.direction, 
						m.getVertices()[t.vertices[0]].position,
						m.getVertices()[t.vertices[1]].position,
						m.getVertices()[t.vertices[2]].position,
						p))
		{
			n = p.x * m.getVertices()[t.vertices[0]].normal +
					p.y * m.getVertices()[t.vertices[1]].normal +
					(1.0f - p.y - p.z) * m.getVertices()[t.vertices[2]].normal;
			p = r(p.z);
			return true;
		}
	}
	
	return false;
}

inline bool trace(const Ray& r, const Mesh& m, float& depth, glm::vec3& p, glm::vec3& n)
{
	bool b = trace(r, m, p, n);
	if(b)
		depth = glm::distance(r.origin, p);
	return b;
}
