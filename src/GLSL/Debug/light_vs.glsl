#version 430

struct LightStruct
{
	vec4		position;
	vec4		color;
};

layout(std140, binding = 0) uniform LightBlock
{
	LightStruct	Lights[1024];
};

uniform mat4 ModelMatrix = mat4(1.0);

out layout(location = 0) vec3  position;
out layout(location = 1) vec3  color;
out layout(location = 2) float draw;

void main()
{
	draw = Lights[gl_VertexID].color.w;
	if(draw >= 0.0)
	{
		vec4 pos = ModelMatrix * Lights[gl_VertexID].position;
		position = pos.xyz;
		color = Lights[gl_VertexID].color.rgb;
		gl_Position = pos;
	}
}
