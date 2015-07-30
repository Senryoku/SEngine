#include <RiggedMesh.hpp>

RiggedMesh::RiggedMesh() :
	Mesh(),
	_vertexBoneBuffer(Buffer::Target::VertexAttributes)
{
}

void RiggedMesh::createVAO()
{
	Mesh::createVAO();
	
	_vao.bind();
	_vertexBoneBuffer.init();
	_vertexBoneBuffer.bind();
	_vertexBoneBuffer.data(_vertexBoneData.data(), sizeof(VertexBoneData) * _vertexBoneData.size(), Buffer::Usage::StaticDraw);
    _vao.attributeI(3, 4, GL_INT, sizeof(VertexBoneData), (GLvoid *) offsetof(struct VertexBoneData, id));
    _vao.attribute(4, 4, GL_FLOAT, GL_FALSE, sizeof(VertexBoneData), (GLvoid *) offsetof(struct VertexBoneData, weight));
	
	_vao.unbind();
	_vertexBoneBuffer.unbind();
	
	_material.getShadingProgram().bindUniformBlock("Bones", _bonesBuffer); 
}
