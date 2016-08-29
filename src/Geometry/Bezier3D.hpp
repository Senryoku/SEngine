#pragma once

#include <vector>

#include <glm/glm.hpp>

inline size_t fact(size_t n)
{
	size_t r = 1;
	for(size_t i = 2; i <= n; i++)
		r *= i;
	return r;
}

inline size_t binomial(size_t n, size_t k)
{
	return (k <= n) ? fact(n)/(fact(k)*fact(n - k)) : 0;
}

inline float berstein(size_t m, size_t i, float u)
{
	return binomial(m, i)*pow(u, i)*pow(1 - u, m - i);
}

class Bezier3D
{
public:
	Bezier3D(const std::vector<glm::vec3>& Data, size_t nbPoints = 100) :
		_controlPoints(Data)
	{
		compute(nbPoints);
	}
	
	~Bezier3D() =default;

	void compute(size_t nbPoints = 100)
	{
		_points.clear();
		_points.reserve(nbPoints);
		for(size_t i(0); i < nbPoints; i++)
		{
			glm::vec3 P(0, 0, 0);
			for(size_t j(0); j < _controlPoints.size(); j++)
				P += _controlPoints[j] * berstein(_controlPoints.size() - 1, j,
					static_cast<float>(i)/static_cast<float>(nbPoints));
			_points.push_back(P);
		}
	}
	
	inline const std::vector<glm::vec3>& getControlPoints() { return _controlPoints; }

	inline size_t getControlPointCount() { return _controlPoints.size(); }
	inline size_t getPointCount() { return _points.size(); }

	inline glm::vec3 operator[](size_t nPoint) { return _points[std::min(nPoint, _points.size() - 1)]; }

	inline Bezier3D& operator+=(glm::vec3 V) { _controlPoints.push_back(V); return *this;}
	
	inline void reset() { _controlPoints.clear(); }
	
	inline glm::vec3 getDelta(size_t nPoint) {
		if(nPoint >= _points.size() || nPoint == 0)
			return glm::vec3(0, 0, 0);
		return _points[nPoint] - _points[nPoint - 1];
	}

	inline glm::vec3 get(float t)
	{
		if(t < 0.f) t = 0.f;
		while(t > 1.f) t -= 1.f;
		float t2 = t * _points.size();
		size_t idx = static_cast<size_t>(t2);
		const auto& p0 = _points[idx];
		const auto& p1 = _points[idx + 1];
		float t3 = (t2 - idx);
		return (1.0f - t3) * p0 + t3 * p1;
	}
	inline glm::vec3 operator()(float t) { return get(t); }
	
private:
	std::vector<glm::vec3> _controlPoints;
	std::vector<glm::vec3> _points;
};
