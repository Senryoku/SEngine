#version 430 core
#pragma include ../cook_torrance.glsl

layout(std140) uniform Camera {
	mat4 ViewMatrix;
	mat4 ProjectionMatrix;
};

uniform mat4 ModelMatrix = mat4(1.0);

uniform vec3 CameraPosition;

uniform vec3 Color = vec3(1.0);
uniform vec3 Ambiant = vec3(0.04);

uniform float R = 0.4;
uniform float F0 = 0.2;

uniform float BumpScale = 0.1;

uniform layout(binding = 0) sampler2D Texture;
uniform layout(binding = 1) sampler2D NormalMap;
uniform layout(binding = 2) samplerCube EnvMap;

subroutine vec4 color();
subroutine uniform color colorFunction;

subroutine vec3 normal();
subroutine uniform normal normalFunction;

in layout(location = 0) vec3 world_position;
in layout(location = 1) vec3 world_normal;
in layout(location = 2) vec2 texcoord;

out layout(location = 0) vec4 colorOut;

vec2 encode_normal(vec3 n)
{
    vec2 enc = normalize(n.xy) * (sqrt(-n.z * 0.5 + 0.5));
    return enc * 0.5 + 0.5;
}

mat3 tangent_space(vec3 n)
{
	vec3 N = normalize(n);
	vec3 T = vec3(N.y, -N.x, 0.0);
	if(T == vec3(0.0))
		T =  vec3(N.z, -N.x, 0.0);
	T = normalize(T);
	vec3 B = cross(T, N);
	T = cross(N, B);
	return mat3(T, B, N);
}

subroutine(color)
vec4 uniform_color()
{
	return vec4(Color, 1.0);
}

subroutine(color)
vec4 texture_color()
{
	return texture(Texture, texcoord);
}

subroutine(normal)
vec3 basic_normal()
{
	return normalize(world_normal);
}

subroutine(normal)
vec3 normal_mapping()
{
	vec3 map = normalize(texture(NormalMap, texcoord).xyz);
	map = map * 2.0 - 1.0;
    map.z = sqrt(1.0 - dot( map.xy, map.xy ) );
    map.y = -map.y;
	mat3 TBN = tangent_space(normalize(world_normal));
	return normalize(TBN * map);
}

void main(void)
{
	vec4 c = colorFunction();
	if(c.a == 0.0) // Fully transparent, discard the fragment
		discard;
		
	vec3 n = normalFunction();

	if(!gl_FrontFacing) n = -n;
	
	colorOut.rgb = c.rgb * Ambiant +
		cookTorrance(world_position, n, 
			normalize(CameraPosition - world_position),
			c.rgb, 
			vec3(100.0), vec3(1.0), 
			R, F0);
	colorOut.a = 1.0;
}
