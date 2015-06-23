#include "RadialTerrain.hpp"

#include <cmath> // std::sqrt

double RadialTerrain::getHeight(const glm::vec2& V) const
{
	double H = 0.f;
	double sumFactor = 0.f;
	
	for(auto& T : _terrains)
	{
		double sqr_r = (V.x - T.getCenter().x) * (V.x - T.getCenter().x) 
						+ (V.y - T.getCenter().y) * (V.y - T.getCenter().y);
		double sqr_R = T.getRadius() * T.getRadius();
		if(sqr_r < sqr_R)
		{	
			// TODO : Improve factor !!
			// TODO : Change it to a parameter ?
			//double factor = (1.f - sqr_r/sqr_R); // Bof
			//double factor = sqr_R*T.getRadius()/(sqr_r + sqr_R) - sqr_R*T.getRadius()/(2*sqr_R); // Agnesi, Bof
			
			// Slow but cool ! (continuous)
			double r = std::sqrt(sqr_r);
			double factor = (2.f/(sqr_R*T.getRadius())*r - 3.f/sqr_R)*sqr_r + 1.f;
			
			sumFactor += factor;
			H += factor*T.getHeight(V);
		}
	}
	
	if(sumFactor <= 0.f) return 0.f;
	else H /= sumFactor;
	
	return H;
}