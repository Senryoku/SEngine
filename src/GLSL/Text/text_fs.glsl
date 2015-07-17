#version 430 core

uniform layout(binding = 0) sampler2D Font;

uniform float	Threshold = 0.75f;
uniform float	OutlineThreshold = 0.25f;
uniform vec3	Color = vec3(1.0);

in vec2 texcoords;
out vec4 color;

void main(void)
{   
	color = vec4(Color, 0.0);
    float sdf = texture(Font, texcoords).a;
	
	color.a = smoothstep(0.0, 1.0, (sdf - OutlineThreshold)/(Threshold - OutlineThreshold));
	
	if(color.a <= 0.0)
		discard;
}

