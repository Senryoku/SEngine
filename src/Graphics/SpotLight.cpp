#include <SpotLight.hpp>

#include <glm/gtc/matrix_transform.hpp> // glm::lookAt, glm::perspective

#include <MathTools.hpp>
#include <ResourcesManager.hpp>

SpotLight::SpotLight(unsigned int shadowMapResolution) :
	DirectionalLight(shadowMapResolution)
{
	updateMatrices();
}

void SpotLight::setRange(float r)
{
	_range = r; 
	updateMatrices();
}

void SpotLight::setAngle(float a)
{
	_angle = a;
	updateMatrices();
}

void SpotLight::updateMatrices()
{
	_projection = glm::perspective(_angle, 1.0f, 0.5f, _range);
	
	glm::vec3 up{0, 1, 0};
	if(glm::cross(_direction, up) == glm::vec3(0.0))
		up = {0, 0, 1};
	_view = glm::lookAt(_position, _position + _direction, up);
	_VPMatrix = _projection * _view;
	_biasedVPMatrix = s_depthBiasMVP * _VPMatrix;
	
	GPUData tmpStruct = getGPUData();
	_gpuBuffer.data(&tmpStruct, sizeof(GPUData), Buffer::Usage::DynamicDraw);
}
