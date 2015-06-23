#version 430 core

layout(location = 0)
uniform mat4 ModelViewMatrix;
layout(location = 1)
uniform mat4 ProjectionMatrix;

in layout(location = 0) vec3 position;
out layout(location = 0) vec3 texcoords;

void main(void)
{
	texcoords = normalize(position);
	gl_Position = ProjectionMatrix * mat4(mat3(ModelViewMatrix)) * vec4(position, 1.f);
}
