#pragma once

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include <Buffer.hpp>
#include <Frustum.hpp>

class Camera
{
public:
	Camera(glm::vec3 position = BasePosition,
		   glm::vec3 direction = BaseDirection,
		   glm::vec3 up = BaseUp,
		   float speed = BaseSpeed,
		   float sensitivity = BaseSensitivity);
	~Camera() =default;
	
	void strafeRight(float dt = 1.f);
	void strafeLeft(float dt = 1.f);
	
	void moveForward(float dt = 1.f);
	void moveBackward(float dt = 1.f);
	void moveUp(float dt = 1.f);
	void moveDown(float dt = 1.f);
	
	void look(glm::vec2 v);
	
	void updateView();
	void updateProjection(float ratio);
	void updateGPUBuffer();
	
	void reset();
	
	inline float getFoV()                         const { return _fov; }
	inline const glm::mat4& getViewMatrix()       const { return _viewMatrix; }
	inline const glm::mat4& getProjectionMatrix() const { return _projection; }
	inline const glm::mat4& getInvProjection()    const { return _invProjection; }
	inline const glm::mat4& getInvViewMatrix()    const { return _invViewMatrix; }
	inline auto& getGPUBuffer()                         { return _cameraBuffer; }
	inline const auto& getGPUBuffer()             const { return _cameraBuffer; }
	
	inline void setPosition(const glm::vec3& pos)  { _position = pos; }
	inline void setDirection(const glm::vec3& dir) { _direction = dir; }
	inline void lookAt(const glm::vec3& at)        { _direction = glm::normalize(at - _position); }
	
	inline void setFoV(float fov)                       { _fov = fov; updateProjection(_ratio); }
	
	inline glm::vec3& getPosition()              { return _position; }
	inline const glm::vec3& getPosition()  const { return _position; }
	inline const glm::vec3& getDirection() const { return _direction; }
	inline const glm::vec3& getUp()        const { return _up; }
	inline const glm::vec3& getRight()     const { return _cross; }
	
	inline const Frustum& getFrustum() const { return _frustum; }
	inline void updateFrustum()
	{
		_frustum = Frustum(getProjectionMatrix() * getViewMatrix());
	}
	
	// Public attributes
	
	float speed;
	float sensitivity;
	
	static const glm::vec3 BasePosition;
	static const glm::vec3 BaseDirection;
	static const glm::vec3 BaseUp;
	static const float     BaseSpeed;
	static const float     BaseSensitivity;
	
private:	
	glm::vec3		_position;
	glm::vec3		_direction;
	glm::vec3		_up;
		
	glm::vec3		_cross;
	glm::vec2		_moveMouvement;
		
	glm::mat4		_viewMatrix;
	
	// Projection data
	float			_fov = 60.0;
	float 			_ratio = 16.0/9.0;
	float 			_near = 0.1f;
	float 			_far = 1000.0f;
	glm::mat4 		_projection;
	
	Frustum			_frustum;
	
	// Cache
	glm::mat4 		_invProjection;
	glm::mat4 		_invViewMatrix;
	glm::mat4		_invViewProjection;
	
	// GPU Data (Move it in application/renderer?)
	struct GPUViewProjection
	{
		glm::mat4	view;
		glm::mat4	projection;
	};
	GPUViewProjection	_gpuCameraData;
	UniformBuffer		_cameraBuffer;
};
