#include <Transformation.hpp>

#include <glm/gtx/transform.hpp>

Transformation::Transformation(const glm::mat4& m)
{
	setMatrix(m);
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

Transformation::Transformation(const nlohmann::json& json)
{	
	glm::mat4 r;
	if(json.is_array())
	{
		for(int i = 0; i < 4; ++i)
			for(int j = 0; j < 4; ++j)
				r[i][j] = json[i * 4 + j];
	} else {
		r = glm::scale(
				glm::translate(
					glm::rotate(static_cast<float>(json["rotation"][0]), glm::vec3{1, 0, 0}) *  
					glm::rotate(static_cast<float>(json["rotation"][1]), glm::vec3{0, 1, 0}) *
					glm::rotate(static_cast<float>(json["rotation"][2]), glm::vec3{0, 0, 1})
				, vec3(json["position"])), 
				vec3(json["scale"]));
	}
	setMatrix(r);
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
	if(_parent != invalid_component_idx)
		return get_component<Transformation>(_parent)(_position);
	else
		return _position;
}

glm::quat Transformation::getGlobalRotation() const
{
	if(_parent != invalid_component_idx)
		return get_component<Transformation>(_parent).getGlobalRotation() * _rotation;
	else 
		return _rotation;
}

glm::vec3 Transformation::getGlobalScale() const
{ 
	if(_parent != invalid_component_idx)
		return get_component<Transformation>(_parent).getGlobalScale() * _scale;
	else
		return _scale;
}

void Transformation::setMatrix(const glm::mat4& m)
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
		get_component<Transformation>(_parent).getGlobalMatrix() * _modelMatrix;

	for(ComponentID c : _children)
		get_component<Transformation>(c).updateGlobalModelMatrix();
}
