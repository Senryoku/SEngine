#include <Terrain.hpp>

Mesh create(const Terrain& t, const glm::vec2& start, const glm::vec2& end, const glm::ivec2& precision)
{
	Mesh m;
	auto d = glm::abs(end - start) * glm::vec2(1.f / precision.x, 1.f / precision.y);
	for(int i = 0; i < precision.x; ++i)
		for(int j = 0; j < precision.y; ++j)
		{
			auto v = glm::vec2(i * d.x, j * d.y);
			m.getVertices().push_back(Mesh::Vertex(glm::vec3(v.x, t(v.x, v.y), v.y), 
										glm::vec3(0.0), 
										glm::vec2(i, j)
							));
		}

	// Add Faces
	for(int i = 0; i < precision.x - 1; ++i)
	{
		for(int j = 0; j < precision.y - 1; ++j)
		{
			const size_t v[3] = {(size_t) i*precision.y + j, 
							   (size_t) i*precision.y + j + 1, 
							   (size_t) (i + 1)*precision.y + j};
			m.getTriangles().push_back(Mesh::Triangle(v[0],
													v[1],
													v[2]));
							 
			const size_t v2[3] = {(size_t) (i + 1)*precision.y + j,
								(size_t) i*precision.y + j + 1,
								(size_t) (i + 1)*precision.y + j + 1};
			m.getTriangles().push_back(Mesh::Triangle(v2[0],
													v2[1],
													v2[2]));
		}
	}
	
	m.computeNormals();
		
	return m;
}
