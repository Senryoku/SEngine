#version 430

in vec2 texcoords;
out vec4 color;

layout(binding = 0) uniform sampler2D ColorFrame;
layout(binding = 1) uniform sampler2D BloomFrame;

uniform float Gamma = 2.2;
uniform float Exposure = 3.0;

// Reinhard Tone Mapping
vec3 reinhard(vec3 c)
{
	return c / (c + vec3(1.0));
}

vec3 exposureToneMapping(vec3 c, float e)
{
	return vec3(1.0) - exp(-c * e);
}

void main()
{
	color = texture(ColorFrame, texcoords) + texture(BloomFrame, texcoords);
	
	color.rgb = exposureToneMapping(color.rgb, Exposure);
	
	// Gamma correction (Deactivated for now)
	//color.rgb = pow(color.rgb, vec3(1.0 / Gamma));
}
