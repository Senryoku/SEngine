#include <MeshRenderer.hpp>

#include <Context.hpp>
#include <Resources.hpp>

MeshRenderer::MeshRenderer(const Mesh& mesh) :
	_mesh{&mesh},
	_material{mesh.getMaterial()},
	_entity{get_owner<MeshRenderer>(*this)}
{
	_occlusion_query.init();
	_aabb_vertices_buffer.init();
	update_aabb_vertices();
}

MeshRenderer::MeshRenderer(MeshRenderer&& m) :
	_mesh{m._mesh},
	_material{std::move(m._material)},
	_entity{m._entity}
{
	m._mesh = nullptr;
	m._entity = invalid_entity;
	_occlusion_query.init();
	_aabb_vertices_buffer.init();
	update_aabb_vertices();
}

MeshRenderer::MeshRenderer(const nlohmann::json& json) :
	_mesh{&Resources::getMesh(json["mesh"])},
	_material{_mesh->getMaterial()},
	_entity{get_owner<MeshRenderer>(*this)}
{
	if(json.count("material") > 0)
		update_material(json["material"], _material);
	_occlusion_query.init();
	_aabb_vertices_buffer.init();
	update_aabb_vertices();
}

nlohmann::json MeshRenderer::json() const
{
	return {
		{"mesh", getMesh().getName()},
		{"material", tojson(getMaterial())}
	};
}

bool MeshRenderer::isVisible(const Frustum& f) const
{
    return f.isIntersecting(getAABB());
}

/**
 * FIXME: This is way too slow, and the result should be cached somehow
**/
bool MeshRenderer::isVisible(const glm::mat4& ProjectionMatrix, const glm::mat4& ViewMatrix) const
{
	assert(_mesh != nullptr);
	assert(_entity != invalid_entity);
	
	/// @todo Use MeshRenderer's bounding box
	const BoundingBox& bbox = _mesh->getBoundingBox();
	auto gmm = getTransformation().getGlobalMatrix();
	const glm::vec4 a = gmm * glm::vec4(bbox.min, 1.0);
	const glm::vec4 b = gmm * glm::vec4(bbox.max, 1.0);

	std::array<glm::vec4, 8> p = {glm::vec4{a.x, a.y, a.z, 1.0},
								  glm::vec4{a.x, a.y, b.z, 1.0},
								  glm::vec4{a.x, b.y, a.z, 1.0},
								  glm::vec4{a.x, b.y, b.z, 1.0},
								  glm::vec4{b.x, a.y, a.z, 1.0},
								  glm::vec4{b.x, a.y, b.z, 1.0},
								  glm::vec4{b.x, b.y, a.z, 1.0},
								  glm::vec4{b.x, b.y, b.z, 1.0}};
						
	bool front = false;
	for(auto& t : p)
	{
		t = ViewMatrix * t;
		front = front || t.z < 0.0;
	}

	if(!front) return false;

	glm::vec2 min = glm::vec2(2.0, 2.0);
	glm::vec2 max = glm::vec2(-2.0, -2.0);
						
	for(auto& t : p)
	{
		t = ProjectionMatrix * t;
		if(t.w > 0.0) t /= t.w;
		min.x = std::min(min.x, t.x);
		min.y = std::min(min.y, t.y);
		max.x = std::max(max.x, t.x);
		max.y = std::max(max.y, t.y);
	}

	return !(max.x < -1.0 || max.y < -1.0 ||
			 min.x >  1.0 || min.y >  1.0);
}

void MeshRenderer::update_aabb_vertices()
{
	auto aabb = _mesh->getBoundingBox().getBounds();
	static constexpr std::array<int, 12 * 3> indexes = {
		0, 1, 3,
		0, 3, 2,
		0, 5, 1,
		0, 4, 5,
		0, 6, 4,
		0, 2, 6,
		1, 5, 7,
		1, 7, 3,
		4, 6, 7,
		4, 7, 5,
		2, 3, 7,
		2, 7, 6
	};
	for(int i = 0; i < 12 * 3; ++i)
		_aabb_vertices[i] = aabb[indexes[i]];
	_aabb_vertices_buffer.data(&_aabb_vertices[0], 3 * sizeof(GLfloat) * _aabb_vertices.size(), Buffer::Usage::DynamicDraw);
}

void MeshRenderer::draw_bounding_box() const
{
	_aabb_vertices_buffer.bind();
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	setUniform("ModelMatrix", getTransformation().getGlobalMatrix());
	glDrawArrays(GL_TRIANGLES, 0, 12 * 3 * 3);
}

void MeshRenderer::occlusion_query()
{
	glDepthFunc(GL_LEQUAL);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glDepthMask(GL_FALSE);
	Context::disable(Capability::CullFace);
	_occlusion_query.begin(Query::Target::AnySamplesPassed);
	draw_bounding_box();
	_occlusion_query.end();
	Context::enable(Capability::CullFace);
	glDepthMask(GL_TRUE);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthFunc(GL_LESS);
}
