#version 440

uniform sampler3D	Data0;
uniform vec3		Resolution;
uniform vec3		CameraPosition;

uniform float Time = 0.0;

// Configuration
uniform float Tex3DRes = 32.0;
uniform int DisplayedLoD = 0;
uniform int Shadows = 1; 
uniform float AOStrength = 0.5; 
uniform float AOThreshold = 0.5; 

uniform float Ambient = 0.2;

int Steps; // Max. ray steps before bailing out
float maxLoD = log2(32) - 1;

///////////////////////////////////////////////////////////////////////////
// Object

float object(vec3 p, float bias)
{
	return textureLod(Data0, p / Tex3DRes + vec3(0.5), DisplayedLoD).x;
}
/*
float object(vec3 p)
{
	return texture(Data0, p / Tex3DRes + vec3(0.5)).x;
}
*/
///////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// Tracing

bool traceBox(vec3 ro, vec3 rd, vec3 lb, vec3 rt, out float t)
{
	if(all(lessThan(lb, ro)) && all(lessThan(ro, rt))) // Inside the box
	{
		t = 0;
		return true;
	}
	
	vec3 dirfrac;
	dirfrac.x = 1.0f / rd.x;
	dirfrac.y = 1.0f / rd.y;
	dirfrac.z = 1.0f / rd.z;
	// lb is the corner of AABB with minimal coordinates - left bottom, rt is maximal corner
	// r.org is origin of ray
	float t1 = (lb.x - ro.x) * dirfrac.x;
	float t2 = (rt.x - ro.x) * dirfrac.x;
	float t3 = (lb.y - ro.y) * dirfrac.y;
	float t4 = (rt.y - ro.y) * dirfrac.y;
	float t5 = (lb.z - ro.z) * dirfrac.z;
	float t6 = (rt.z - ro.z) * dirfrac.z;

	float tmin = max(max(min(t1, t2), min(t3, t4)), min(t5, t6));
	float tmax = min(min(max(t1, t2), max(t3, t4)), max(t5, t6));

	// if tmax < 0, ray (line) is intersecting AABB, but whole AABB is behing us
	if (tmax < 0)
	{
		t = tmax;
		return false;
	}

	// if tmin > tmax, ray doesn't intersect AABB
	if (tmin > tmax)
	{
		t = tmax;
		return false;
	}

	t = tmin;
	return true;
}

bool traceLoD(vec3 origin, vec3 direction, int LoD, out vec3 p, out vec3 n, out int i)
{
	#define BRANCHLESS 1
	p = origin;
	float step = 0.0;
	float depth = 0.0;
	ivec3 dirSign = ivec3(sign(direction));
	
	ivec3 rayPos = ivec3(p);
    vec3 absDir = abs(direction);
	#if BRANCHLESS
	vec3 deltaDist = abs(vec3(length(direction)) / direction);
	vec3 d = (sign(direction) * (vec3(rayPos) - origin) + (sign(direction) * 0.5) + 0.5) * deltaDist;
	bvec3 mask;
	#else
    vec3 projLength = 1.0 / (absDir + 0.0001);
    vec3 d = p - rayPos;
    if(dirSign.x == 1) d.x = 1 - d.x;
    if(dirSign.y == 1) d.y = 1 - d.y;
    if(dirSign.z == 1) d.z = 1 - d.z;
    d *= projLength;
	#endif
	
	float v = texelFetch(Data0, ivec3(rayPos), LoD).x;
	for(i = 0; i < Steps; i++)
	{
		if (v > 0.0)
		{
			p = rayPos + 0.5;
			
			#if BRANCHLESS
			n = vec3(mask) * (-dirSign);
			#else
			d = abs(d - projLength);
			if(d.x < 0.0001) n = -dirSign.x * vec3(1, 0, 0);
			if(d.y < 0.0001) n = -dirSign.y * vec3(0, 1, 0);
			if(d.z < 0.0001) n = -dirSign.z * vec3(0, 0, 1);
			n = normalize(n);
			#endif
			
			return true;
		} else {
			#if BRANCHLESS
			bvec3 b1 = lessThan(d.xyz, d.yzx);
			bvec3 b2 = lessThanEqual(d.xyz, d.zxy);
			mask = b1 && b2;
			d += vec3(mask) * deltaDist;
			rayPos += ivec3(mask) * dirSign;
			#else
            if(d.x < d.y || d.z < d.y)
			{
                if(d.x < d.z)
				{
                    rayPos.x += dirSign.x;
					d.yz -= d.x;
                    d.x = projLength.x;
                } else {
                    rayPos.z += dirSign.z;
					d.xy -= d.z;
                    d.z = projLength.z;
                }
            } else {
                rayPos.y += dirSign.y;
				d.xz -= d.y;
                d.y = projLength.y;
            }
			#endif
		}
		
		if(any(greaterThanEqual(rayPos, ivec3(Tex3DRes / pow(2, LoD)))) || 
			any(lessThan(rayPos, ivec3(0, 0, 0))))
            return false;
			
		v = texelFetch(Data0, ivec3(rayPos), LoD).x;
	}

	return false;
}

//#define NoLOD

bool trace(vec3 a, vec3 u, out vec3 p, out vec3 n, out int i)
{
	a = a + Tex3DRes / 2;
#ifdef NoLOD
	return traceLoD(a / pow(2, DisplayedLoD), u, DisplayedLoD, p, n, i);
#else
	vec3 ap;
	float t;
	for(int LoD = int(maxLoD - 1); LoD >= DisplayedLoD;)
	{
		if(traceLoD(a / pow(2, LoD), u, LoD, ap, n, i))
		{
			if(LoD == DisplayedLoD)
			{
				p = ap;
				return true;
			}
			
			float voxel_size = pow(2, LoD);
			float res = Tex3DRes / voxel_size;
			ap = voxel_size * ap;
			
			--LoD;
			traceBox(a, u, ap - vec3(0.51 * voxel_size), ap + vec3(0.51 * voxel_size), t);
			a = a + t * u;
		} else return false;
	}

	return false;
#endif
}

//////////////////////////////////////////////////////////////////////////

float sqr(float f)
{
	return f * f;
}

float diag(float f)
{
	return sqrt(2) * abs(f);
}

vec3 LightDirection = normalize(vec3(0.0, -1.0, 1.0));

out vec4 outColor;
uniform vec3 Forward;
void main(void)
{
	maxLoD = log2(Tex3DRes) - 1;
	Steps = 250; // Max. ray steps before bailing out
	
	LightDirection = normalize(-vec3(cos(.3 * Time), 1.0, sin(.3 * Time)));

	vec2 pixel = (gl_FragCoord.xy / Resolution.xy) * 2.0 - 1.0;
	float asp = Resolution.x / Resolution.y;
	
	vec3 rd = normalize(vec3(asp * pixel.x, pixel.y, 2.0));
	vec3 ro = CameraPosition;

	const vec3 up = vec3(0.0, -1.0, 0.0);
	const vec3 right = normalize(cross(up, Forward));
	const mat4 viewMatrix = mat4(vec4(right, 0), vec4(cross(right, Forward), 0), vec4(Forward, 0), vec4(vec3(0.0), 1));
	
	rd = vec3(viewMatrix * vec4(rd, 1));
	
	//////////////////////////////////////////////
	
	const int box_count = 9;
	vec3 box_positions[box_count] = {vec3(0.0), vec3(-128.0, 0.0, 0.0), vec3(128.0, 0.0, 0.0),
									vec3(0.0, 0.0, 128.0), vec3(-128.0, 0.0, 128.0), vec3(128.0, 0.0, 128.0),
									vec3(0.0, 0.0, -128.0), vec3(-128.0, 0.0, -128.0), vec3(128.0, 0.0, -128.0)};
	vec3 box_colors[box_count] = {vec3(0.0, 1.0, 0.0), vec3(1.0, 0.0, 0.0), vec3(0.0, 0.0, 1.0),
								vec3(1.0, 1.0, 0.0), vec3(1.0, 0.0, 1.0), vec3(1.0, 0.0, 1.0),
								vec3(0.0, 1.0, 1.0), vec3(1.0, 1.0, 1.0), vec3(0.0, 1.0, 1.0)};
	
	vec3 rgb = vec3(0.0, 0.0, 0.5 * max(0.5, pixel.y + 0.5) + 0.1);
	float t = 100000000.0;
	int s = 0;
	bool hit = false;
	bool hits[box_count] = {false, false, false, false, false, false, false, false, false};
	float ts[box_count] = {100000000, 100000000, 100000000, 100000000, 100000000, 100000000, 100000000, 100000000, 100000000};
	vec3 pos = vec3(0.0);
	vec3 n = vec3(0.0);
	
	float ttmp = 0;
	for(int i = 0; i < box_count; ++i)
	{
		hits[i] = traceBox(ro, rd, box_positions[i] + vec3(-0.5 * Tex3DRes), box_positions[i] + vec3(0.5 * Tex3DRes), ts[i]);
	}
	
	int current_hit = 0;
	
	for(int i = 0; i < box_count; ++i)
	{
		if((!hit || ts[i] < ts[current_hit]) && hits[i])
		{
			//rgb *= 0.5f; // Debug (View Chunks)
			
			if(trace(ro + ts[i] * rd - box_positions[i], rd, pos, n, s))
			{
				hit = true;
				rgb = box_colors[i];
				current_hit = i;
			}
		}
	}

	if(hit)
	{
		// Get precise position
		float voxel_size = pow(2, DisplayedLoD);
		float res = Tex3DRes / voxel_size;
		float t;
		vec3 voxel_world_pos = voxel_size * pos - vec3(Tex3DRes / 2) + box_positions[current_hit];
		if(traceBox(ro, rd, voxel_world_pos - vec3(0.51 * voxel_size), 
							voxel_world_pos + vec3(0.51 * voxel_size), t))
		{
			pos = ro + t * rd;
			
			// Ambient Occlusion (Doesn't work for DisplayedLoD != 0 ?)
			rgb *= 1.0 - AOStrength * max(0.0, (textureLod(Data0, (pos + 0.01 * n - box_positions[current_hit])/res + 0.5, DisplayedLoD).x - AOThreshold));
			
			// Shadow casting
			if(Shadows > 0)
			{
				vec3 dummy;
				bool shadow_hit = false;
				for(int i = 0; i < box_count; ++i)
				{
					if(traceBox(pos, -LightDirection, box_positions[i] + vec3(-0.5 * res), box_positions[i] + vec3(0.5 * res), t))
						if(trace(pos - t * LightDirection - box_positions[i] - 0.1 * LightDirection, -LightDirection, dummy, dummy, s))
						{
							shadow_hit = true;
							break;
						}
				}
				if(!shadow_hit)
					rgb *= max(Ambient, dot(n, -LightDirection));
				else
					rgb *= Ambient;
			} else {
				rgb *= max(Ambient, dot(n, -LightDirection));
			}
		}
	}
	
	outColor = vec4(rgb, 1.0);
}
