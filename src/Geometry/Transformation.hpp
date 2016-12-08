#pragma once 

#include <vector>
#include <functional>
#include <algorithm>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <Component.hpp>

class Transformation
{
public:
	Transformation(const glm::mat4& m = glm::mat4{1.0f});
	Transformation(const glm::vec3& p, 
		const glm::quat& r = glm::quat{}, 
		const glm::vec3& scale = glm::vec3{1.0f});
	Transformation(const Transformation&) =delete;
	Transformation(Transformation&&);
	
	~Transformation();
	
	inline const glm::mat4& getModelMatrix() const { return _modelMatrix; }
	inline const glm::mat4& getGlobalModelMatrix() const { return _globalModelMatrix; };
	inline const glm::vec3& getPosition() const { return _position; }
	inline const glm::quat& getRotation() const { return _rotation; }
	inline const glm::vec3& getScale() const { return _scale; }
	
	inline ComponentID getParent() const { return _parent; }
	inline const std::vector<ComponentID>& getChildren() const { return _children; }
	
	void setModelMatrix(const glm::mat4& m);
	
	inline void setPosition(const glm::vec3& p) { _position = p; computeMatrix(); }
	inline void setRotation(const glm::quat& r) { _rotation = r; computeMatrix(); }
	inline void setScale(const glm::vec3& s)    { _scale = s;    computeMatrix(); }
	
	inline void setScale(float s) { setScale(glm::vec3{s, s, s}); }
	
	inline glm::vec4 apply(const glm::vec4& v) const { return _modelMatrix * v; }
	inline glm::vec3 apply(const glm::vec3& v) const { return glm::vec3{apply(glm::vec4{v, 1.0f})}; }
	
	template<typename T>
	inline T operator()(const T& v) const { return apply(v); }

	/// @todo Maybe passing a ComponentID would be safer?...
	void addChild(Transformation& t);
	void addChild(ComponentID t);
	void remChild(const Transformation& t);
	void remChild(ComponentID t);
	void setParent(Transformation& t);
	void setParent(ComponentID t);

private:
	glm::mat4		_modelMatrix;
	glm::mat4		_globalModelMatrix;
	
	glm::vec3		_position;
	glm::quat		_rotation;
	glm::vec3		_scale;

	ComponentID					_parent = invalid_component_idx;
	std::vector<ComponentID>	_children;
	
	void computeMatrix();
	void updateGlobalModelMatrix();
};
