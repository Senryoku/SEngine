#include <Mesh.hpp>

#include <iostream>
#include <algorithm>

#include <tiny_obj_loader.h>

#include <Resources.hpp>

//////////////////////// Mesh::Triangle ///////////////////////////

Mesh::Triangle::Triangle(size_t v1,
						 size_t v2,
						 size_t v3) :
	vertices{v1, v2, v3}
{
}

/////////////////////// Mesh::Vertex //////////////////////////////

Mesh::Vertex::Vertex(glm::vec3 pos,
					 glm::vec3 nor,
					 glm::vec2 tex) :
	position(pos),
	normal(nor),
	texcoord(tex)
{
}

/////////////////////// Mesh //////////////////////////////////////

Mesh::Mesh() :
	_vao(),
	_vertex_buffer(Buffer::Target::VertexAttributes),
	_index_buffer(Buffer::Target::VertexIndices)
{
}

Transformation Mesh::resetPivot()
{
	glm::vec3 pivot = 0.5f * (_bbox.max + _bbox.min);
	pivot.y = _bbox.min.y;
	
	for(auto& v : _vertices)
		v.position -= pivot;
	computeBoundingBox();
	
	return Transformation(pivot);
}

void Mesh::createVAO()
{
	_vao.init();
	_vao.bind();
	
	_vertex_buffer.init();
	_vertex_buffer.bind();
	
	_vertex_buffer.data(&_vertices[0], sizeof(Vertex)*_vertices.size(), Buffer::Usage::StaticDraw);

    _vao.attribute(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *) offsetof(struct Vertex, position));
    _vao.attribute(1, 3, GL_FLOAT, GL_TRUE, sizeof(Vertex), (GLvoid *) offsetof(struct Vertex, normal));
    _vao.attribute(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *) offsetof(struct Vertex, texcoord));

	_index_buffer.init();
	_index_buffer.bind();
	_index_buffer.data(&_triangles[0], sizeof(size_t) * _triangles.size() * 3, Buffer::Usage::StaticDraw);
	
	_vao.unbind(); // Unbind first on purpose :)
	_index_buffer.unbind();
	_vertex_buffer.unbind();
}

void Mesh::draw() const
{
	if(!_vao)
	{
		Log::error("Draw call on a uninitialized mesh !");
		return;
	}
	_vao.bind();
	glDrawElements(GL_TRIANGLES, _triangles.size() * 3,  GL_UNSIGNED_INT, 0);
	_vao.unbind();
}

void Mesh::computeNormals()
{
	// Here, normals are the average of adjacent triangles' normals
	// (so we have exactly one normal per vertex)
	for(auto& v : _vertices)
		v.normal = glm::vec3{0.0f};

	for(Triangle& t : _triangles)
	{
		std::array<size_t, 3>& v = t.vertices;
		// Normal of this triangle
		glm::vec3 norm = glm::normalize(
							glm::cross(_vertices[v[1]].position - _vertices[v[0]].position,
									   _vertices[v[2]].position - _vertices[v[0]].position)
						 );

		for(size_t i = 0; i < 3; ++i)
		{
			_vertices[v[i]].normal += norm;
		}
	}

	for(auto& v : _vertices)
		v.normal = glm::normalize(v.normal);
}

void Mesh::computeBoundingBox()
{
	_bbox.min = glm::vec3{std::numeric_limits<float>::max()};
	_bbox.max = glm::vec3{std::numeric_limits<float>::lowest()};
	for(const auto& v : _vertices)
	{
		_bbox.min = glm::min(_bbox.min, v.position);
		_bbox.max = glm::max(_bbox.max, v.position);
	}
}

////////////////////// Static /////////////////////////////////////

#include <RiggedMesh.hpp>

std::vector<Mesh*> Mesh::load(const std::string& path)
{
	return load(path, Resources::getProgram("Deferred"));
}

std::vector<Mesh*> Mesh::load(const std::string& path, const Program& p)
{
	std::vector<Mesh*> M;
	Log::info("Loading ", path, "...");
	std::string rep = path.substr(0, path.find_last_of('/') + 1);

	// OBJ Loading
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string err;
	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, path.c_str(), rep.c_str(), true);

	if (!err.empty()) // `err` may contain warning message.
		Log::error(err);

	if (!ret)
		return M;
	
	M.resize(shapes.size());
	for(size_t s = 0; s < shapes.size(); s++)
	{
		std::string name{path};
		name.append("::" + shapes[s].name + "[" + std::to_string(s) + "]");
		while(Resources::isMesh(name))
		{
			Log::warn("Mesh '", name, "' was already loaded. Re-loading it after appending '_' to its name.");
			name.append("_");
		}
		
		M[s] = &Resources::getMesh(name);
		M[s]->_name = shapes[s].name;
		M[s]->_path = path;
		M[s]->getMaterial().setShadingProgram(p);
		
		if(!materials.empty())
		{
			for(size_t i = 0; i < shapes[s].mesh.material_ids.size() - 1; ++i)
				if(shapes[s].mesh.material_ids[i] >= 0 && shapes[s].mesh.material_ids[i + 1] >= 0 && 
					shapes[s].mesh.material_ids[i] != shapes[s].mesh.material_ids[i + 1])
				{
					Log::warn("We're only supporting one material per mesh but '", name, "' uses at least '",
						materials[shapes[s].mesh.material_ids[i]].name, "' (", shapes[s].mesh.material_ids[i], ") and '",
						materials[shapes[s].mesh.material_ids[i + 1]].name, "' (", shapes[s].mesh.material_ids[i + 1], ").");
					break;
				}
			const auto& material = materials[shapes[s].mesh.material_ids[0]];
					
			if(!material.diffuse_texname.empty())
			{
				std::string p = rep;
				p.append(material.diffuse_texname);
				std::replace(p.begin(), p.end(), '\\', '/');
				auto& t = Resources::getTexture<Texture2D>(p);
				if(!t.isValid())
				{
					Log::info("Loading diffuse texture '", p, "'.");
					t.load(p);
				}
				if(t.isValid())
				{
					M[s]->getMaterial().setUniform("Texture", t);
					M[s]->getMaterial().setUniform("Color", glm::vec3{1.0});
					M[s]->getMaterial().setSubroutine(ShaderType::Fragment, "colorFunction", "texture_color");
				} else {
					Log::error("Texture ", p, " is invalid.");
				}
			} else {
				M[s]->getMaterial().setUniform("Color", glm::vec3{material.diffuse[0], material.diffuse[1], material.diffuse[2]});
				M[s]->getMaterial().setSubroutine(ShaderType::Fragment, "colorFunction", "uniform_color");
			}
			
			std::string normal_map;
			if(!material.bump_texname.empty()) normal_map = material.bump_texname; // map_bump, bump
			if(!material.normal_texname.empty()) normal_map = material.normal_texname;
			
			if(!normal_map.empty())
			{
				std::string p = rep;
				p.append(normal_map);
				std::replace(p.begin(), p.end(), '\\', '/');
				auto& t = Resources::getTexture<Texture2D>(p);
				if(!t.isValid())
				{
					Log::info("Loading normal texture '", p, "'.");
					t.load(p);
				}
				if(t.isValid())
				{
					M[s]->getMaterial().setUniform("NormalMap", t);
					M[s]->getMaterial().setSubroutine(ShaderType::Fragment, "normalFunction", "normal_mapping");
				} else {
					Log::error("Texture ", p, " is invalid.");
				}
			} else {
				M[s]->getMaterial().setSubroutine(ShaderType::Fragment, "normalFunction", "basic_normal");
			}
		} else {
			M[s]->getMaterial().setUniform("Color", glm::vec3{1.0f});
			M[s]->getMaterial().setSubroutine(ShaderType::Fragment, "colorFunction", "uniform_color");
			M[s]->getMaterial().setSubroutine(ShaderType::Fragment, "normalFunction", "basic_normal");
		}
		
		auto minmax = glm::vec3{
			attrib.vertices[3 * shapes[s].mesh.indices[0].vertex_index + 0],
			attrib.vertices[3 * shapes[s].mesh.indices[0].vertex_index + 1],
			attrib.vertices[3 * shapes[s].mesh.indices[0].vertex_index + 2]
		};
		auto min = minmax;
		auto max = minmax;
		for(const auto& i : shapes[s].mesh.indices)
		{
			glm::vec3 v{
				attrib.vertices[3 * i.vertex_index + 0],
				attrib.vertices[3 * i.vertex_index + 1],
				attrib.vertices[3 * i.vertex_index + 2]
			};
			auto n = i.normal_index >= 0 ?
				glm::vec3{
					attrib.normals[3 * i.normal_index + 0],
					attrib.normals[3 * i.normal_index + 1],
					attrib.normals[3 * i.normal_index + 2]
				} : 
				glm::vec3{0.0f};
			auto t = i.texcoord_index >= 0 ?
				glm::vec2{
					attrib.texcoords[2 * i.texcoord_index + 0],
					1.0f - attrib.texcoords[2 * i.texcoord_index + 1] // !
				} : 
				glm::vec2{0.0f};
			M[s]->getVertices().push_back(Mesh::Vertex{v, n, t});
			
			min = glm::vec3(std::min(min.x, v.x), std::min(min.y, v.y), std::min(min.z, v.z));
			max = glm::vec3(std::max(max.x, v.x), std::max(max.y, v.y), std::max(max.z, v.z));
		}
		
		M[s]->setBoundingBox({min, max});
		
		for(size_t i = 0; i < shapes[s].mesh.num_face_vertices.size(); ++i)
		{
			assert(shapes[s].mesh.num_face_vertices[i] == 3);
			M[s]->getTriangles().push_back(Mesh::Triangle{
				3 * i + 0,
				3 * i + 1,
				3 * i + 2
			});
		}
		
		if(attrib.normals.empty())
			M[s]->computeNormals();
	}
	
	return M;
}
