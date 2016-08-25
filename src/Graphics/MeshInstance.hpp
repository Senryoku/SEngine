#pragma once

#include <Mesh.hpp>
#include <Transformation.hpp>

class MeshInstance
{
public:
	MeshInstance(const Mesh& mesh, const Transformation& t = Transformation{});
	
	void draw() const
	{
		_material.use();
		setUniform("ModelMatrix", _transformation.getModelMatrix()); // @todo Should be in the material...
		_mesh->draw();
	}
	
	inline Material& getMaterial() { return _material; }
	inline const Mesh& getMesh() const { return *_mesh; }
	
	inline Transformation& getTransformation() { return _transformation; }
	inline const Transformation& getTransformation() const { return _transformation; }
	
	bool isVisible(const glm::mat4& ProjectionMatrix, const glm::mat4& ViewMatrix) const;
	
	inline AABB<glm::vec3> getAABB() const
	{
		const BoundingBox& bbox = _mesh->getBoundingBox();
		return AABB<glm::vec3>{
			glm::vec3{_transformation.getModelMatrix() * glm::vec4{bbox.min, 1.0}},
			glm::vec3{_transformation.getModelMatrix() * glm::vec4{bbox.max, 1.0}}
		};
	}
	
private:
	const Mesh*		_mesh = nullptr;	
	Material		_material;
	Transformation	_transformation;
};
