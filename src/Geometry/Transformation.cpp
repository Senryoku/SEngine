#include <Transformation.hpp>


Transformation::Transformation(const glm::mat4& m)
{
	setModelMatrix(m);
}

Transformation::Transformation(const glm::vec3& p, const glm::quat& r, const glm::vec3& s) :
	_position(p),
	_rotation(r),
	_scale(s)
{
	computeMatrix();
}

