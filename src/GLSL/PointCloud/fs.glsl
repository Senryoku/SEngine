#version 440

uniform sampler3D	Data0;
uniform sampler3D	Data0_n;
uniform vec3		Resolution;
uniform vec3		CameraPosition;
uniform vec3 		Forward;

const vec3 LightDirection = normalize(vec3(0.0, -1.0, 1.0));

const float Tex3DRes = 256.0;

float falloff(float r, float R)
{
	float x = clamp(r / R, 0.0, 1.0);
	float y = (1.0 - x * x);
	return y * y * y;
}

float point(vec3 p, vec3 c, float e, float R)
{
	return e * falloff(length(p - c), R);
}

float SphereTracedObject(vec3 p)
{
	return texture(Data0, p + vec3(0.5, 0.5, 0.5)).x;
}

// Normal of the SphereTracedObject at point p
// p : point
vec3 SphereTracedObjectNormal(in vec3 p)
{
	return texture(Data0_n, p + vec3(0.5, 0.5, 0.5)).xyz;
	
	//float eps = 0.0001;
	float eps = 1.0 / Tex3DRes;
	vec3 n;
	vec3 ex = vec3(eps, 0.0, 0.0);
	vec3 ey = vec3(0.0, eps, 0.0);
	vec3 ez = vec3(0.0, 0.0, eps);
	n.x = SphereTracedObject(p + ex) - SphereTracedObject(p - ex);
	n.y = SphereTracedObject(p + ey) - SphereTracedObject(p - ey);
	n.z = SphereTracedObject(p + ez) - SphereTracedObject(p - ez);
	return -normalize(n);
}

const int Steps = 500; // Max. ray steps before bailing out
const float Epsilon = 0.5 * 1.0 / Tex3DRes; // Marching epsilon

const float RayMaxLength = 2.0;

// Returns lipschtz constant of the object's function
// /!\ Have to be updated with the object function...
float object_lipschitz()
{
    return 11.0; // Good enough :] More permissive constant => Speeds up the whole thing. (But can cause artefacts)
	//return 7.0 * Energy * falloff_lipschitz(Radius) + Energy * falloff_lipschitz(2.0);
}

// Trace ray using sphere tracing
// a : ray origin
// u : ray direction
vec3 SphereTrace(vec3 a, vec3 u, out bool hit)
{
	hit = false;
	vec3 p = a;
	float lambda = object_lipschitz();
	float depth = 0.0;
	float step = 0.0;
	for(int i = 0; i < Steps; i++)
	{
		float v = SphereTracedObject(p);
		if (v > 0.0)
		{
			hit = true; 
			return p;
		}
		
		step = max(abs(v) / lambda, Epsilon);
		depth += step;
		
		if(depth > RayMaxLength)
			return p;
		
		p += step * u;
	}
	return p;
}

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

out vec4 outColor;

void main(void)
{
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
	vec3 box_positions[box_count] = {vec3(0.0), vec3(-3.0, 0.0, 0.0), vec3(3.0, 0.0, 0.0)};
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
		hits[i] = traceBox(ro, rd, box_positions[i] + vec3(-0.5), box_positions[i] + vec3(0.5), ts[i]);
	}
	
	int current_hit = 0;
	
	for(int i = 0; i < box_count; ++i)
	{
		if((!hit || ts[i] < ts[current_hit]) && hits[i])
		{
			pos = SphereTrace(ro + ts[i] * rd - box_positions[i], rd, hit);
			
			rgb *= 0.5; // Debug
			
			if(hit)
			{
				rgb = box_colors[i];
				current_hit = i;
				
				
				bool shadow_hit = false;
				SphereTrace(pos - 0.01 * LightDirection, -LightDirection, shadow_hit);
				if(!shadow_hit)
					rgb *= min(1.0, 0.1 + max(0.0, dot(SphereTracedObjectNormal(pos), -LightDirection)));
				else 
					rgb *= 0.1;
			}
		}
	}
		
	outColor = vec4(rgb, 1.0);
}
