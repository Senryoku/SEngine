#include "Camera.hpp"

#include <glm/gtc/matrix_transform.hpp> // glm::lookAt
#include <glm/gtc/type_ptr.hpp> // glm::value_ptr

#include <MathTools.hpp>

const glm::vec3 Camera::BasePosition = {-1.f, 1.f, -1.f};
const glm::vec3 Camera::BaseDirection = {1.f, 0.f, 1.f};
const glm::vec3 Camera::BaseUp = {0.f, 1.f, 0.f};
const float Camera::BaseSpeed = 100.f;
const float Camera::BaseSensitivity = 0.1f;

Camera::Camera(glm::vec3 position,
			   glm::vec3 direction,
			   glm::vec3 up,
			   float speed,
			   float sensitivity) :
	_speed(speed),
	_sensitivity(sensitivity),
	_position(position),
	_direction(direction),
	_up(up)
{
	_cross = glm::normalize(glm::cross(_direction, _up));
}


void Camera::strafeRight(float dt)
{
	_position += dt * _speed * _cross;
}

void Camera::strafeLeft(float dt)
{
	_position -= dt * _speed * _cross;
}

void Camera::moveForward(float dt)
{
	_position += dt * _speed * _direction;
}

void Camera::moveBackward(float dt)
{
	_position -= dt * _speed * _direction;
}

void Camera::moveUp(float dt)
{
	_position += dt * _speed * _up;
}

void Camera::moveDown(float dt)
{
	_position -= dt * _speed * _up;
}

void Camera::look(glm::vec2 v)
{
	_moveMouvement += v*_sensitivity;
	if(_moveMouvement.y > 89) _moveMouvement.y = 89;
	else if (_moveMouvement.y < -89) _moveMouvement.y = -89;
	double r_temp = std::cos(_moveMouvement.y*pi()/180);
	_direction.x += r_temp*std::cos(_moveMouvement.x*pi()/180);
	_direction.y += std::sin(_moveMouvement.y*pi()/180);
	_direction.z += r_temp*std::sin(_moveMouvement.x*pi()/180);

	_direction = glm::normalize(_direction);
	_cross = glm::normalize(glm::cross(_direction, _up));
}

void Camera::reset()
{
	_speed = BaseSpeed;
	_sensitivity = BaseSensitivity;
	_position = BasePosition;
	_direction = BaseDirection;
	_up = BaseUp;
}

void Camera::updateView()
{
	_matrix = glm::lookAt(_position, _position + _direction, _up);
}
