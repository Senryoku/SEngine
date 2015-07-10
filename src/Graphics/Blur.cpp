#include <Blur.hpp>

#include <ResourcesManager.hpp>

void blur(const Texture2D& t, size_t resx, size_t resy, unsigned int level)
{
	assert(resx > 0);
	if(resy == 0)
		resy = resx;
	
	ComputeShader& GaussianBlurH = ResourcesManager::getInstance().getShader<ComputeShader>("GaussianBlurH");
	if(!GaussianBlurH)
	{
		GaussianBlurH.loadFromFile("src/GLSL/gaussian_blur_h_cs.glsl");
		GaussianBlurH.compile();
	}
	ComputeShader& GaussianBlurV = ResourcesManager::getInstance().getShader<ComputeShader>("GaussianBlurV");
	if(!GaussianBlurV)
	{
		GaussianBlurV.loadFromFile("src/GLSL/gaussian_blur_v_cs.glsl");
		GaussianBlurV.compile();
	}
	
	t.bindImage(0, level, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	GaussianBlurH.getProgram().setUniform("Texture", (int) 0);
	GaussianBlurH.compute(resx / GaussianBlurH.getWorkgroupSize().x + 1, resy, 1);
	GaussianBlurH.memoryBarrier();
	GaussianBlurV.getProgram().setUniform("Texture", (int) 0);
	GaussianBlurV.compute(resx, resy / GaussianBlurV.getWorkgroupSize().y + 1, 1);
	GaussianBlurV.memoryBarrier();
}
