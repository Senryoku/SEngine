#version 430 core

uniform layout(binding = 0) sampler2D Opaque;
uniform layout(binding = 1) sampler2D Transparence0;

in vec2 texcoords;
out vec3 color;

void main(void)
{   
    color = texture(Opaque, texcoords).rgb;
	vec4 transpacence0 = texture(Transparence0, texcoords);
	color = (1.0 - transpacence0.a) * color + transpacence0.a * transpacence0.rgb;
}

