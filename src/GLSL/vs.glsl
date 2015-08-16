#version 430 core

layout(std140) uniform Camera
{
	mat4 ViewMatrix;
	mat4 ProjectionMatrix;
};

uniform mat4 ModelMatrix = mat4(1.0);

in layout(location = 0) vec3 in_position;

void main(void)
{
	gl_Position =  ProjectionMatrix * ViewMatrix * ModelMatrix * vec4(in_position, 1.f);
}
