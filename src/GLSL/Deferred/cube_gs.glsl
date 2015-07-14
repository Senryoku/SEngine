#version 430
#extension GL_EXT_geometry_shader4:enable
//layout(triangles, invocations = 6) in;
layout(triangles) in;
layout(triangle_strip, max_vertices = 18) out;

layout(std140) uniform Camera
{
	mat4 ViewMatrix;
	mat4 ProjectionMatrix;
};

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


in layout(location = 0) vec3 in_position[3];
in layout(location = 1) vec3 in_normal[3];
in layout(location = 2) vec2 in_texcoord[3];

out layout(location = 0) vec3 world_position;
out layout(location = 1) vec3 world_normal;
out layout(location = 2) vec2 texcoord;

void main(void)
{
	//gl_Layer = gl_InvocationID;
	for(gl_Layer = 0; gl_Layer < 6; ++gl_Layer)
	{
		for(int i = 0; i < 3; ++i)
		{
			world_position = in_position[i];
			gl_Position = ProjectionMatrix * CubeFaceMatrix[gl_Layer] * ViewMatrix * vec4(in_position[i], 1.0);
			gl_PrimitiveID = gl_PrimitiveIDIn;
			world_normal = in_normal[i];
			texcoord = in_texcoord[i];
			EmitVertex();
		}
		EndPrimitive();
	}
}
