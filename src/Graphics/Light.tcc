#pragma once

template<typename T>
Program* 			Light<T>::s_depthProgram = nullptr;

///////////////////////////////////////////////////////////////////

template<typename T>
Light<T>::Light(unsigned int shadowMapResolution) :
	_shadowMapResolution(shadowMapResolution),
	_shadowMapFramebuffer(_shadowMapResolution)
{
}
