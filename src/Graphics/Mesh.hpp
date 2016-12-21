#pragma once

#include <string>
#include <vector>
#include <array>

#include <glm/glm.hpp>
#define GLM_FORCE_RADIANS
#include <glm/gtc/type_ptr.hpp> // glm::value_ptr

#include <Buffer.hpp>
#include <VertexArray.hpp>
#include <BoundingShape.hpp>
#include <Material.hpp>
#include <Transformation.hpp>
#include <Log.hpp>

class Mesh
{
public:
	/**
	 * Three indices of vertices forming a triangle.
	**/
	struct Triangle
	{
		Triangle(size_t v1,
				size_t v2,
				size_t v3);

		Triangle(const Triangle& T) =default;

		std::array<size_t, 3>	vertices;
	};

	/**
	 * Base structure for Vertices (as they will be passed to the GPU).
	**/
	struct Vertex
	{
		Vertex() =default;
		Vertex(glm::vec3 pos,
			glm::vec3 nor = glm::vec3{0.0},
			glm::vec2 tex = glm::vec2{0.0});

		glm::vec3	position;
		glm::vec3	normal;
		glm::vec2	texcoord;
	};
	
	Mesh();

	inline std::vector<Vertex>&			getVertices()		{ return _vertices; }			///< @return Array of Vertices
	inline std::vector<Triangle>& 		getTriangles()		{ return _triangles; }			///< @return Array of Triangles
	inline Material& 					getMaterial()		{ return _material; }			///< @return Material
	
	inline const std::string&			getName()			const { return _name; }			///< @return Name
	inline const std::string&			getPath()			const { return _path; }			///< @return Path @see _path
	
	inline const std::vector<Vertex>&	getVertices() 		const { return _vertices; }		///< @return Array of Vertices
	inline const std::vector<Triangle>&	getTriangles()		const { return _triangles; }	///< @return Array of Triangles
	inline const Material&				getMaterial()		const { return _material; }		///< @return Material
	inline const VertexArray& 			getVAO()			const { return _vao; }			///< @return VertexArray Object
	inline const Buffer& 				getVertexBuffer()	const { return _vertex_buffer; }///< @return Vertex Buffer
	inline const Buffer&				getIndexBuffer()	const { return _index_buffer; }	///< @return Index Buffer
	
	inline void	setName(const std::string& name) { _name = name; }
	
	void computeNormals();
	glm::vec3 resetPivot();
	
	void createVAO();
	void update();
	void draw() const;
	
	void computeBoundingBox();
	inline void setBoundingBox(const BoundingBox& bbox)	{ _bbox = bbox; }
	inline const BoundingBox& getBoundingBox() const	{ return _bbox; }

	static std::vector<Mesh*> load(const std::string& path);
	static std::vector<Mesh*> load(const std::string& path, const Program& p);

protected:
	std::string				_name;	///< Name
	std::string				_path;	///< Path to the file from where the mesh was loaded (optional)

	std::vector<Vertex>		_vertices;
	std::vector<Triangle>	_triangles;
	
	VertexArray				_vao;
	Buffer					_vertex_buffer;
	Buffer					_index_buffer;
	
	Material 				_material; ///< Base (default) Material for this mesh
	
	BoundingBox				_bbox;
};
