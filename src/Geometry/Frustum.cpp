#include <Frustum.hpp>

Frustum::Frustum(const glm::mat4& projmat)
{   
	// Fast Extraction of Viewing Frustum Planes from the World-View-Projection Matrix
	// Gribb & Hartmann
	// http://www8.cs.umu.se/kurser/5DV051/HT12/lab/plane_extraction.pdf
	// https://stackoverflow.com/questions/12836967/extracting-view-frustum-planes-hartmann-gribbs-method
	std::array<std::array<float, 4>, 6> coeff;
    for(int i = 0; i < 4; ++i) 
	{
		coeff[Top][i]    = projmat[i][3] - projmat[i][1];
		coeff[Bottom][i] = projmat[i][3] + projmat[i][1];
		coeff[Left][i]   = projmat[i][3] + projmat[i][0];
		coeff[Right][i]  = projmat[i][3] - projmat[i][0]; 
		coeff[Near][i]   = projmat[i][3] + projmat[i][2];
		coeff[Far][i]    = projmat[i][3] - projmat[i][2];
	}
	
	for(int i = 0; i < 6; ++i)
		planes[i].setCoefficients(coeff[i]);
}

bool Frustum::isIntersecting(const AABB<glm::vec3>& aabb) const
{
	// http://old.cescg.org/CESCG-2002/DSykoraJJelinek/
	glm::vec3 m = (aabb.max + aabb.min) / 2.0f;
	glm::vec3 d = aabb.max - m;
    for(int i = 0; i < 6; ++i)
    {
		float mf = glm::dot(m, planes[i].getNormal()) + planes[i][3];
		float n = glm::dot(d, glm::abs(planes[i].getNormal()));
		if(mf + n < 0) return false; // Strictly outside
		if(mf - n < 0) return true; // Intersecting
    }
	return true;
}
