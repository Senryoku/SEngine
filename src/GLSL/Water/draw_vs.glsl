#version 430 core

uniform int size_x = 200;
uniform int size_y = 200;
uniform float cell_size;

layout(std140) uniform Camera
{
	mat4 ViewMatrix;
	mat4 ProjectionMatrix;
};

struct WaterCell
{
	vec4 data; // Water Height, Ground Height, Speed(xy)
};

layout(std140, binding = 4) buffer InBuffer
{
	WaterCell	Ins[];
};

out layout(location = 0) vec4 out_position;
out layout(location = 1) int id;

void main()
{
	out_position = vec4(cell_size * (gl_VertexID / size_x), 
						Ins[gl_VertexID].data.x + Ins[gl_VertexID].data.y, 
						cell_size * (gl_VertexID % size_y), 
						1.0);
	id = gl_VertexID;
}
	