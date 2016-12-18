#pragma once

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include <Frustum.hpp>
#include <Texture2D.hpp>
#include <Framebuffer.hpp>
#include <MeshRenderer.hpp>
#include <Shaders.hpp>
#include <serialization.hpp>

/**
 * SpotLight
 *
 * Describes a spotlight which can cast shadows (Variance Shadow Mapping).
**/
class SpotLight
{
public:
	struct GPUData
	{
		glm::vec4	position_range;
		glm::vec4	color_info;
		glm::mat4	depthMVP;
	};
	using ShadowBuffer = Framebuffer<Texture2D, 1, Texture2D, true>;
	
	// Public attributes
	bool			dynamic = false;	///< Tells the application if the shadow map should be redrawn each frame
	unsigned int	downsampling = 1;	///< Strengh of the shadow map's downsampling
	
	/**
	 * Constructor
	 *
	 * @param shadowMapResolution Resolution of the shadow map depth texture.
	**/
	SpotLight(unsigned int shadowMapResolution = 2048);
	
	SpotLight(const nlohmann::json& json);
	
	/**
	 * Destructor
	**/
	~SpotLight() =default;

	/**
	 * Initialize the shadow mapping attributes (Shaders, Framebuffer...)
	 * for this light.
	**/
	void init();
	
	/// @return RW Reference to the color of the light
	inline glm::vec3& getColor() { return _color; }
	
	/// @return Color of the light
	inline const glm::vec3& getColor() const { return _color; }
	
	/// @return SpotLight's GPU buffer.
	inline const UniformBuffer& getGPUBuffer() const { return _gpuBuffer; }
	
	/// @return SpotLight's data structured for GPU use.
	inline GPUData getGPUData() const { return GPUData{glm::vec4(getTransformation().getPosition(), _range),  
															glm::vec4(glm::vec3(getColor()), 0.0), 
															getBiasedMatrix()}; }
	
	/// @return Range of the light
	inline float getRange() const { return _range; }
	
	inline float getAngle() const { return _angle; }

	/// @return Light's projection matrix.
	inline const glm::mat4& getProjectionMatrix() const { return _projection; }

	/// @return Light's shadow map frame buffer.
	inline const ShadowBuffer& getShadowBuffer() const { return _shadowMapFramebuffer; }
	
	/// @return Light's shadow map depth texture.
	inline const Texture2D& getShadowMap() const { return _shadowMapFramebuffer.getColor(); }
	
	inline size_t getResolution() const { return _shadowMapResolution; }
	
	/// @return World to SpotLight's view space matrix.
	inline const glm::mat4& getViewMatrix() const { return _view; }

	/**
	 * Returns the transformation matrix (Projection * View) for this SpotLight. 
	 *
	 * @return World to SpotLight's screen space matrix.
	 * @see getBiasedMatrix()
	**/
	inline const glm::mat4& getMatrix() const { return _VPMatrix; }

	/**
	 * Returns the biased transformation matrix (Projection * View) for this SpotLight.
	 * (Biased meaning "in [0, 1] range", i.e. texture friendly :])
	 *
	 * @return biased World to SpotLight's view space matrix.
	 * @see getMatrix()
	**/
	inline const glm::mat4& getBiasedMatrix() const { return _biasedVPMatrix; }
	
	/**
	 * Modifies the color of the light source
	 * @param col New Color
	**/
	inline void setColor(const glm::vec3& col) { _color = col; }

	/**
	 * Modifies the range of the light source
	 * @param r New range
	**/
	void setRange(float r);
	
	/**
	 * Modifies the opening angle of the light source
	 * @param a New angle (rad)
	**/
	void setAngle(float a);

	/**
	 * Sets the projection matrix for this Light. 
	**/
	inline void setProjectionMatrix(const glm::mat4& p) { _projection = p; updateMatrices(); }
	
	/**
	 * Warning! Reconstruct the buffer (obviously)
	**/
	inline void setResolution(size_t r)
	{
		_shadowMapResolution = r;
		_shadowMapFramebuffer = ShadowBuffer(_shadowMapResolution);
		updateMatrices();
		init();
	}
	
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
	void drawShadowMap(const ComponentIterator<MeshRenderer>& objects) const;
	
	/**
	 * Updates SpotLight's internal transformation matrices according to
	 * its current position/direction/range.
	**/
	void updateMatrices();
	
	// Static
	
	inline static const glm::mat4& getBiasMatrix() { return s_depthBiasMVP; }
	
	/**
	 * @return Program used to draw the shadow map.
	**/
	inline static const Program& getShadowMapProgram()
	{
		assert(s_depthProgram != nullptr);
		return *s_depthProgram;
	}
	
protected:
	EntityID			_entity = invalid_entity;
	
	glm::vec3			_color = glm::vec3(1.f);	///< Light's color
	float				_angle = 0.7853975;			///< SpotLight's opening angle (rad)
	float				_range = 1000.0; 			///< SpotLight's range, mainly used for the Shadow Mapping settings
	
	unsigned int		_shadowMapResolution;		///< Resolution of the shadow map (depth map)
	ShadowBuffer		_shadowMapFramebuffer;		///< Framebuffer used to draw the shadow map
	glm::mat4			_projection;				///< Projection matrix used to draw the shadow map
	UniformBuffer		_gpuBuffer;					///< Buffer used for shadow mapping
	glm::mat4			_view;						///< View matrix computed from the transformation
	glm::mat4			_VPMatrix;					///< ViewProjection matrix used to draw the shadow map
	glm::mat4			_biasedVPMatrix;			///< Biased ViewProjection matrix used to compute the shadows projected on the scene
	
	inline const Transformation& getTransformation() const { return entities[_entity].get<Transformation>(); }
	
	// Static
	static void initPrograms();
	static const glm::mat4	s_depthBiasMVP;	///< Used to compute the biased ViewProjection matrix
	static Program* 		s_depthProgram;	///< Program used to draw the shadow map
};
