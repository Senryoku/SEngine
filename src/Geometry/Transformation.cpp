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

Transformation::Transformation(Transformation&& t)
{
	_modelMatrix = t._modelMatrix;
	_globalModelMatrix = t._globalModelMatrix;
	
	_position = t._position;
	_rotation = t._rotation;
	_scale = t._scale;

	_parent = t._parent;
	t._parent = invalid_component_idx;
	_children = t._children;
	t._children.clear();
}

Transformation::~Transformation()
{
	if(_parent != invalid_component_idx)
	{
		auto& parent = get_component<Transformation>(_parent);
		parent.remChild(*this);
	
		for(auto& c : _children)
			parent.addChild(c);
	} else {
		for(auto& c : _children)
			get_component<Transformation>(c).setParent(invalid_component_idx);
	}
}

glm::vec3 Transformation::getGlobalPosition() const
{
	return get_component<Transformation>(_parent)(_position);
}

glm::quat Transformation::getGlobalRotation() const
{
	glm::vec3 scale;
	glm::quat rotation;
	glm::vec3 translation;
	glm::vec3 skew;
	glm::vec4 perspective;
	glm::decompose(_globalModelMatrix, scale, rotation, translation, skew, perspective);
	return rotation;
}

glm::vec3 Transformation::getGlobalScale() const
{ 
	glm::vec3 scale;
	glm::quat rotation;
	glm::vec3 translation;
	glm::vec3 skew;
	glm::vec4 perspective;
	glm::decompose(_globalModelMatrix, scale, rotation, translation, skew, perspective);
	return scale;
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
		get_component<Transformation>(t._parent).remChild(t);
	_children.push_back(get_id<Transformation>(t));
	t.setParent(*this);
}

void Transformation::addChild(ComponentID t)
{
	addChild(get_component<Transformation>(t));
}

void Transformation::remChild(const Transformation& t)
{
	remChild(get_id<Transformation>(t));
}

void Transformation::remChild(ComponentID t)
{
	_children.erase(std::find(_children.begin(), _children.end(), t));
}

void Transformation::setParent(Transformation& t)
{
	setParent(get_id(t));
}

void Transformation::setParent(ComponentID t)
{
	_parent = t;
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
	_globalModelMatrix = _parent == invalid_component_idx ? 
		_modelMatrix :
		get_component<Transformation>(_parent).getGlobalModelMatrix() * _modelMatrix;

	for(ComponentID c : _children)
		get_component<Transformation>(c).updateGlobalModelMatrix();
}
