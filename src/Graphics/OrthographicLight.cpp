#include <OrthographicLight.hpp>

#include <glm/gtc/matrix_transform.hpp> // glm::ortho

OrthographicLight::OrthographicLight(const Camera& target, unsigned int shadowMapResolution) : 
	DirectionalLight(shadowMapResolution),
	_camera(&target)
{
}

void OrthographicLight::updateMatrices()
{
	// Construct an orthonormal base from the light's direction
	glm::vec3 up{0, 1, 0};
	if(glm::cross(_direction, up) == glm::vec3(0.0))
		up = {0, 0, 1};
	glm::vec3 right = glm::cross(_direction, up);
	up = glm::cross(right, _direction);

	// @todo Automatically compute an optimal projection/view
	if(_camera != nullptr)
	{
		///@todo Get it to work.
		
		// Should be the real thing.
		float inRad = 60.f * glm::pi<float>()/180.f;
		auto p = glm::perspective(inRad, 1366.0f/720.0f, 0.1f, 1000.0f);
		
		glm::mat4 inverse = glm::inverse(p * _camera->getMatrix());
		glm::vec4 Bounds4[8] = {
					inverse * glm::vec4(-1.0, -1.0, -1.0, 1.0),
					inverse * glm::vec4( 1.0, -1.0, -1.0, 1.0),
					inverse * glm::vec4(-1.0,  1.0, -1.0, 1.0),
					inverse * glm::vec4( 1.0,  1.0, -1.0, 1.0),
					inverse * glm::vec4(-1.0, -1.0,  1.0, 1.0),
					inverse * glm::vec4( 1.0, -1.0,  1.0, 1.0),
					inverse * glm::vec4(-1.0,  1.0,  1.0, 1.0),
					inverse * glm::vec4( 1.0,  1.0,  1.0, 1.0)
		};
		
		glm::vec3 Bounds[8];
		for(int i = 0; i < 8; ++i)
			Bounds[i] = glm::vec3(Bounds4[i] / Bounds4[i].w);
		
		glm::vec3 min_forward{0.0f}, min_up{0.0f}, min_right{0.0f};
		float min = 100000.0;
		float max = -100000.0;
		for(const auto& v : Bounds)
		{
			min = glm::min(min, glm::dot(v, _direction));
			max = glm::max(max, glm::dot(v, _direction));
		}
		min_forward = min * _direction;
		//float bound_forward = max - min;
		
		min = 100000.0;
		max = -100000.0;
		for(const auto& v : Bounds)
		{
			min = glm::min(min, glm::dot(v, up));
			max = glm::max(max, glm::dot(v, up));
		}
		min_up = min * up;
		float bound_up = max - min;
		
		min = 100000.0;
		max = -100000.0;
		for(const auto& v : Bounds)
		{
			min = glm::min(min, glm::dot(v, right));
			max = glm::max(max, glm::dot(v, right));
		}
		min_right = min * right;
		float bound_right = max - min;
		
		// Should be adjusted according to the possible occluders
		float range = 300.f;
		
		_position = min_right + min_up + min_forward + 0.5f * (bound_up * up + bound_right * right) 
			- 0.5f * range * _direction;

		_projection = glm::ortho(-bound_right * 0.5f, bound_right * 0.5f, -bound_up * 0.5f, bound_up * 0.5f, 0.01f, range);
	} else 
		_projection = glm::ortho(-100.0f * 0.5f, 100.0f * 0.5f, -100.0f * 0.5f, 100.0f * 0.5f, 0.01f, 500.0f);
	
	_view = glm::lookAt(_position, _position + _direction, up);
	_VPMatrix = _projection * _view;
	_biasedVPMatrix = s_depthBiasMVP * _VPMatrix;
	
	GPUData tmpStruct = getGPUData();
	_gpuBuffer.data(&tmpStruct, sizeof(GPUData), Buffer::Usage::DynamicDraw);
}