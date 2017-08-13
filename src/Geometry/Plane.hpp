#pragma once

#include <cassert>
#include <array>
#include <glm/glm.hpp>

class Plane
{
public:
	Plane() =default;

	Plane(float a, float b, float c, float d)
	{
		setCoefficients(a, b, c, d);
	}
	
	Plane(const glm::vec3& point, const glm::vec3& normal) :
		_point(point),
		_normal(glm::normalize(normal))
	{
		updateCoefficients();
	}
	
	inline const glm::vec3& getPoint() const { return _point; }
	inline const glm::vec3& getNormal() const { return _normal; }
	
	inline void set(const glm::vec3& p, const glm::vec3& n) { _point = p; _normal = n; updateCoefficients(); }
	inline void setPoint(const glm::vec3& v) { _point = v; updateCoefficients(); }
	inline void setNormal(const glm::vec3& v) { _normal = glm::normalize(v); updateCoefficients(); }
	
	inline void setCoefficients(float a, float b, float c, float d)
	{
		_coefficients = {a, b, c, d};
		normalizeCoefficients();
		updateNormalPoint();
	}
	
	inline void setCoefficients(const std::array<float, 4>& c)
	{
		_coefficients = c;
		normalizeCoefficients();
		updateNormalPoint();
	}
	
	float operator[](unsigned int i) const
	{
		assert(i < 4);
		return _coefficients[i];
	}
	
private:
	glm::vec3	_point;
	glm::vec3	_normal;
	
	void updateNormalPoint()
	{
		_normal = {_coefficients[0], _coefficients[1], _coefficients[2]};
		_point = {_coefficients[3] / _coefficients[0], 0.0f, 0.0f};
	}
	
	std::array<float, 4>	_coefficients;
	void updateCoefficients()
	{
		_coefficients[0] = _normal[0];
		_coefficients[1] = _normal[1];
		_coefficients[2] = _normal[2];
		_coefficients[3] = glm::dot(_normal, _point);
	}
	
	void normalizeCoefficients()
	{
		float n = std::sqrt(_coefficients[0] * _coefficients[0]
			              + _coefficients[1] * _coefficients[1]
			              + _coefficients[2] * _coefficients[2]);
		for(int i = 0; i < 4; ++i)
			_coefficients[i] /= n;
	}
};
