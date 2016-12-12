#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/intersect.hpp>

#include <Mesh.hpp>
#include <MeshRenderer.hpp>
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
	float		radius;
};

inline bool traceSphere(const glm::vec3& origin, const glm::vec3& dir, const glm::vec3& center, float radius);
inline bool traceSphere(const glm::vec3& origin, const glm::vec3& dir, const glm::vec3& center, float radius, float& depth);

inline bool trace(const Ray& r, const Sphere& s, glm::vec3& p, glm::vec3& n);
inline bool trace(const Ray& r, const Plane& p, float& d);
inline bool trace(const Ray& r, const Plane& pl, float& d, glm::vec3& p, glm::vec3& n);
inline bool trace(const Ray& r, const Sphere& s);
inline bool trace(const Ray& r, const Sphere& s, float& depth);
inline bool trace(const Ray& r, const Sphere& s, float& depth, glm::vec3& p, glm::vec3& n);
inline bool trace(const Ray& r, const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, glm::vec3& p);
inline bool trace(const Ray& r, const AABB<glm::vec3>& b);
inline bool trace(const Ray& r, const AABB<glm::vec3>& b, float& t);
inline bool trace(const Ray& r, const MeshRenderer& o, float& t);
inline bool trace(const Ray& r, const Mesh& m, glm::vec3& p, glm::vec3& n);
inline bool trace(const Ray& r, const Mesh& m, float& depth, glm::vec3& p, glm::vec3& n);

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

inline bool trace(const Ray& r, const Plane& p, float& d)
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

inline bool trace(const Ray& r, const AABB<glm::vec3>& b, float& t)
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
 
	float tmp = tmin > 0.0 ? tmin : tmax;
    if(tmax > glm::max(tmin, 0.0f) && tmp < t)
	{
		t = tmp;
		return true;
	} else {
		return false;
	}
}

inline bool trace(const Ray& r, const MeshRenderer& o, float& t)
{
	float tmp_depth = t;
	if(!trace(r, o.getAABB(), tmp_depth))
		return false;
	
	bool hit = false;
	glm::vec3 tmp;
	glm::vec3 triangle[3];
	for(const auto& tr : o.getMesh().getTriangles())
	{
		for(int i = 0; i < 3; ++i)
			triangle[i] = glm::vec3{o.getTransformation().getGlobalMatrix() * glm::vec4{o.getMesh().getVertices()[tr.vertices[i]].position, 1.0}};
		if(glm::intersectRayTriangle(r.origin,
						r.direction, 
						triangle[0],
						triangle[1],
						triangle[2],
						tmp))
		{
			if(tmp.z > 0.0 && tmp.z < t)
			{
				t = tmp.z;
				hit = true;
			}
		}
	}
	
	return hit;
}

inline bool trace(const Ray& r, const Mesh& m, glm::vec3& p, glm::vec3& n)
{
	if(!trace(r, m.getBoundingBox()))
		return false;
	
	bool hit = false;
	glm::vec3 tmp;
	for(auto& t : m.getTriangles())
	{
		if(glm::intersectRayTriangle(r.origin,
						r.direction, 
						m.getVertices()[t.vertices[0]].position,
						m.getVertices()[t.vertices[1]].position,
						m.getVertices()[t.vertices[2]].position,
						tmp))
		{
			p = r(tmp.z);
			n = tmp.x * m.getVertices()[t.vertices[0]].normal +
					tmp.y * m.getVertices()[t.vertices[1]].normal +
					(1.0f - tmp.y - tmp.z) * m.getVertices()[t.vertices[2]].normal;
			hit = true;
		}
	}
	
	return hit;
}

inline bool trace(const Ray& r, const Mesh& m, float& depth, glm::vec3& p, glm::vec3& n)
{
	if(!trace(r, m.getBoundingBox()))
		return false;
	
	bool hit = false;
	glm::vec3 tmp;
	for(auto& t : m.getTriangles())
	{
		if(glm::intersectRayTriangle(r.origin,
						r.direction, 
						m.getVertices()[t.vertices[0]].position,
						m.getVertices()[t.vertices[1]].position,
						m.getVertices()[t.vertices[2]].position,
						tmp))
		{
			if(tmp.z < depth)
			{
				depth = tmp.z;
				p = r(tmp.z);
				n = tmp.x * m.getVertices()[t.vertices[0]].normal +
						tmp.y * m.getVertices()[t.vertices[1]].normal +
						(1.0f - tmp.x - tmp.y) * m.getVertices()[t.vertices[2]].normal;
				n = glm::normalize(n);
				hit = true;
			}
		}
	}
	
	return hit;
}
