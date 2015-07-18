#version 430
#pragma include cook_torrance.glsl

#define SHADOWBLOCKCOUNT			10
#define CUBESHADOWBLOCKCOUNT		3
// 2 + SHADOWBLOCKCOUNT
#define CUBESHADOWBLOCKOFFSET	12


/*************
 * How data is laid down :
 * ColorMaterial.xyz	=> Color
 * ColorMaterial.w	=> if > 0 : Non transparent
 * PositionDepth.xyz	=> World Position
 * PositionDepth.w	=> Depth
 * Normal.xy			=> Compressed Normal
 * Normal.z			=> Fresnel Reflectance (F0)
 * Normal.w			=> Roughness (R)
**************/

struct LightStruct
{
	vec4		position;
	vec4		color;
};

layout(std140, binding = 1) uniform LightBlock
{
	LightStruct	Lights[1024];
};

layout(std140, binding = 2) uniform ShadowBlock
{
	vec4		position_range;
	vec4		color;
	mat4 		depthMVP;
} Shadows[SHADOWBLOCKCOUNT];

layout(std140, binding = CUBESHADOWBLOCKOFFSET) uniform CubeShadowBlock
{
	vec4		position_range;
	vec4		color;
} CubeShadows[CUBESHADOWBLOCKCOUNT];

uniform unsigned int LightCount = 75;

uniform unsigned int ShadowCount = 0;
uniform unsigned int CubeShadowCount = 0;
uniform float	MinVariance = 0.0000001;
uniform float	ShadowClamp = 0.8;

uniform float	Bloom = 1.0;

uniform float	Gamma = 2.2;
uniform float	Exposure = 5.0;
uniform vec3	Ambiant = vec3(0.06);

uniform int	AOSamples = 8;
uniform float	AOThreshold = 1.0f;
uniform float	AORadius = 10.0f;

uniform vec3	CameraPosition;

layout(binding = 0, rgba32f) uniform image2D ColorMaterial;
layout(binding = 1, rgba32f) uniform readonly image2D PositionDepth;
layout(binding = 2, rgba32f) uniform image2D Normal;

layout(binding = 3) uniform sampler2D ShadowMaps[SHADOWBLOCKCOUNT];
layout(binding = CUBESHADOWBLOCKOFFSET) uniform samplerCube CubeShadowMaps[CUBESHADOWBLOCKCOUNT];

// Bounding Box
shared int bbmin_x;
shared int bbmin_y;
shared int bbmin_z;
shared int bbmax_x;
shared int bbmax_y;
shared int bbmax_z;

// Lights
shared int local_lights_count; // = 0;
shared int local_lights[1024];
shared int lit;

void add_light(int l)
{
	int idx = atomicAdd(local_lights_count, 1);
	local_lights[idx] = l;
}

float square(float f)
{
	return f * f;
}

bool sphereAABBIntersect(vec3 min, vec3 max, vec3 center, float radius)
{
    float r = radius * radius;
    if(center.x < min.x) r -= square(center.x - min.x);
    else if(center.x > max.x) r -= square(center.x - max.x);
    if(center.y < min.y) r -= square(center.y - min.y);
    else if(center.y > max.y) r -= square(center.y - max.y);
    if(center.z < min.z) r -= square(center.z - min.z);
    else if(center.z > max.z) r -= square(center.z - max.z);
    return r > 0;
}

vec3 decode_normal(vec2 enc)
{
    vec4 nn = vec4(enc, 0, 0) * vec4(2, 2, 0, 0) + vec4(-1, -1, 1, -1);
    float l = dot(nn.xyz,-nn.xyw);
    nn.z = l;
    nn.xy *= sqrt(l);
    return nn.xyz * 2 + vec3(0,0,-1);
}

#pragma include ../poisson_samples.glsl
float ambiantOcclusion(vec3 p, vec3 n, uvec2 pix, float depth)
{
	float ao = 0.0;
	
    for (int i = 0; i < AOSamples; ++i)
    {
		// Get Sample
        ivec2 samplePixel = ivec2(pix + (poisson16[i] * (AORadius / depth)));
        vec3 samplePos = imageLoad(PositionDepth, samplePixel).xyz;
        vec3 sampleDir = normalize(samplePos - p);

		// Compute relevant values
        float NdotS = max(dot(n, sampleDir), 0);
        float VPdistSP = distance(p, samplePos);

        float a = 1.0 - smoothstep(AOThreshold, AOThreshold * 2, VPdistSP);
        float b = NdotS;

        ao += (a * b);
    }
	return 1.0 - ao / max(AOSamples, 1);
}

vec3 exposureToneMapping(vec3 c, float e)
{
	return vec3(1.0) - exp(-c * e);
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

const int highValue = 2147483646;
const float boxfactor = 10000.0f; // Minimize the impact of the use of int for bounding boxes

layout (local_size_x = 16, local_size_y = 16) in;
void main(void)
{
	uvec2 pixel = gl_GlobalInvocationID.xy;
	uvec2 local_pixel = gl_LocalInvocationID.xy;
	ivec2 image_size = imageSize(ColorMaterial).xy;
	
	bool isVisible = pixel.x >= 0 && pixel.y >= 0 && pixel.x < image_size.x && pixel.y < image_size.y;
	vec4 colmat;
	vec4 position;
	
	if(local_pixel == uvec2(0, 0))
	{
		local_lights_count = 0;
		
		bbmin_x = highValue;
		bbmin_y = highValue;
		bbmin_z = highValue;
		bbmax_x = -highValue;
		bbmax_y = -highValue;
		bbmax_z = -highValue;
		lit = 0;
	}
	barrier();
		
	// Compute Bounding Box
	if(isVisible)
	{
		position = imageLoad(PositionDepth, ivec2(pixel));
		isVisible = isVisible && position.w >= 0.0 && position.w <= 1.0;
		if(isVisible)
		{
			colmat = imageLoad(ColorMaterial, ivec2(pixel));
			
			if(colmat.w > 0.0)
			{
				ivec3 scaledp = ivec3(boxfactor * position + 1.0);
				ivec3 scaledm = ivec3(boxfactor * position - 1.0);
				
				atomicMin(bbmin_x, scaledm.x);
				atomicMin(bbmin_y, scaledm.y);
				atomicMin(bbmin_z, scaledm.z);
				atomicMax(bbmax_x, scaledp.x);
				atomicMax(bbmax_y, scaledp.y);
				atomicMax(bbmax_z, scaledp.z);
				
				lit = 1;
			}
		}
	}
	barrier();
	
	// Construct boundingbox
	if(lit > 0)
	{
		// FIXME: Problem here!! Wrong min/max, or sphereAABBIntersect bug!
		vec3 min_bbox = (vec3(bbmin_x, bbmin_y, bbmin_z) - 1.0) / boxfactor;
		vec3 max_bbox = (vec3(bbmax_x, bbmax_y, bbmax_z) + 1.0) / boxfactor;

		// Test lights
		for(uint i = gl_LocalInvocationIndex; i < LightCount; i += gl_WorkGroupSize.x * gl_WorkGroupSize.y)
		{
			if(sphereAABBIntersect(min_bbox, max_bbox, Lights[i].position.xyz, Lights[i].position.w))
				add_light(int(i));
		}
	}
	
	barrier();
	
	//Compute lights' contributions
	
	if(isVisible)
	{
		vec3 color = colmat.xyz;
		vec4 ColorOut = vec4(Ambiant * color, 1.0);
		if(colmat.w <= 0.0)
		{
			ColorOut = vec4(color, 1.0);
		} else if(lit > 0) {
			vec4 data = imageLoad(Normal, ivec2(pixel));
			vec3 normal = normalize(decode_normal(data.xy));
		
			vec3 V = normalize(CameraPosition - position.xyz);
			float depth = length(CameraPosition - position.xyz);
			
			ColorOut *= ambiantOcclusion(position.xyz, normal, pixel, depth);
			
			// Simple Point Lights
			for(int l2 = 0; l2 < local_lights_count; ++l2)
			{
				float sqRad = Lights[local_lights[l2]].position.w;
				sqRad *= sqRad;
				float d = dot(position.xyz - Lights[local_lights[l2]].position.xyz,
							position.xyz - Lights[local_lights[l2]].position.xyz);
				if(d < sqRad)
					ColorOut.rgb += (1.0 - d/sqRad) * (1.0 - d/sqRad) *
						cookTorrance(position.xyz, normal, V, color,
							Lights[local_lights[l2]].position.xyz, Lights[local_lights[l2]].color.rgb,
							data.w, data.z);
			}
			
			// Shadow casting Spot and Orthographic Lights
			for(int shadow = 0; shadow < ShadowCount; ++shadow)
			{
				vec4 sc = Shadows[shadow].depthMVP * vec4(position.xyz, 1.0);
				sc /= sc.w;
				bool spotlight = Shadows[shadow].color.w != 0.0;
				float r = (!spotlight) ? 0.0 :
								(sc.x - 0.5) * (sc.x - 0.5) + (sc.y - 0.5) * (sc.y - 0.5); // Spot Light
				if((sc.x >= 0 && sc.x <= 1.f) &&
					(sc.y >= 0 && sc.y <= 1.f) && 
					r < 0.25)
				{
					vec2 moments = texture2D(ShadowMaps[shadow], sc.xy).xy;
					float visibility = VSM(sc.z, moments);
					
					float att = (!spotlight) ? 1.0 :
							max(0.0, (1.0 - square(length(Shadows[shadow].position_range.xyz - position.xyz)/Shadows[shadow].position_range.w)));
					vec3 light_pos = (!spotlight) ? position.xyz - 10.0 * Shadows[shadow].position_range.xyz:
											Shadows[shadow].position_range.xyz;
					ColorOut.rgb += att * att * visibility * cookTorrance(position.xyz, normal, V, color, 
										light_pos, Shadows[shadow].color.rgb, 
										data.w, data.z);
				}
			}
			
			// Shadow casting Omnidirectional lights
			for(int shadow = 0; shadow < CubeShadowCount; ++shadow)
			{
				float dist = distance(position.xyz, CubeShadows[shadow].position_range.xyz);
				if(dist < CubeShadows[shadow].position_range.w)
				{
					vec3 direction = normalize(position.xyz - CubeShadows[shadow].position_range.xyz);
					
					vec2 moments = texture(CubeShadowMaps[shadow], direction).xy;
					float visibility = VSM(dist, moments);
					
					float att = max(0.0, (1.0 - square(dist/CubeShadows[shadow].position_range.w)));
					ColorOut.rgb += att * att * visibility * cookTorrance(position.xyz, normal, V, color, 
										CubeShadows[shadow].position_range.xyz, CubeShadows[shadow].color.rgb, 
										data.w, data.z);
				}
			}

		}

		// Delay Tone Mapping and Gamma Correction if using Bloom
		if(Bloom > 0.0) 
		{
			// Storing thresholded color for Bloom
			imageStore(Normal, ivec2(pixel), (dot(ColorOut.rgb, vec3(0.2126, 0.7152, 0.0722)) > Bloom) ? ColorOut : vec4(0.0));
		} else if(Exposure > 0.0) { 
			// Tone Mapping
			ColorOut.rgb = exposureToneMapping(ColorOut.rgb, Exposure);

			// Gamma correction (Deactivated for now)
			//ColorOut.rgb = pow(ColorOut.rgb, vec3(1.0 / Gamma));
		}

		imageStore(ColorMaterial, ivec2(pixel), ColorOut);
	}
}
