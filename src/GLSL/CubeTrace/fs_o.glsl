#version 440

uniform sampler3D	Data0;
uniform vec3		Resolution;
uniform vec3		CameraPosition;

// Configuration
uniform float Tex3DRes = 32.0;
int Steps; // Max. ray steps before bailing out
float Epsilon; // Marching epsilon

uniform float maxLoD = log2(32);
uniform float displayedLoD = 0.0;
///////////////////////////////////////////////////////////////////////////
// Object

float object(vec3 p, float bias)
{
	return textureLod(Data0, p / Tex3DRes + vec3(0.5), displayedLoD).x;
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

// DOESN'T WORK!
// Cubic cells are not as good as sphere for tracing... I'm so dumb -.-'
vec3 lodTrace(vec3 a, vec3 u, out bool hit, out int i)
{
	hit = false;
	vec3 p = a;
	float step = 0.0;
	float LoD = maxLoD;
	float depth = 0.0;
	for(i = 0; i < Steps; i++)
	{
		float v = object(p, LoD);
		if (v > 0.0)
		{
			if(LoD < displayedLoD + 0.5)
			{
				vec3 p2 = p;
				p -= (1.0 + LoD) * Epsilon * u;
				for(int j = 0; j < 10; ++j)
				{
					vec3 tmp = p;
					p = (p + p2) / 2;
					if(object(p, displayedLoD) > 0.0)
					{
						p2 = p;
						p = tmp;
					}
				}
				hit = true; 
				return p;
			}
			LoD = clamp(LoD - 1.0, 0.0, maxLoD);
		} else {
			float e = (LoD < displayedLoD + 1.5) ? Epsilon : Epsilon;
			depth += max(1.0, LoD) * e;
			if(depth >= Tex3DRes * sqrt(2.0))
				return p;
			p += max(1.0, LoD) * e * u;
			LoD = clamp(LoD + 1.0, 0.0, maxLoD);
		}
	}
	return p;
}
/*
vec3 trace(vec3 a, vec3 u, out bool hit)
{
	hit = false;
	vec3 p = a;
	for(int i = 0; i < Steps; i++)
	{
		float v = object(p);
		if (v > 0.0)
		{
			hit = true; 
			return p;
		}
		
		p += Epsilon * u;
	}
	return p;
}
*/
//////////////////////////////////////////////////////////////////////////

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

const vec3 LightDirection = normalize(vec3(0.0, -1.0, 1.0));

out vec4 outColor;
uniform vec3 Forward;
void main(void)
{
	Steps = int(sqrt(2.0) * Tex3DRes * 10); // Max. ray steps before bailing out
	Epsilon = 0.01; // Marching epsilon

	vec2 pixel = (gl_FragCoord.xy / Resolution.xy) * 2.0 - 1.0;
	float asp = Resolution.x / Resolution.y;
	
	vec3 rd = normalize(vec3(asp * pixel.x, pixel.y, 2.0));
	vec3 ro = CameraPosition;

	const vec3 up = vec3(0.0, -1.0, 0.0);
	const vec3 right = normalize(cross(up, Forward));
	const mat4 viewMatrix = mat4(vec4(right, 0), vec4(cross(right, Forward), 0), vec4(Forward, 0), vec4(vec3(0.0), 1));
	
	rd = vec3(viewMatrix * vec4(rd, 1));
	
	//////////////////////////////////////////////
	
	const int box_count = 3;
	vec3 box_positions[box_count] = {vec3(0.0), vec3(-128.0, 0.0, 0.0), vec3(128.0, 0.0, 0.0)};
	vec3 box_colors[box_count] = {vec3(0.0, 1.0, 0.0), vec3(1.0, 0.0, 0.0), vec3(0.0, 0.0, 1.0)};
	
	vec3 rgb = vec3(0.0, 0.0, 0.5 * max(0.5, pixel.y + 0.5) + 0.1);
	float t = 100000000.0;
	int s = 0;
	bool hit = false;
	bool hits[box_count] = {false, false, false};
	float ts[box_count] = {100000000, 100000000, 100000000};
	vec3 pos = vec3(0.0);
	
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
			pos = lodTrace(ro + ts[i] * rd - box_positions[i], rd, hit, s);
			
			rgb *= 0.5f; // Debug
			
			if (hit)
			{
				/*
				vec3 center = box_positions[i] + ivec3(pos);
				float t = 0;
				hits[i] = traceBox(ro, rd, center + vec3(-0.5), center + vec3(0.5), t);
				pos = ro + rd * t;
				*/
				rgb = box_colors[i];
				current_hit = i;
				
				pos += vec3(0.5 * Tex3DRes);
				vec3 n = pos - (ivec3(pos) + vec3(0.5));
				vec3 an = abs(n);
				if(an.x > an.y && an.x > an.z) n = sign(n.x) * vec3(1.0, 0.0, 0.0);
				else if(an.y > an.x && an.y > an.z) n = sign(n.y) * vec3(0.0, 1.0, 0.0);
				else n = sign(n.z) * vec3(0.0, 0.0, 1.0);
				
				rgb *= max(0.1, dot(n, -LightDirection));
				
			}
		}
	}
		
	outColor = vec4(rgb, 1.0);
}
