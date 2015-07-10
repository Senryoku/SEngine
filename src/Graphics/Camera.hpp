#pragma once

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

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
	
	void reset();
	
	inline const glm::mat4& getMatrix() const { return _matrix; }
	
	inline void setPosition(const glm::vec3& pos) { _position = pos; }
	inline void setDirection(const glm::vec3& dir) { _direction = dir; }
	inline void lookAt(const glm::vec3& at) { _direction = glm::normalize(at - _position); }
	
	inline glm::vec3& getPosition() { return _position; }
	inline const glm::vec3& getPosition() const { return _position; }
	inline const glm::vec3& getDirection() const { return _direction; }
	inline const glm::vec3& getUp() const { return _up; }
	inline const glm::vec3& getRight() const { return _cross; }
	inline float& speed() { return _speed; }
	inline float& sensitivity() { return _sensitivity; }
	
private:
	float		_speed;
	float		_sensitivity;

	glm::vec3	_position;
	glm::vec3	_direction;
	glm::vec3	_up;
	
	glm::vec3	_cross;
	glm::vec2	_moveMouvement;
	
	glm::mat4	_matrix;
	
	static const glm::vec3 BasePosition;
	static const glm::vec3 BaseDirection;
	static const glm::vec3 BaseUp;
	static const float BaseSpeed;
	static const float BaseSensitivity;
};
