#version 430 core

uniform mat4 DepthVP;
uniform mat4 ModelMatrix = mat4(1.0);
 
in layout(location = 0) vec3 position;
in layout(location = 1) vec3 normal;
in layout(location = 2) vec2 texcoord;

void main()
{
	gl_Position =  DepthVP * ModelMatrix * vec4(position, 1.f);
}
