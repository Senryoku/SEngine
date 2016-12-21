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
	_matrix = t._matrix;
	_globalMatrix = t._globalMatrix;
	
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
	if(json.is_array())
	{
		glm::mat4 r;
		for(int i = 0; i < 4; ++i)
			for(int j = 0; j < 4; ++j)
				r[i][j] = json[i * 4 + j];
		setMatrix(r);
	} else {
		_position = vec3(json["position"]);
		_rotation = quat(json["rotation"]);
		_scale = vec3(json["scale"]);
		computeMatrix();
	}
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

nlohmann::json Transformation::json() const
{
	return {
		{"position", tojson(getPosition())},
		{"rotation", tojson(getRotation())},
		{"scale", tojson(getScale())},
		{"parent", getParent()}
	};
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
	_matrix = m;
	
	glm::vec3 skew;
	glm::vec4 perspective;
	glm::decompose(_matrix, _scale, _rotation, _position, skew, perspective);
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
	_matrix = glm::translate(glm::mat4(1.0f), _position) * 
		glm::mat4_cast(_rotation) * 
		glm::scale(glm::mat4(1.0f), _scale);
	updateGlobalModelMatrix();
}

inline void Transformation::updateGlobalModelMatrix()
{
	_globalMatrix = _parent == invalid_component_idx ? 
		_matrix :
		get_component<Transformation>(_parent).getGlobalMatrix() * _matrix;

	for(ComponentID c : _children)
		get_component<Transformation>(c).updateGlobalModelMatrix();
}
