#version 430 core

#define Samples9

layout(std140) uniform Camera {
	mat4 ViewMatrix;
	mat4 ProjectionMatrix;
};

uniform mat4 ModelMatrix = mat4(1.0);

uniform int useNormalMap = 1;

uniform layout(binding = 0) sampler2D Texture;
uniform layout(binding = 1) sampler2D NormalMap;

in layout(location = 0) vec3 world_position;
in layout(location = 1) vec3 world_normal;
in layout(location = 2) vec2 texcoord;

out layout(location = 0) vec4 colorDepthOut;
out layout(location = 1) vec4 worldPositionOut;
out layout(location = 2) vec4 worldNormalOut;

/////////////////////////////////////////////////////////////////////////////////////////////////////
// C/P From http://www.geeks3d.com/20130122/normal-mapping-without-precomputed-tangent-space-vectors/

// http://www.thetenthplanet.de/archives/1180
mat3 cotangent_frame(vec3 N, vec3 p, vec2 uv)
{
    // get edge vectors of the pixel triangle
    vec3 dp1 = dFdx( p );
    vec3 dp2 = dFdy( p );
    vec2 duv1 = dFdx( uv );
    vec2 duv2 = dFdy( uv );
 
    // solve the linear system
    vec3 dp2perp = cross( dp2, N );
    vec3 dp1perp = cross( N, dp1 );
    vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
    vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;
 
    // construct a scale-invariant frame 
    float invmax = inversesqrt( max( dot(T,T), dot(B,B) ) );
    return mat3( T * invmax, B * invmax, N );
}

vec3 perturb_normal( vec3 N, vec3 V, vec2 texcoord )
{
	vec3 map = texture(NormalMap, texcoord).xyz;
	map = map * 2.0 - 1.0;
    map.z = sqrt(1.0 - dot( map.xy, map.xy ) );
    map.y = -map.y;
	mat3 TBN = cotangent_frame(N, -V, texcoord);
	return normalize(TBN * map);
}

void main(void)
{	
	worldNormalOut.rgb = (useNormalMap > 0) ? perturb_normal(normalize(world_normal), world_position, texcoord) : 
						 normalize(world_normal);
	worldNormalOut.a = 1.0;
	
	worldPositionOut.xyz = world_position;
	worldPositionOut.w = gl_FragCoord.z;
	
	colorDepthOut.rgb = texture(Texture, texcoord).rgb;
	colorDepthOut.w = 0.0;
}
