#version 430

layout(triangles) in;
layout(triangle_strip, max_vertices = 18) out;

uniform mat4 Projections[6];
uniform vec3 Position;

in layout(location = 0) vec3 world_position[3];

out layout(location = 0) vec4 final_position;

void main(void)
{
	for(gl_Layer = 0; gl_Layer < 6; ++gl_Layer)
	{
		for(int i = 0; i < 3; ++i)
		{
			final_position = vec4(world_position[i], 1.0);
			gl_Position = Projections[gl_Layer] * vec4(world_position[i] - Position, 1.0);
			EmitVertex();
		}
		EndPrimitive();
	}
}
