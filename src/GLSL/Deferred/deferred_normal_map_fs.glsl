#version 430 core

#define Samples9

layout(std140) uniform Camera {
	mat4 ViewMatrix;
	mat4 ProjectionMatrix;
};

uniform mat4 ModelMatrix = mat4(1.0);

uniform int useNormalMap = 1;
uniform int useBumpMap = 0;
uniform float R = 0.4;
uniform float F0 = 0.1;
uniform float k = 0.5;

uniform float BumpScale = 0.1;

uniform layout(binding = 0) sampler2D Texture;
uniform layout(binding = 1) sampler2D NormalMap;
uniform layout(binding = 2) sampler2D BumpMap;

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

vec3 perturb_normal(vec3 N, vec3 V, vec2 texcoord)
{
	vec3 map = normalize(texture(NormalMap, texcoord).xyz);
	map = map * 2.0 - 1.0;
    map.z = sqrt(1.0 - dot( map.xy, map.xy ) );
    map.y = -map.y;
	mat3 TBN = cotangent_frame(N, -V, texcoord);
	return normalize(TBN * map);
}

vec2 parallax(vec2 texCoords, vec3 viewDir)
{ 
    float height = texture(BumpMap, texCoords).r;
	if(useBumpMap == 0 || height == 0.0)
		return texCoords;
    vec2 p = viewDir.xy / (viewDir.z * (height * BumpScale));
    return texCoords - p;    
} 

void main(void)
{
	/*
	// Parallax Mapping, not tested.
	vec3 cameraPosition = -vec3(ViewMatrix[3]) * mat3(ViewMatrix);
	TBN = cotangent_frame(normalize(world_normal), -world_position, texcoord);
	
	vec2 tc = parallax(texcoord, TBN * (cameraPosition - world_position));
	*/
	
	vec4 c = texture(Texture, texcoord);
	if(c.a == 0.0) // Fully transparent, discard the fragment
		discard;
		
	vec3 n = (useNormalMap > 0) ? perturb_normal(normalize(world_normal), world_position, texcoord) : 
						 normalize(world_normal);

	worldNormalOut.xy = encode_normal(n);
	worldNormalOut.z = F0;
	worldNormalOut.w = k;
	
	worldPositionOut.xyz = world_position;
	worldPositionOut.w = gl_FragCoord.z;
	
	colorMatOut.rgb = c.rgb;
	colorMatOut.w = R;
}
