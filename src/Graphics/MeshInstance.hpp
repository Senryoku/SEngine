#pragma once

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
	
	inline void setModelMatrix(const glm::mat4& m)
	{ 
		_modelMatrix = m; 
		_material.setUniform("ModelMatrix", _modelMatrix); // Shouldn't have to do that if mat4 pointer worked correctly as uniforms...
	}
	
	bool isVisible(const glm::mat4& ProjectionMatrix, const glm::mat4& ViewMatrix) const;
	
	AABB<glm::vec3> getAABB() const
	{
		const BoundingBox& bbox = _mesh->getBoundingBox();
		return AABB<glm::vec3>(glm::vec3(_modelMatrix * glm::vec4(bbox.min, 1.0)),
								glm::vec3(_modelMatrix * glm::vec4(bbox.max, 1.0)));
	}
	
private:
	const Mesh*		_mesh = nullptr;	
	Material		_material;
	glm::mat4		_modelMatrix;
};
