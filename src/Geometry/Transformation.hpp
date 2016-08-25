#pragma once 

#include <vector>
#include <functional>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>

class Transformation
{
public:
	Transformation(const glm::mat4& m = glm::mat4{1.0f});
	Transformation(const glm::vec3& p, 
		const glm::quat& r = glm::quat{0.0f, 0.0f, 0.0f, 1.0f}, 
		const glm::vec3& scale = glm::vec3{1.0f});
	Transformation(const Transformation&) =default;
	
	inline const glm::mat4& getModelMatrix() const { return _modelMatrix; }
	inline const glm::vec3& getPosition() const { return _position; }
	inline const glm::quat& getRotation() const { return _rotation; }
	inline const glm::vec3& getScale() const { return _scale; }
	
	inline void setModelMatrix(const glm::mat4& m)
	{ 
		_modelMatrix = m;
		
		glm::vec3 skew;
		glm::vec4 perspective;
		glm::decompose(_modelMatrix, _scale, _rotation, _position, skew, perspective);
	}
	
	inline void setPosition(const glm::vec3& p)
	{
		_position = p;
		computeMatrix();
	}
	
	inline void setRotation(const glm::quat& r)
	{
		_rotation = r;
		computeMatrix();
	}
	
	inline void setScale(const glm::vec3& s)
	{
		_scale = s;
		computeMatrix();
	}
	
	inline void setScale(float s)
	{
		setScale(glm::vec3{s, s, s});
	}

private:
	glm::mat4		_modelMatrix;
	
	glm::vec3		_position;
	glm::quat		_rotation;
	glm::vec3		_scale;
	
	inline void computeMatrix()
	{
		_modelMatrix = glm::translate(glm::mat4(1.0f), _position) * 
			glm::mat4_cast(_rotation) * 
			glm::scale(glm::mat4(1.0f), _scale);
	}
};
