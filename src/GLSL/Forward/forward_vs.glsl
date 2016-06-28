#version 430 core

const int CASCADE_COUNT = 3;

layout(std140) uniform Camera
{
	mat4 ViewMatrix;
	mat4 ProjectionMatrix;
};

uniform mat4 ModelMatrix = mat4(1.0);
uniform mat4 LightMatrix[CASCADE_COUNT];

in layout(location = 0) vec3 in_position;
in layout(location = 1) vec3 in_normal;
in layout(location = 2) vec2 in_texcoord;

out layout(location = 0) vec3 world_position;
out layout(location = 1) vec3 world_normal;
out layout(location = 2) vec2 texcoord;
out layout(location = 3) float clipspace_z;
out layout(location = 4) vec3 lightspace_position[CASCADE_COUNT];

void main(void)
{
	vec4 P = ModelMatrix * vec4(in_position, 1.f);
    gl_Position =  ProjectionMatrix * ViewMatrix * P;
	
	world_position = P.xyz / P.w;
	world_normal = mat3(ModelMatrix) * in_normal;
	texcoord = in_texcoord;
	
    for(int i = 0; i < CASCADE_COUNT; i++)
	{
		vec4 t = LightMatrix[i] * P;
        lightspace_position[i] = t.xyz / t.w;
    }
    clipspace_z = gl_Position.z;
}
