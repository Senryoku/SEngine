#pragma once

#include <glm/glm.hpp>

#include <CubeMap.hpp>
#include <Buffer.hpp>
#include <VertexArray.hpp>

class Skybox
{
public:
	Skybox();
	Skybox(const std::array<std::string, 6>& Paths);
	Skybox(const CubeMap& cubemap);
	~Skybox() =default;
	
	void draw(const glm::mat4& p, const glm::mat4& mv) const;
	void cubedraw() const;
	void loadCubeMap(const std::array<std::string, 6>& Paths);
	
	CubeMap& getCubemap() { return _cubeMap; }
	
	operator bool() const { return _cubeMap; }
	
private:
	CubeMap	_cubeMap;
	
	static void init();
	
	static VertexArray		s_vao;
	static Buffer			s_vertex_buffer;
	static Buffer			s_index_buffer;
};
