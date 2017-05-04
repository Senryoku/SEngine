#pragma once

#include <Mesh.hpp>
#include <Transformation.hpp>
#include <Entity.hpp>
#include <serialization.hpp>

class MeshRenderer
{
public:
	MeshRenderer() =default;
	explicit MeshRenderer(const Mesh& mesh);
	MeshRenderer(const nlohmann::json& json);
	MeshRenderer(MeshRenderer&&);
	
	nlohmann::json json() const;
	
	inline void draw() const;
	
	inline Material& getMaterial()             { return _material; }
	inline const Material& getMaterial() const { return _material; }
	inline const Mesh& getMesh()         const { return *_mesh; }
	
	inline const Transformation& getTransformation() const { return entities[_entity].get<Transformation>(); }
	
	bool isVisible(const glm::mat4& ProjectionMatrix, const glm::mat4& ViewMatrix) const;
	
	inline AABB<glm::vec3> getAABB() const;
	
private:
	const Mesh*				_mesh = nullptr;	
	Material				_material;
	EntityID				_entity = invalid_entity;
};

inline void MeshRenderer::draw() const
{
	assert(_mesh != nullptr);
	assert(_entity != invalid_entity);
	_material.use();
	setUniform("ModelMatrix", getTransformation().getGlobalMatrix());
	_mesh->draw();
}

inline AABB<glm::vec3> MeshRenderer::getAABB() const
{
	assert(_mesh != nullptr);
	// Will not yield a perfectly fit AABB (one would have to process every vertex to get it), 
	// but a correct one, meaning it will contain the entire transformed model.
	auto bbox = _mesh->getBoundingBox().getBounds();
	auto min = glm::vec3{std::numeric_limits<float>::max()};
	auto max = glm::vec3{std::numeric_limits<float>::lowest()};
	for(auto v : bbox)
	{	
		v = getTransformation()(v);
		min = glm::min(min, v);
		max = glm::max(max, v);
	}
	if(min.x == max.x) max.x = stdext::next_representable(max.x);
	if(min.y == max.y) max.y = stdext::next_representable(max.y);
	if(min.z == max.z) max.z = stdext::next_representable(max.z);
	return AABB<glm::vec3>{min, max};
}
