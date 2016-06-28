#pragma once

#include <Application.hpp>

class DeferredRenderer : public Application
{
public:
	DeferredRenderer() =default;
	DeferredRenderer(int argc, char* argv[]);
	virtual ~DeferredRenderer() =default;
	
	inline size_t getInternalWidth() const
	{
		return (_internalWidth != 0 && _internalHeight != 0) ?
				_internalWidth : _width;
	}
	
	inline size_t getInternalHeight() const
	{
		return (_internalWidth != 0 && _internalHeight != 0) ?
				_internalHeight : _height;
	}
	
	virtual void screen(const std::string& path) const override;
	
	void setInternalResolution(size_t width, size_t height);
	
	virtual void init(const std::string& windowName = "Default Window") override;
	
	virtual void run_init() override;

protected:
	/**
	 * G-Buffer:
	 *  Color0 : Color (xyz) and MaterialInfo (w)
	 *  Color1 : World Position (xyz) and Depth (w)
	 *  Color2 : Encoded Normal (xy), F0 (z) and R (w)
	**/
	Framebuffer<Texture2D, 3>		_offscreenRender;
	
	// Downsampling
	bool		_postProcessBlur = false;
	size_t		_internalWidth = 0;
	size_t		_internalHeight = 0;
	
	// Post Process settings
	float		_exposure = 2.0f;
	float		_bloom = 1.2f;
	int			_bloomBlur = 2;
	int			_bloomDownsampling = 3;
	glm::vec3	_ambiant = glm::vec3(0.06);
	
	float		_minVariance = 0.0000001f;
	int			_aoSamples = 16;
	float		_aoThreshold = 1.0f;
	float		_aoRadius = 200.0f;
	
	int			_volumeSamples = 0;
	float		_atmosphericDensity = 0.005f;
	
	Query		_updateTiming;
	Query		_GBufferPassTiming;
	Query		_lightPassTiming;
	Query		_postProcessTiming;
	Query		_GUITiming;
	
	virtual void initGBuffer(size_t width, size_t height);

	virtual void render() override;
	
	virtual void renderGBuffer();
	virtual void renderGBufferPost() {};
	virtual void renderLightPass();
	virtual void renderPostProcess();
	
	virtual void resize_callback(GLFWwindow* _window, int width, int height) override;
};
