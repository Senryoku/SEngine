#pragma once

#include <DirectionalLight.hpp>

/**
 * SpotLight
 *
 * Describes a spotlight which can cast shadows (Variance Shadow Mapping).
**/
class SpotLight : public DirectionalLight
{
public:
	struct GPUData
	{
		glm::vec4	position_range;
		glm::vec4	color_info;
		glm::mat4	depthMVP;
	};

	/**
	 * Constructor
	 *
	 * @param shadowMapResolution Resolution of the shadow map depth texture.
	**/
	SpotLight(unsigned int shadowMapResolution = 2048);
	
	/**
	 * Destructor
	**/
	virtual ~SpotLight() =default;

	/**
	 * @return Position of the light
	**/
	inline const glm::vec3& getPosition() const { return _position; }
	
	/**
	 * @return Range of the light
	**/
	inline float getRange() const { return _range; }
	
	/**
	 * @return SpotLight's data structured for GPU use.
	**/
	inline GPUData getGPUData() const { return GPUData{glm::vec4(getPosition(), _range),  glm::vec4(glm::vec3(getColor()), 0.0), getBiasedMatrix()}; }

	/**
	 * Modifies the position of the light source
	 * @param pos New position
	**/
	inline void setPosition(const glm::vec3& pos) { _position = pos; updateMatrices(); }
	
	/**
	 * Modifies the direction of the light source to look at the specified point in World Space
	 * @param at SpotLight's new focus point
	**/
	inline void lookAt(const glm::vec3& at) { setDirection(at - _position); }
	
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
	 * Updates SpotLight's internal transformation matrices according to
	 * its current position/direction/range.
	**/
	virtual void updateMatrices() override;
	
protected:	
	glm::vec3			_position = glm::vec3(0.f);				///< SpotLight's position in World Space
	float				_angle = 0.7853975;						///< SpotLight's opening angle (rad)
	float				_range = 1000.0; 							///< SpotLight's range, mainly used for the Shadow Mapping settings
};
