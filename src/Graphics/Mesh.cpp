#include <Mesh.hpp>

#include <iostream>
#include <algorithm>

#include <assimp/Importer.hpp> 	// C++ importer interface
#include <assimp/scene.h> 			// Output data structure
#include <assimp/postprocess.h> // Post processing flags

#include <ResourcesManager.hpp>

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
	_index_buffer.data(&_triangles[0], sizeof(size_t)*_triangles.size()*3, Buffer::Usage::StaticDraw);
	
	_vao.unbind(); // Unbind first on purpose :)
	_index_buffer.unbind();
	_vertex_buffer.unbind();
}

void Mesh::draw() const
{
	if(!_vao)
	{
		std::cerr << "Draw call on a uninitialized mesh !" << std::endl;
		return;
	}
	_vao.bind();
	glDrawElements(GL_TRIANGLES, _triangles.size()*3,  GL_UNSIGNED_INT, 0);
	_vao.unbind();
}

void Mesh::computeNormals()
{
	// Here, normals are the average of adjacent triangles' normals
	// (so we have exactly one normal per vertex)
	for(auto& v : _vertices)
		v.normal = glm::vec3();

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

////////////////////// Static /////////////////////////////////////
	
std::vector<Mesh*> Mesh::load(const std::string& path)
{
	//std::cout << "Loading " << path << " using assimp..." << std::endl;
	
	std::vector<Mesh*> M;
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path.c_str(), 
												aiProcess_CalcTangentSpace |
												aiProcess_Triangulate |
												aiProcess_JoinIdenticalVertices |
												aiProcess_SortByPType |
												aiProcess_GenSmoothNormals | 
												aiProcess_FlipUVs);
																
	 // If the import failed, report it
	if( !scene)
	{
		std::cerr << importer.GetErrorString() << std::endl;
		return M;
	}
	
	std::string rep = path.substr(0, path.find_last_of('/') + 1);
	
	if(scene->HasMeshes())
	{
		M.resize(scene->mNumMeshes);
		for(unsigned int meshIdx = 0; meshIdx < scene->mNumMeshes; ++meshIdx)
		{
			aiMesh* LoadedMesh = scene->mMeshes[meshIdx];
			
			std::string name(path);
			name.append("::" + std::to_string(meshIdx));
			name.append(scene->mMeshes[meshIdx]->mName.C_Str());
			while(ResourcesManager::getInstance().isMesh(name))
			{
				std::cout << "Warning: Mesh '" << name << "' was already loaded. Re-loading it under the name '";
				name.append("_");
				std::cout << name << "'." << std::endl;
			}
			
			//std::cout << "Loading '" << name << "'." << std::endl;
			M[meshIdx] = &ResourcesManager::getInstance().getMesh(name);
			
			//std::cout << "Material Index: " << LoadedMesh->mMaterialIndex << std::endl;
			aiMaterial* m = scene->mMaterials[LoadedMesh->mMaterialIndex];
			
			aiString Texture;
			M[meshIdx]->getMaterial().setShadingProgram(ResourcesManager::getInstance().getProgram("Deferred"));
			if(m->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), Texture) == AI_SUCCESS)
			{
				std::string p = rep;
				p.append(Texture.C_Str());
				std::replace(p.begin(), p.end(), '\\', '/');
				//std::cout << "Loading " << p << std::endl;
				auto& t = ResourcesManager::getInstance().getTexture<Texture2D>(p);
				if(!t.isValid())
					t.load(p);
				if(t.isValid())
					M[meshIdx]->getMaterial().setUniform("Texture", t);
			}
			
			M[meshIdx]->getMaterial().setUniform("useNormalMap", 0);
			if(m->Get(AI_MATKEY_TEXTURE(aiTextureType_NORMALS, 0), Texture) == AI_SUCCESS
				|| m->Get(AI_MATKEY_TEXTURE(aiTextureType_HEIGHT, 0), Texture) == AI_SUCCESS) // Special case for .obj...
			{
				std::string p = rep;
				p.append(Texture.C_Str());
				std::replace(p.begin(), p.end(), '\\', '/');
				//std::cout << "Loading NM " << p << std::endl;
				auto& t = ResourcesManager::getInstance().getTexture<Texture2D>(p);
				if(!t.isValid())
					t.load(p);
				if(t.isValid())
				{
					M[meshIdx]->getMaterial().setUniform("NormalMap", t);
					M[meshIdx]->getMaterial().setUniform("useNormalMap", 1);
				}
			}
			
			/*
			M[meshIdx]->getMaterial().setUniform("useBumpMap", 0);
			if(m->Get(AI_MATKEY_TEXTURE(aiTextureType_DISPLACEMENT, 0), Texture) == AI_SUCCESS)
			{
				std::string p = rep;
				p.append(Texture.C_Str());
				std::replace(p.begin(), p.end(), '\\', '/');
				std::cout << "Loading BumpMap " << p << std::endl;
				auto& t = ResourcesManager::getInstance().getTexture<Texture2D>(p);
				if(!t.isValid())
					t.load(p);
				if(t.isValid())
				{
					M[meshIdx]->getMaterial().setUniform("BumpMap", t);
					M[meshIdx]->getMaterial().setUniform("useBumpMap", 1);
				}
			}
			*/
			
			aiVector3D* n = LoadedMesh->mNormals;
			aiVector3D** t = LoadedMesh->mTextureCoords;
			aiVector3D& minmax = LoadedMesh->mVertices[0];
			M[meshIdx]->_bbox.min = glm::vec3(minmax.x, minmax.y, minmax.z);
			M[meshIdx]->_bbox.max = glm::vec3(minmax.x, minmax.y, minmax.z);
			for(unsigned int i = 0; i < LoadedMesh->mNumVertices; ++i)
			{
				aiVector3D& v = LoadedMesh->mVertices[i];
				M[meshIdx]->getVertices().push_back(Mesh::Vertex(glm::vec3(v.x, v.y, v.z),
																				(n == nullptr) ? glm::vec3() : glm::vec3((*(n + i)).x, (*(n + i)).y, (*(n + i)).z),
																				(t == nullptr) ? glm::vec2() : glm::vec2(t[0][i].x, t[0][i].y)));
				
				M[meshIdx]->_bbox.min = glm::vec3(std::min(M[meshIdx]->_bbox.min.x, v.x), std::min(M[meshIdx]->_bbox.min.y, v.y), std::min(M[meshIdx]->_bbox.min.z, v.z));
				M[meshIdx]->_bbox.max = glm::vec3(std::max(M[meshIdx]->_bbox.max.x, v.x), std::max(M[meshIdx]->_bbox.max.y, v.y), std::max(M[meshIdx]->_bbox.max.z, v.z));
			}
			
			for(unsigned int i = 0; i < LoadedMesh->mNumFaces; ++i)
			{
				aiFace& f = LoadedMesh->mFaces[i];
				unsigned int* idx = f.mIndices;
				M[meshIdx]->getTriangles().push_back(Mesh::Triangle(idx[0], idx[1], idx[2]));
			}
		}
	}
	
	//std::cout << "Loading of " << path << " done." << std::endl;
	
	return M;
}
