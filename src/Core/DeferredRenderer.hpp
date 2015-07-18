#pragma once

#include <Application.hpp>

class DeferredRenderer : public Application
{
public:
	DeferredRenderer();
	DeferredRenderer(int argc, char* argv[]);
	virtual ~DeferredRenderer() =default;
	
	virtual void run_init() override;

protected:
	Framebuffer<Texture2D, 3>		_offscreenRender;
	
	// Post Process settings
	float		_exposure = 2.0f;
	float		_bloom = 1.2f;
	int			_bloomBlur = 1;
	int			_bloomDownsampling = 1;
	glm::vec3	_ambiant = glm::vec3(0.06);
	
	float		_minVariance = 0.0000001f;
	int			_aoSamples = 16;
	float		_aoThreshold = 1.0f;
	float		_aoRadius = 200.0f;

	virtual void render() override;
	
	virtual void renderGBuffer();
	virtual void renderGBufferPost() {};
	virtual void renderLightPass();
	virtual void renderPostProcess();
	
	virtual void resize_callback(GLFWwindow* _window, int width, int height) override;
	virtual void key_callback(GLFWwindow* _window, int key, int scancode, int action, int mods) override;
};
