#include <Frustum.hpp>

Frustum::Frustum(const glm::mat4& projmat)
{   
	// Fast Extraction of Viewing Frustum Planes from the World-View-Projection Matrix
	// Gribb & Hartmann
	// http://www8.cs.umu.se/kurser/5DV051/HT12/lab/plane_extraction.pdf
	// https://stackoverflow.com/questions/12836967/extracting-view-frustum-planes-hartmann-gribbs-method
	std::array<std::array<float, 4>, 6> coeff{};
    for(int i = 4; --i;) coeff[Top][i]       = projmat[i][3] - projmat[i][1];
    for(int i = 4; --i;) coeff[Bottom][i]    = projmat[i][3] + projmat[i][1];
	for(int i = 4; --i;) coeff[Left][i]      = projmat[i][3] + projmat[i][0];
    for(int i = 4; --i;) coeff[Right][i]     = projmat[i][3] - projmat[i][0]; 
    for(int i = 4; --i;) coeff[Near][i]      = projmat[i][3] + projmat[i][2];
    for(int i = 4; --i;) coeff[Far][i]       = projmat[i][3] - projmat[i][2];
	
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
		// First version
		float mf = glm::dot(m, planes[i].getNormal()) + planes[i][3];
		float n = glm::dot(d, glm::abs(planes[i].getNormal()));
		if(mf + n < 0) return false;
		
		/*
		// Optimized with p-vertex and n-vertex
		const auto& n = planes[i].getNormal();
		const auto& a = aabb.min;
		const auto& b = aabb.max;
		glm::vec3 pv, nv;
		
		const bool nx = n.x > 0, ny = n.y > 0, nz = n.z > 0;
		if( nx &&  ny &&  nz) { pv = b;             nv = a;             }
		if( nx &&  ny && !nz) { pv = {b.x,b.y,a.z}; nv = {a.x,a.y,b.z}; }
		if( nx && !ny &&  nz) { pv = {b.x,a.y,b.z}; nv = {a.x,b.y,a.z}; }
		if( nx && !ny && !nz) { pv = {b.x,a.y,a.z}; nv = {a.x,b.y,b.z}; }
		if(!nx &&  ny &&  nz) { pv = {a.x,b.y,b.z}; nv = {b.x,a.y,a.z}; }
		if(!nx &&  ny && !nz) { pv = {a.x,b.y,a.z}; nv = {b.x,a.y,b.z}; }
		if(!nx && !ny &&  nz) { pv = {a.x,a.y,b.z}; nv = {b.x,b.y,a.z}; }
		if(!nx && !ny && !nz) { pv = a;             nv = b;             }
		
		if(glm::dot(n, nv) > planes[i][3]) return false;
		
		if(glm::dot(n, pv) > planes[i][3]) return true; // Intersection
		*/
    }
	return true;
}
