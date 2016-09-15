#pragma once

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include <Frustum.hpp>
#include <Texture2D.hpp>
#include <Framebuffer.hpp>
#include <MeshRenderer.hpp>
#include <Shaders.hpp>

/**
 * ShadowCastingLight
**/
template<typename T>
class Light
{
public:
	using ShadowBuffer = Framebuffer<T, 1, T, true>;
	
	// Public attributes
	bool			dynamic = false;	///< Tells the application if the shadow map should be redrawn each frame
	unsigned int	downsampling = 1;	///< Strengh of the shadow map's downsampling

	/**
	 * Constructor
	 *
	 * @param shadowMapResolution Resolution of the shadow map depth texture.
	**/
	Light(unsigned int shadowMapResolution = 2048);
	
	/**
	 * Destructor
	**/
	virtual ~Light() =default;

	/**
	 * Initialize the shadow mapping attributes (Shaders, Framebuffer...)
	 * for this light.
	**/
	virtual void init() =0;
	
	/**
	 * @return RW Reference to the color of the light
	**/
	inline glm::vec3& getColor() { return _color; }
	
	/**
	 * @return Color of the light
	**/
	inline const glm::vec3& getColor() const { return _color; }
	
	/**
	 * @return SpotLight's GPU buffer.
	**/
	inline const UniformBuffer& getGPUBuffer() const { return _gpuBuffer; }
	
	/**
	 * Modifies the color of the light source
	 * @param col New Color
	**/
	inline void setColor(const glm::vec3& col) { _color = col; }

	/**
	 * Returns the projection matrix for this Light. 
	 *
	 * @return Light's projection matrix.
	**/
	inline const glm::mat4& getProjectionMatrix() const { return _projection; }

	/**
	 * Sets the projection matrix for this Light. 
	**/
	inline void setProjectionMatrix(const glm::mat4& p) { _projection = p; updateMatrices(); }

	/**
	 * @return Light's shadow map frame buffer.
	**/
	inline const ShadowBuffer& getShadowBuffer() const { return _shadowMapFramebuffer; }
	
	/**
	 * @return Light's shadow map depth texture.
	**/
	inline const T& getShadowMap() const { return _shadowMapFramebuffer.getColor(); }
	
	inline size_t getResolution() const { return _shadowMapResolution; }
	
	/**
	 * Warning!
	**/
	inline void setResolution(size_t r);
	
	/**
	 * Updates Light's internal transformation matrices according to
	 * its current position/direction/range.
	**/
	virtual void updateMatrices() =0;
	
	/**
	 * Setup the context to draw to this light' shadow map.
	 * @todo Find a better name...
	**/
	virtual void bind() const =0;
	
	/**
	 * Restores the default framebuffer.
	 * @todo Find a better name...
	**/
	virtual void unbind() const =0;
	
	/**
	 * Draws passed objects to this light's shadow map
	**/
	virtual void drawShadowMap(const ComponentIterator<MeshRenderer>& objects) const =0;
	
	// Static
	
	/**
	 * @return Program used to draw the shadow map.
	**/
	inline static const Program& getShadowMapProgram();
	
protected:
	glm::vec3			_color = glm::vec3(1.f);	///< Light's color
	
	unsigned int		_shadowMapResolution;		///< Resolution of the shadow map (depth map)
	ShadowBuffer		_shadowMapFramebuffer;		///< Framebuffer used to draw the shadow map
	glm::mat4			_projection;				///< Projection matrix used to draw the shadow map
	UniformBuffer		_gpuBuffer;					///< Buffer used for shadow mapping
	
	virtual void initPrograms() =0;
	
	// Static
	static Program* 		s_depthProgram;	///< Program used to draw the shadow map
};

// Inlined functions

template<typename T>
inline const Program& Light<T>::getShadowMapProgram()
{
	assert(s_depthProgram != nullptr);
	return *s_depthProgram;
}

template<typename T>
inline void Light<T>::setResolution(size_t r)
{
	_shadowMapResolution = r;
	_shadowMapFramebuffer = ShadowBuffer(_shadowMapResolution);
}

#include <Light.tcc>
