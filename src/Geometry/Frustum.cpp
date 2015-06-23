#include <Frustum.hpp>

#include <MathTools.hpp>

void Frustum::setPerspective(float angle, float ratio, float znear, float zfar)
{
	_angle = angle;
	_ratio = ratio;
	_znear = znear;
	_zfar = zfar;
	
	float tang = std::tan(pi()/180.f * angle * 0.5f);
	_nearHeight = _znear * tang;
	_nearWidth = _nearHeight * ratio;
	_farHeight = _zfar * tang;
	_farWidth = _farHeight * ratio;
	
	_changed = true;
}

void Frustum::setLookAt(const glm::vec3& pos, const glm::vec3& dir, const glm::vec3& up)
{
	glm::vec3 nearCenter, farCenter, X, Y, Z;
	
	Z = glm::normalize(-dir);
	X = glm::normalize(glm::cross(up, Z));
	Y = glm::cross(Z, X);
	
	nearCenter = pos - _znear * Z;
	farCenter = pos - _zfar * Z;
	_planes[Face::Near].set(nearCenter, -Z);
	_planes[Face::Far].set(farCenter, Z);

	glm::vec3 aux, normal;

	aux = glm::normalize((nearCenter + _nearHeight * Y) - pos);
	normal = glm::cross(aux, X);
	_planes[Face::Top].set(nearCenter + Y * _nearHeight, normal);

	aux = glm::normalize((nearCenter - _nearHeight * Y) - pos);
	normal = glm::cross(X, aux);
	_planes[Face::Bottom].set(nearCenter - Y * _nearHeight, normal);
	
	aux = glm::normalize((nearCenter - _nearWidth * X) - pos);
	normal = glm::cross(aux, Y);
	_planes[Face::Left].set(nearCenter - X * _nearWidth, normal);

	aux = glm::normalize((nearCenter + _nearWidth * X) - pos);
	normal = glm::cross(Y, aux);
	_planes[Face::Right].set(nearCenter + _nearWidth * X, normal);
	
	_changed = true;
}

///< Testing against a sphere
bool Frustum::isIntersecting(const glm::vec3& center, float radius) const
{
	float distance;
	for(int i = 0; i < 6; ++i)
	{
		// Signed distance
		distance = glm::dot(center - _planes[i].getPoint(), _planes[i].getNormal());
		if (distance < -radius)
			return false;
		//else if (distance < radius) // Sphere is on the edge of the Frustum
		//	... 
	}
	return true;
}
