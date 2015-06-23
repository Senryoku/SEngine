#pragma once

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include <Frustum.hpp>
#include <Texture2D.hpp>
#include <Framebuffer.hpp>
#include <MeshInstance.hpp>
#include <Shaders.hpp>

/**
 * Light
 *
 * Describes a spotlight which can cast shadows (Shadow Mapping).
 * @todo Internalize shadow map drawing
 * @todo Re-use frustum
**/
class Light
{
public:
	using ShadowBuffer = Framebuffer<Texture2D, 0, Texture2D, true>;
	
	struct GPUData
	{
		glm::vec4	position;
		glm::vec4	color;
		glm::mat4	depthMVP;
	};

	/**
	 * Constructor
	 *
	 * @param shadowMapResolution Resolution of the shadow map depth texture.
	**/
	Light(unsigned int shadowMapResolution = 4096);
	
	/**
	 * Destructor
	**/
	~Light() =default;

	/**
	 * Initialize the shadow mapping attributes (Shaders, Framebuffer...)
	 * for this light.
	**/
	void init();
	
	/**
	 * @return Color of the light
	**/
	inline const glm::vec4& getColor() const { return _color; }
	
	/**
	 * @return Position of the light
	**/
	inline const glm::vec3& getPosition() const { return _position; }
	
	/**
	 * @return Direction of the light
	**/
	inline const glm::vec3& getDirection() const { return _direction; }
	
	/**
	 * @return Range of the light
	**/
	inline float getRange() const { return _range; }
	
	/**
	 * @return Light's data structured for GPU use.
	**/
	inline GPUData getGPUData() const { return GPUData{glm::vec4(getPosition(), 1.0),  getColor(), getBiasedMatrix()}; }
	
	inline const UniformBuffer& getGPUBuffer() const { return _gpuBuffer; }
	
	/**
	 * Modifies the color of the light source
	 * @param col New Color
	**/
	inline void setColor(const glm::vec4& col) { _color = col; }
	
	/**
	 * Modifies the position of the light source
	 * @param pos New position
	**/
	inline void setPosition(const glm::vec3& pos) { _position = pos; }
	
	/**
	 * Modifies the direction of the light source (should be normalized)
	 * @param dir Normalized new direction
	**/
	inline void setDirection(const glm::vec3& dir) { _direction = dir; }
	
	/**
	 * Modifies the direction of the light source to look at the specified point in World Space
	 * @param at Light's new focus point
	**/
	inline void lookAt(const glm::vec3& at) { _direction = glm::normalize(at - _position); }
	
	/**
	 * Modifies the range of the light source
	 * @param r New range
	**/
	inline void setRange(float r) { _range = r; }
	
	/**
	 * Returns the view matrix for this Light. 
	 *
	 * @return World to Light's view space matrix.
	**/
	inline const glm::mat4& getViewMatrix() const { return _view; }
	
	/**
	 * Returns the projection matrix for this Light. 
	 *
	 * @return Light's projection matrix.
	**/
	inline const glm::mat4& getProjectionMatrix() const { return _projection; }
		
	/**
	 * Sets the projection matrix for this Light. 
	**/
	inline void setProjectionMatrix(const glm::mat4& p) { _projection = p; }
	
	/**
	 * Returns the transformation matrix (Projection * View) for this Light. 
	 *
	 * @return World to Light's screen space matrix.
	 * @see getBiasedMatrix()
	**/
	inline const glm::mat4& getMatrix() const { return _VPMatrix; }
	
	/**
	 * Returns the biased transformation matrix (Projection * View) for this Light.
	 * (Biased meaning "in [0, 1] range", i.e. texture friendly :])
	 *
	 * @return biased World to Light's view space matrix.
	 * @see getMatrix()
	**/
	inline const glm::mat4& getBiasedMatrix() const { return _biasedVPMatrix; }
	
	/**
	 * @return Light's shadow map framme buffer.
	**/
	inline const ShadowBuffer& getShadowBuffer() const { return _shadowMapFramebuffer; }
	
	/**
	 * @return Light's shadow map depth texture.
	**/
	inline const Texture2D& getShadowMap() const { return _shadowMapFramebuffer.getDepth(); }
	
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
	 * Setup the context to draw to this light' shadow map, assuming instanced draw calls.
	 * @todo Find a better name... (Even more urgent.)
	**/
	void bindInstanced() const;
	
	/**
	 * Restores the default framebuffer.
	 * @todo Find a better name...
	**/
	void unbind() const;
	
	/**
	 * Draws passed objects to this light's shadow map
	**/
	void drawShadowMap(const std::vector<MeshInstance>& objects) const;
	
	// Static
	
	/**
	 * @return Bias matrix (from screen space to texture space)
	**/
	inline static const glm::mat4& getBiasMatrix();
	
	/**
	 * @return Program used to draw the shadow map.
	**/
	inline static const Program& getShadowMapProgram();
	
	/**
	 * @return Program used to draw the shadow map for instanced Draw Calls.
	**/
	inline static const Program& getShadowMapInstanceProgram();
	
protected:
	glm::vec4			_color = glm::vec4(1.f);				///< Light's color
	
	glm::vec3			_position = glm::vec3(0.f);				///< Light's position in World Space
	glm::vec3			_direction = glm::vec3(0.f, 0.f, 1.f);	///< Light's direction in World Space
	float				_range = 1000.0; 							///< Light's range, mainly used for the Shadow Mapping settings

	unsigned int		_shadowMapResolution = 4096;	///< Resolution of the shadow map (depth map)
	ShadowBuffer		_shadowMapFramebuffer;		///< Framebuffer used to draw the shadow map
	glm::mat4			_view;							///< View matrix used to draw the shadow map
	glm::mat4			_projection;					///< Projection matrix used to draw the shadow map
	glm::mat4			_VPMatrix;						///< ViewProjection matrix used to draw the shadow map
	glm::mat4			_biasedVPMatrix;				///< Biased ViewProjection matrix used to compute the shadows projected on the scene
	UniformBuffer		_gpuBuffer;					///< Buffer used for shadow mapping
	
	// Static
	static const glm::mat4	s_depthBiasMVP;	///< Used to compute the biased ViewProjection matrix
	
	static Program* 			s_depthProgram;	///< Program used to draw the shadow map
    static VertexShader*		s_depthVS;			///< VertexShader used to draw the shadow map
    static FragmentShader*	s_depthFS;			///< FragmentShader used to draw the shadow map
	
	static Program* 		s_depthInstanceProgram;	///< Program used to draw the shadow map for instanced Draw Calls
    static VertexShader*	s_depthInstanceVS;		///< VertexShader used to draw the shadow map for instanced Draw Calls
	
	static void initPrograms();
};

// Inlined functions

inline const glm::mat4& Light::getBiasMatrix()
{
	return s_depthBiasMVP;
}

inline const Program& Light::getShadowMapProgram()
{
	assert(s_depthProgram != nullptr);
	return *s_depthProgram;
}

inline const Program& Light::getShadowMapInstanceProgram()
{
	assert(s_depthInstanceProgram != nullptr);
	return *s_depthInstanceProgram;
}
