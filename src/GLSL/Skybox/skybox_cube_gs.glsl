#version 430

layout(triangles) in;
layout(triangle_strip, max_vertices = 18) out;

#pragma include ../cube_matrices.glsl

in vec2 texcoords[3];

out vec2 texcoord;
out mat4 cubeMatrix;

void main(void)
{
	for(gl_Layer = 0; gl_Layer < 6; ++gl_Layer)
	{
		for(int i = 0; i < 3; ++i)
		{
			gl_Position = gl_in[i].gl_Position;
			gl_PrimitiveID = gl_PrimitiveIDIn;
			texcoord = texcoords[i];
			cubeMatrix = CubeFaceMatrix[gl_Layer];
			EmitVertex();
		}
		EndPrimitive();
	}
}
