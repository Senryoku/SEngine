#pragma once

#include <Texture2D.hpp>
#include <Light.hpp>

class DirectionalLight : public Light<Texture2D>
{
public:
	DirectionalLight(unsigned int shadowMapResolution);
	virtual ~DirectionalLight() =default;

	virtual void init() override;
	virtual void bind() const override;
	virtual void unbind() const override;
	virtual void drawShadowMap(const ComponentIterator<MeshRenderer>& objects) const override;
	
	/**
	 * @return Direction of the light
	**/
	inline const glm::vec3& getDirection() const { return _direction; }
	
	/**
	 * Modifies the direction of the light source
	 * @param dir New direction
	**/
	inline void setDirection(const glm::vec3& dir) { _direction = glm::normalize(dir); updateMatrices(); }
	
	/**
	 * Returns the view matrix for this SpotLight. 
	 *
	 * @return World to SpotLight's view space matrix.
	**/
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

	// Static
	
	inline static const glm::mat4& getBiasMatrix() { return s_depthBiasMVP; }
	
protected:
	glm::vec3			_direction = glm::vec3(0.f, 0.f, 1.f);	///< Light's direction in World Space
	
	glm::mat4			_view;					///< View matrix used to draw the shadow map
	glm::mat4			_VPMatrix;				///< ViewProjection matrix used to draw the shadow map
	glm::mat4			_biasedVPMatrix;		///< Biased ViewProjection matrix used to compute the shadows projected on the scene
	
	virtual void initPrograms() override;
	
	// Static
	static const glm::mat4	s_depthBiasMVP;	///< Used to compute the biased ViewProjection matrix
};
