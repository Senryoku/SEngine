#version 430 core

#define Samples9

layout(std140) uniform Camera {
	mat4 ViewMatrix;
	mat4 ProjectionMatrix;
};

uniform mat4 ModelMatrix = mat4(1.0);

uniform int useNormalMap = 1;
uniform float R = 0.4;
uniform float F0 = 0.1;

uniform layout(binding = 0) sampler2D Texture0;
uniform layout(binding = 1) sampler2D Texture1;
uniform layout(binding = 2) sampler2D NormalMap0;
uniform layout(binding = 3) sampler2D NormalMap1;

in layout(location = 0) vec3 world_position;
in layout(location = 1) vec3 world_normal;
in layout(location = 2) vec2 texcoord;

out layout(location = 0) vec4 colorMatOut;
out layout(location = 1) vec4 worldPositionOut;
out layout(location = 2) vec4 worldNormalOut;

vec2 encode_normal(vec3 n)
{
    vec2 enc = normalize(n.xy) * (sqrt(-n.z * 0.5 + 0.5));
    return enc * 0.5 + 0.5;
}

float mix_tex(float t, float p)
{
	// Add some perturbation
	t += dFdx(worldPositionOut.x) * dFdx(worldPositionOut.y);
	
	if(t < p - 0.5)
		return 0.0;
	else if(t > p + 0.5)
		return 1.0;
	return (t - (p - 0.5));
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

vec3 perturb_normal(vec3 n, vec2 texcoord)
{
	float t = mix_tex(world_position.y, 2.0);
	
	vec3 map = normalize(mix(texture(NormalMap0 , texcoord).xyz,
						texture(NormalMap1 , texcoord).xyz, t));
	map = map * 2.0 - 1.0;
    map.z = sqrt(1.0 - dot( map.xy, map.xy ) );
    map.y = -map.y;
	mat3 TBN = tangent_space(n);
	return normalize(TBN * map);
}

void main(void)
{			
	vec3 n = (useNormalMap > 0) ? 
				perturb_normal(normalize(world_normal), texcoord) : 
				normalize(world_normal);
	
	worldNormalOut.xy = encode_normal(n);
	
	worldNormalOut.z = F0;
	worldNormalOut.w = R;
	
	worldPositionOut.xyz = world_position;
	worldPositionOut.w = gl_FragCoord.z;
	
	float t = mix_tex(world_position.y, 2.0);
	colorMatOut.rgb = mix(texture(Texture0, texcoord).rgb, texture(Texture1, texcoord).rgb, t);
	colorMatOut.w = 1.0;
}
