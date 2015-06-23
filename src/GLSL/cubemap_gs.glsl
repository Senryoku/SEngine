#version 430

layout(triangles) in;
layout(triangle_strip, max_vertices = 18) out;

layout(location = 0)
uniform mat4 ModelViewMatrix;
layout(location = 1)
uniform mat4 ProjectionMatrix;

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

struct VertexData
{
	vec3 position;
	vec3 normal;
	vec2 texcoord;
	vec4 shadowcoord;
};

in VertexData VertexOut[3];

out layout(location = 0) VertexData o;

void main(void)
{
	for(gl_Layer = 0; gl_Layer != 6; ++gl_Layer)
	{
		for(int i = 0; i != 3; ++i)
		{
			o.position = vec3(CubeFaceMatrix[gl_Layer] * vec4(VertexOut[i].position, 1.0));
			gl_Position = ProjectionMatrix * vec4(o.position, 1.0);
			gl_PrimitiveID = gl_PrimitiveIDIn;
			o.normal = VertexOut[i].normal;
			o.texcoord = VertexOut[i].texcoord;
			o.shadowcoord = VertexOut[i].shadowcoord;
			EmitVertex();
		}
		EndPrimitive();
	}
}
