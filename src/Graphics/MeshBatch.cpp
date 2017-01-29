#include <MeshBatch.hpp>

MeshBatch::MeshBatch(const Mesh& mesh) :
	_mesh(&mesh),
	_instances_attributes(Buffer::Target::VertexAttributes)
{
}

void MeshBatch::createVAO()
{
	_vao.init();
	_vao.bind();
	
	_mesh->getVertexBuffer().bind();
	
	// Could be declared elsewhere.
	constexpr unsigned int PerVertexAttributesCount = 3; 
	constexpr unsigned int PerInstanceAttributesCount = 4; 
	
	for(unsigned int i = 0; i < PerVertexAttributesCount + PerInstanceAttributesCount; ++i)
		glEnableVertexAttribArray(i);

	// Basic mesh attributes
    _vao.attribute(0, 3, Type::Float, false, sizeof(Mesh::Vertex), offsetof(struct Mesh::Vertex, position));
    _vao.attribute(1, 3, Type::Float, true, sizeof(Mesh::Vertex), offsetof(struct Mesh::Vertex, normal));
    _vao.attribute(2, 2, Type::Float, false, sizeof(Mesh::Vertex), offsetof(struct Mesh::Vertex, texcoord));

	// Per instance attributes
	_instances_attributes.init();
	_instances_attributes.bind();
	_instances_attributes.data(_instances_data.data(), sizeof(InstanceData) * _instances_data.size(), Buffer::Usage::StaticDraw);
	for(int i = 0; i < 4; ++i)
	{
		_vao.attribute(PerVertexAttributesCount + i, 4, Type::Float, false, sizeof(InstanceData), (sizeof(float) * i * 4));
		glVertexAttribDivisor(PerVertexAttributesCount + i, 1);
	}
	
	_mesh->getIndexBuffer().bind();
	
	_vao.unbind(); // Unbind first on purpose :)
	_mesh->getIndexBuffer().unbind();
	_instances_attributes.unbind();
	_mesh->getVertexBuffer().unbind();
}

void MeshBatch::draw(bool usingMeshMaterial) const
{
	_vao.bind();
	if(usingMeshMaterial) _mesh->getMaterial().use();
	glDrawElementsInstanced(GL_TRIANGLES, _mesh->getTriangles().size() * 3, GL_UNSIGNED_INT, 0, _instances_data.size());
	_vao.unbind();
}

void MeshBatch::draw(const glm::mat4& VPMatrix, bool usingMeshMaterial)
{
	if(!_transformFeedback)
		initVFC();
	
	_transformFeedback.disableRasterization();
	_transformFeedback.bind();
	_transformFeedback.begin();
	
	// @todo VFC
	
	_transformFeedback.end();
	_transformFeedback.unbind();
	_transformFeedback.enableRasterization();
	
	if(usingMeshMaterial) _mesh->getMaterial().use();
	_transformFeedback.draw(Primitive::Triangles);
}

void MeshBatch::initVFC()
{
	_transformFeedback.init();
	_transformFeedback.bindBuffer(0, _transformFeedbackBuffer);
	_transformFeedback.unbind();
}
