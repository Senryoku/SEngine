#pragma once

template<typename T>
Program* 			Light<T>::s_depthProgram = nullptr;
template<typename T>
VertexShader*		Light<T>::s_depthVS = nullptr;
template<typename T>
FragmentShader*	Light<T>::s_depthFS = nullptr;

///////////////////////////////////////////////////////////////////

template<typename T>
Light<T>::Light(unsigned int shadowMapResolution) :
	_shadowMapResolution(shadowMapResolution),
	_shadowMapFramebuffer(_shadowMapResolution)
{
}
