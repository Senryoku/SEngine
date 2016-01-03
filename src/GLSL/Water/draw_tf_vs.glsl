#version 430 core

uniform int size_x = 200;
uniform int size_y = 200;
uniform float cell_size;

in  layout(location = 0) vec4 data; // Water Height, Ground Height, Speed(xy)

out layout(location = 0) vec4 out_data;

void main()
{
	out_data = vec4(cell_size * (gl_VertexID / size_x),
					cell_size * (gl_VertexID % size_y), 
					data.x,
					data.y);
}
	