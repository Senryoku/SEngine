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

void Transformation::setModelMatrix(const glm::mat4& m)
{ 
	_modelMatrix = m;
	
	glm::vec3 skew;
	glm::vec4 perspective;
	glm::decompose(_modelMatrix, _scale, _rotation, _position, skew, perspective);
	updateGlobalModelMatrix();
}
	
void Transformation::addChild(Transformation& t)
{
	if(t._parent != invalid_component_idx)
	{
		auto& c = get_component<Transformation>(t._parent)._children;
		c.erase(std::find(c.begin(), c.end(), get_id(t)));
	}
	_children.push_back(get_id<Transformation>(t));
	t.setParent(*this);
}

void Transformation::setParent(Transformation& t)
{
	_parent = get_id(t);
	updateGlobalModelMatrix();
}

inline void Transformation::computeMatrix()
{
	_modelMatrix = glm::translate(glm::mat4(1.0f), _position) * 
		glm::mat4_cast(_rotation) * 
		glm::scale(glm::mat4(1.0f), _scale);
	updateGlobalModelMatrix();
}

inline void Transformation::updateGlobalModelMatrix()
{
	_globalModelMatrix = _parent == invalid_component_idx ? _modelMatrix :
		get_component<Transformation>(_parent).getGlobalModelMatrix() * _modelMatrix;
	for(ComponentID c : _children)
		get_component<Transformation>(c).updateGlobalModelMatrix();
}
