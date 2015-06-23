#version 430 core

uniform mat4 DepthVP;
 
in layout(location = 0) vec3 position;
in layout(location = 1) vec3 normal;
in layout(location = 2) vec2 texcoord;
in layout(location = 3) mat4 ModelMatrix;

void main()
{
	gl_Position =  DepthVP * ModelMatrix * vec4(position, 1.f);
}
