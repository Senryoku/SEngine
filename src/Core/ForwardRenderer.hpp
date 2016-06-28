#pragma once

#include <array>

#include <Application.hpp>

/**
 * @todo Cascade Shadow Mapping
**/
class ForwardRenderer : public Application
{
public:
	ForwardRenderer();
	ForwardRenderer(int argc, char* argv[]);
	virtual ~ForwardRenderer() =default;
	
	virtual void run_init() override;

	virtual void update() override;
	
protected:
	virtual void render() override;
	
	void update_cascade_matrices();
	
	using ShadowBuffer = Framebuffer<Texture2D, 1, Texture2D, true>;
	static constexpr size_t CascadeCount = 3;
	static constexpr size_t CascadeResolution = 1024;
	
	std::array<ShadowBuffer, CascadeCount>	_shadow_maps;
	glm::mat4								_cascade_light_view;
	glm::mat4								_cascade_proj[CascadeCount];
	glm::mat4								_cascade_matrices[CascadeCount];
	glm::mat4								_cascade_matrices_biased[CascadeCount];
	glm::vec3								_light_direction = glm::normalize(glm::vec3(0.5));
	float									_cascade_far[CascadeCount];
};
