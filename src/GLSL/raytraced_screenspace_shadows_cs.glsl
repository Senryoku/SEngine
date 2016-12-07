#version 430

#define WORKGROUP_SIZE 16

uniform vec3	LightDirection = normalize(vec3(1.0));

layout(binding = 0, rgba32f) uniform readonly image2D PositionDepth;

layout(binding = 1, rgba32f) uniform image2D Out;

layout (local_size_x = WORKGROUP_SIZE, local_size_y = WORKGROUP_SIZE) in;

float shadow(vec2 pixel)
{
	vec3 position = texture2D(PositionDepth, pixel).xyz;
	
	vec3 d = normalize(LightDirection - position.xyz);
	vec3 p = position;
	for(int i = 0; i < VolumeSamples; ++i)
	{
		p += d;
		vec2 screen_p = ...
		float depth = texture2D(PositionDepth, screen_p.xy).w;
		if(screen_p.z > depth)
			return 1.0;
	}
}

void main(void)
{
}
