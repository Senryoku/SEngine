#version 430 core
#pragma include ../cook_torrance.glsl

const int CASCADE_COUNT = 3;

layout(std140) uniform Camera {
	mat4 ViewMatrix;
	mat4 ProjectionMatrix;
};

uniform mat4 ModelMatrix = mat4(1.0);

uniform vec3 CameraPosition;

uniform vec3 Color = vec3(1.0);
uniform vec3 Ambiant = vec3(0.1);

uniform float R = 0.95;
uniform float F0 = 0.2;

uniform float BumpScale = 0.1;

// Shadow parameter (VSM)
uniform float	MinVariance = 0.0000001;
uniform float	ShadowClamp = 0.8;

uniform layout(binding = 0) sampler2D Texture;
uniform layout(binding = 1) sampler2D NormalMap;
uniform layout(binding = 2) samplerCube EnvMap;

uniform layout(binding = 3) sampler2D ShadowMaps[CASCADE_COUNT];
uniform float CascadeFar[CASCADE_COUNT];

subroutine vec4 color();
subroutine uniform color colorFunction;

subroutine vec3 normal();
subroutine uniform normal normalFunction;

in layout(location = 0) vec3 world_position;
in layout(location = 1) vec3 world_normal;
in layout(location = 2) vec2 texcoord;
in layout(location = 3) float clipspace_z;
in layout(location = 4) vec3 lightspace_position[CASCADE_COUNT];

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

float VSM(float dist, vec2 moments)
{
	float d = dist - moments.x;
	if(d > 0.0)
	{
		float variance = moments.y - (moments.x * moments.x);
		variance = max(variance, MinVariance);
		return smoothstep(ShadowClamp, 1.0, variance / (variance + d * d));
	}
	return 1.0;
}

void main(void)
{
	vec4 c = colorFunction();
	if(c.a == 0.0) // Fully transparent, discard the fragment
		discard;
		
	vec3 n = normalFunction();

	if(!gl_FrontFacing) n = -n;
	
	colorOut.rgb = vec3(0);
	float shadow_factor = 1.0f;
    for(int i = 0; i < CASCADE_COUNT; i++)
	{
        if (clipspace_z <= CascadeFar[i])
		{
			vec2 moments = texture2D(ShadowMaps[i], lightspace_position[i].xy).xy;
			shadow_factor = VSM(lightspace_position[i].z, moments);
			// Cascades debug
			colorOut.rgb += 0.1 * vec3((i == 0 ? 1.0 : 0.0), 
									(i == 1 ? 1.0 : 0.0), 
									(i == 2 ? 1.0 : 0.0));
			break;
		}
	}
	
	colorOut.rgb += c.rgb * Ambiant +
		shadow_factor * cookTorrance(
			world_position, 
			n, 
			normalize(CameraPosition - world_position),
			c.rgb, 
			world_position + vec3(1.0),
			vec3(1.0), 
			R, 
			F0
		);
	colorOut.a = 1.0;
}
