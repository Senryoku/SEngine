#version 430 core

uniform mat4 ModelMatrix = mat4(1.0);
 
in layout(location = 0) vec3 position;
in layout(location = 1) vec3 normal;
in layout(location = 2) vec2 texcoord;

out layout(location = 0) vec4 world_position;

void main()
{
	world_position = ModelMatrix * vec4(position, 1.f);
	gl_Position = world_position;
}
