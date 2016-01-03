#version 430

layout(points) in;
layout(points) out;
layout(max_vertices = 1) out;

uniform mat4 ModelMatrix = mat4(1.0);

in layout(location = 0) vec4 in_data[];

out layout(location = 0) vec4 water;
out layout(location = 1) vec4 ground;

void main()
{
	vec4 d = in_data[0];
	water = ModelMatrix * vec4(d.x, d.z + d.w, d.y, 1.0);
	ground = ModelMatrix * vec4(d.xwy, 1.0);
	EmitVertex();
	EndPrimitive();
}
