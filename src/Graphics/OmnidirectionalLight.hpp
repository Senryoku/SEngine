#pragma once

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include <Frustum.hpp>
#include <CubeMap.hpp>
#include <Framebuffer.hpp>
#include <MeshRenderer.hpp>
#include <Shaders.hpp>

/**
 * OmnidirectionalLight
**/
class OmnidirectionalLight
{
public:
	using ShadowBuffer = Framebuffer<CubeMap, 1, CubeMap, true>;
	
	struct GPUData
	{
		glm::vec4	position_range;
		glm::vec4	color_info;
	};
	
	// Public attributes
	bool			dynamic = false;	///< Tells the application if the shadow map should be redrawn each frame
	unsigned int	downsampling = 0;	///< Strengh of the shadow map's downsampling

	/**
	 * Constructor
	 *
	 * @param shadowMapResolution Resolution of the shadow map depth texture.
	**/
	OmnidirectionalLight(unsigned int shadowMapResolution = 2048);
	
	/**
	 * Destructor
	**/
	~OmnidirectionalLight() =default;

	/**
	 * Initialize the shadow mapping attributes (Shaders, Framebuffer...)
	 * for this light.
	**/
	void init();
	
	/**
	 * @return Color of the light
	**/
	inline glm::vec3& getColor() { return _color; }
	inline const glm::vec3& getColor() const { return _color; }
	
	/**
	 * @return Position of the light
	**/
	inline const glm::vec3& getPosition() const { return _position; }
	
	/**
	 * @return Range of the light
	**/
	inline float getRange() const { return _range; }
	
	/**
	 * @return Light's data structured for GPU use.
	**/
	inline GPUData getGPUData() const { return GPUData{glm::vec4(getPosition(), _range),
															glm::vec4(glm::vec3(getColor()), 1.0)}; }
	
	inline const UniformBuffer& getGPUBuffer() const { return _gpuBuffer; }
	
	/**
	 * Modifies the color of the light source
	 * @param col New Color
	**/
	inline void setColor(const glm::vec3& col) { _color = col; }
	
	/**
	 * Modifies the position of the light source
	 * @param pos New position
	**/
	inline void setPosition(const glm::vec3& pos) { _position = pos; updateMatrices(); }
	
	/**
	 * Modifies the range of the light source
	 * @param r New range
	**/
	void setRange(float r);
	
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
	 * @return Light's shadow map framme buffer.
	**/
	inline const ShadowBuffer& getShadowBuffer() const { return _shadowMapFramebuffer; }
	
	/**
	 * @return Light's shadow map depth texture.
	**/
	inline const CubeMap& getShadowMap() const { return _shadowMapFramebuffer.getColor(); }
	
	inline size_t getResolution() const { return _shadowMapResolution; }
	
	inline void setResolution(size_t r)
	{
		_shadowMapResolution = r;
		_shadowMapFramebuffer = ShadowBuffer(_shadowMapResolution);
	}
	
	/**
	 * Updates Light's internal transformation matrices according to
	 * its current position/direction/range.
	**/
	void updateMatrices();
	
	/**
	 * Setup the context to draw to this light' shadow map.
	 * @todo Find a better name...
	**/
	void bind() const;

	/**
	 * Restores the default framebuffer.
	 * @todo Find a better name...
	**/
	void unbind() const;
	
	/**
	 * Draws passed objects to this light's shadow map
	**/
	template<template<typename> typename C>
	void drawShadowMap(const C<MeshRenderer>& objects) const;
	
	// Static
	
	/**
	 * @return Program used to draw the shadow map.
	**/
	inline static const Program& getShadowMapProgram();
	
	/**
	 * @return Program used to draw the shadow map for instanced Draw Calls.
	**/
	inline static const Program& getShadowMapInstanceProgram();
	
protected:
	glm::vec3			_color = glm::vec3(1.f);		///< Light's color
	
	glm::vec3			_position = glm::vec3(0.f);		///< Light's position in World Space
	float				_range = 1000.0; 				///< Light's range, mainly used for the Shadow Mapping settings

	unsigned int		_shadowMapResolution;			///< Resolution of the shadow map (depth map)
	ShadowBuffer		_shadowMapFramebuffer;			///< Framebuffer used to draw the shadow map
	glm::mat4			_projection;					///< Projection matrix used to draw the shadow map
	UniformBuffer		_gpuBuffer;						///< Buffer used for shadow mapping
	
	// Static	
	static Program* 			s_depthProgram;	///< Program used to draw the shadow map
    static VertexShader*		s_depthVS;		///< VertexShader used to draw the shadow map
    static GeometryShader*	s_depthGS;			///< GeometryShader used to draw the shadow map
    static FragmentShader*	s_depthFS;			///< FragmentShader used to draw the shadow map
	
	static void initPrograms();
};

// Inlined functions

inline const Program& OmnidirectionalLight::getShadowMapProgram()
{
	assert(s_depthProgram != nullptr);
	return *s_depthProgram;
}

template<template<typename> typename C>
void OmnidirectionalLight::drawShadowMap(const C<MeshRenderer>& objects) const
{
	//getShadowMap().set(Texture::Parameter::BaseLevel, 0);
	
	BoundingSphere BoundingVolume(_position, _range);
	
	bind();
	Context::disable(Capability::CullFace);
	
	for(auto& b : objects)
	{
		if(intersect(b.getAABB(), BoundingVolume))
		{
			getShadowMapProgram().setUniform("ModelMatrix", b.getTransformation().getModelMatrix());
			b.getMesh().draw();
		}
	}
		
	unbind();
	
	//getShadowMap().generateMipmaps();
	/// @todo Good blur for Cubemaps
	//getShadowMap().set(Texture::Parameter::BaseLevel, downsampling);
}