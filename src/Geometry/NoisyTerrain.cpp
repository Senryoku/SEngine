#include "NoisyTerrain.hpp"

NoisyTerrain::NoisyTerrain() : 
	_a{1.f, 0.75f, 0.5f, 0.25f, 0.125f},
	_l{1000.f, 250.f, 100.f, 0.5f, 1.f},
	_p{0.125f/2.f, 0.125f, 0.25f, 0.5f, 1.f}
{
}

NoisyTerrain::NoisyTerrain(const std::vector<double> & a, 
							const std::vector<double> & l, 
							const std::vector<double> & p) :
	_a(a),
	_l(l),
	_p(p)
{
}

double NoisyTerrain::getHeight(double x, double y) const
{
	double height = 0; 

	for(unsigned int i = 0; i < _a.size() ; i++)
	{
		double noise = scaled_raw_noise_2d(0.0, 1.0, x/_l[i] + _p[i], y/_l[i] + _p[i]);
		height += _a[i] * noise;
	}

	return height; 
}
