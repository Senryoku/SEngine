#pragma once

#include <Mesh.hpp>
#include <Transformation.hpp>

class MeshRenderer
{
public:
	MeshRenderer(const Mesh& mesh, const Transformation& t = Transformation{});
	
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
		// Will not yield a perfectly fit AABB (one would have to process every vertex to get it), 
		// but a correct one, meaning it will contain the entire transformed model.
		auto bbox = _mesh->getBoundingBox().getBounds();
		auto min = glm::vec3{std::numeric_limits<float>::max()};
		auto max = glm::vec3{std::numeric_limits<float>::lowest()};
		for(auto v : bbox)
		{	
			v = _transformation(v);
			min = glm::min(min, v);
			max = glm::max(max, v);
		}
		return AABB<glm::vec3>{min, max};
	}
	
private:
	const Mesh*		_mesh = nullptr;	
	Material		_material;
	Transformation	_transformation;
};
