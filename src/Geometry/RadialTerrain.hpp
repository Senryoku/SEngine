#pragma once

#include <glm/glm.hpp>
#include <forward_list>

#include <Terrain.hpp>
#include <ElevationFunction.hpp>

class RadialTerrain : public Terrain
{
public:
	using ElevationFunction = std::function<double(double, double)>;
	
	class Zone : public Terrain
	{
	public:
		Zone(const ElevationFunction& f,
						const glm::vec2& c,
						float r) :
			_func(f),
			_center(c),
			_radius(r)
		{ }
		
		inline const glm::vec2& getCenter() const { return _center; }
		inline float getRadius() const { return _radius; }
		
		inline double getHeight(const glm::vec2& V) const { return _func(V.x, V.y); }
		inline double operator()(const glm::vec2& V) const { return getHeight(V); }
		inline double operator()(double x, double y) const { return _func(x, y); }
		
	private:
		ElevationFunction	_func;
		glm::vec2			_center;
		float				_radius;
	};
	
	RadialTerrain(std::initializer_list<Zone> T) :
		_terrains(T.begin(), T.end())
	{
	}
	
	double getHeight(const glm::vec2& V) const;
	inline double operator()(const glm::vec2& V) const { return getHeight(V); }
	inline double operator()(double x, double y) const { return getHeight(glm::vec2(x, y)); }
	
private:
	std::forward_list<Zone>	_terrains;
};
