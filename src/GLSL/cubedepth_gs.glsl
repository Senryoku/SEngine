#version 430
#extension GL_EXT_geometry_shader4:enable

layout(triangles) in;
layout(triangle_strip, max_vertices = 18) out;

uniform mat4 Projection;
uniform vec3 Position;

const mat4 CubeFaceMatrix[6] = mat4[6](
	mat4(
		 0.0,  0.0, -1.0,  0.0,
		 0.0, -1.0,  0.0,  0.0,
		-1.0,  0.0,  0.0,  0.0,
		 0.0,  0.0,  0.0,  1.0 
	), mat4(
		 0.0,  0.0,  1.0,  0.0,
		 0.0, -1.0,  0.0,  0.0,
		 1.0,  0.0,  0.0,  0.0,
		 0.0,  0.0,  0.0,  1.0 
	), mat4(
		 1.0,  0.0,  0.0,  0.0,
		 0.0,  0.0, -1.0,  0.0,
		 0.0,  1.0,  0.0,  0.0,
		 0.0,  0.0,  0.0,  1.0 
	), mat4(
		 1.0,  0.0,  0.0,  0.0,
		 0.0,  0.0,  1.0,  0.0,
		 0.0, -1.0,  0.0,  0.0,
		 0.0,  0.0,  0.0,  1.0 
	), mat4(
		 1.0,  0.0,  0.0,  0.0,
		 0.0, -1.0,  0.0,  0.0,
		 0.0,  0.0, -1.0,  0.0,
		 0.0,  0.0,  0.0,  1.0 
	), mat4(
		-1.0,  0.0,  0.0,  0.0,
		 0.0, -1.0,  0.0,  0.0,
		 0.0,  0.0,  1.0,  0.0,
		 0.0,  0.0,  0.0,  1.0 
	)
);


in layout(location = 0) vec3 world_position[3];

out layout(location = 0) vec4 final_position;

void main(void)
{
	for(gl_Layer = 0; gl_Layer < 6; ++gl_Layer)
	{
		for(int i = 0; i < 3; ++i)
		{
			final_position = vec4(world_position[i], 1.0);
			gl_Position = Projection * CubeFaceMatrix[gl_Layer] * vec4(world_position[i] - Position, 1.0);
			EmitVertex();
		}
		EndPrimitive();
	}
}
