#pragma once

#include <Mesh.hpp>
#include <TransformFeedback.hpp>

/**
 * Easy way to get multiple instances of a mesh draw efficiently
 * @todo All of it.
 * @todo Easy VFC with Transform Feedback ?
**/
class MeshBatch
{
public:
	/**
	 * Data for each instance of the batch.
	**/
	struct InstanceData
	{
		glm::mat4	modelMatrix; ///< Model Matrix
	};
	
	/**
	 * Constructor
	 * @param mesh Mesh to draw for each instance.
	**/
	MeshBatch(const Mesh& mesh);
	
	/**
	 * Creates and initialize the VAO for drawing.
	 * Mesh has to be correctly initialized before ! (Vertex and Index Buffer initialized)
	**/
	void createVAO();
	
	/**
	 * Draw all the instances.
	 * @param usingMeshMaterial if true, binds the mesh's material before drawing.
	**/
	void draw(bool usingMeshMaterial = true) const;
	/**
	 * Draw all the instances with View Frustum Culling.
	 * @param VPMatrix ViewProjection Matrix used for culling.
	 * @param usingMeshMaterial if true, binds the mesh's material before drawing.
	**/
	void draw(const glm::mat4& VPMatrix, bool usingMeshMaterial = true);
	
	/**
	 * Initialize TransformFeedback for View Frustum Culling.
	**/
	void initVFC();
	
	std::vector<InstanceData>&			getInstancesData()		{ return _instances_data; }
	const std::vector<InstanceData>&	getInstancesData() const	{ return _instances_data; }
	
	Buffer&				getInstancesAttributes()		{ return _instances_attributes; }
	const Buffer&		getInstancesAttributes() const	{ return _instances_attributes; }
	
private:
	const Mesh*					_mesh;					///< Mesh to draw.
	std::vector<InstanceData>	_instances_data;		///< Attributes for each instance to draw.
	
	VertexArray					_vao;					///< VertexArray Object.
	Buffer						_instances_attributes;	///< Buffer containing the per-instance data
	
	TransformFeedback			_transformFeedback;		///< TransformFeedback for View Frustum Culling.
	Buffer						_transformFeedbackBuffer;
};
