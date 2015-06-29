#pragma once

#include <initializer_list>
#include <vector>

#include <glm/glm.hpp>

#include <PerlinNoise.hpp>
#include <Terrain.hpp>

class NoisyTerrain : public Terrain
{
public:
	NoisyTerrain();
	NoisyTerrain(const std::vector<double> & a, 
					const std::vector<double> & l, 
					const std::vector<double> & p);

	double getHeight(double x, double y) const;
	inline double operator()(double x, double y) const override { return getHeight(x, y); }

private:
	std::vector<double>		_a;			///< Amplitudes
	std::vector<double>		_l;			///< Frequencies
	std::vector<double>		_p;			///< Phases
};
