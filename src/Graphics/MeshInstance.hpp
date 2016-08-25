#pragma once

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <Mesh.hpp>

class MeshInstance
{
public:
	MeshInstance(const Mesh& mesh, const glm::mat4& modelMatrix = glm::mat4(1.0));
	
	void draw() const
	{
		_material.use();
		_mesh->draw();
	}
	
	Material& getMaterial() { return _material; }
	
	inline const Mesh& getMesh() const { return *_mesh; }
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
		
		update();
	}
	
	inline void setPosition(const glm::vec3& p)
	{
		_position = p;
		computeMatrix();
		update();
	}
	
	inline void setRotation(const glm::quat& r)
	{
		_rotation = r;
		computeMatrix();
		update();
	}
	
	inline void setScale(const glm::vec3& s)
	{
		_scale = s;
		computeMatrix();
		update();
	}
	
	inline void setScale(float s)
	{
		setScale(glm::vec3{s, s, s});
	}
	
	bool isVisible(const glm::mat4& ProjectionMatrix, const glm::mat4& ViewMatrix) const;
	
	inline AABB<glm::vec3> getAABB() const
	{
		const BoundingBox& bbox = _mesh->getBoundingBox();
		return AABB<glm::vec3>{glm::vec3{_modelMatrix * glm::vec4{bbox.min, 1.0}},
								glm::vec3{_modelMatrix * glm::vec4{bbox.max, 1.0}}};
	}
	
private:
	const Mesh*		_mesh = nullptr;	
	Material		_material;
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
	
	inline void update()
	{
		_material.setUniform("ModelMatrix", _modelMatrix); // Shouldn't have to do that if mat4 pointer worked correctly as uniforms...
	}
};
