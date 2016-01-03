#version 440

uniform sampler3D	Data0;
uniform vec3		Resolution;
uniform vec3		CameraPosition;

uniform float Time = 0.0;

// Configuration
uniform float Tex3DRes = 32.0;
uniform float displayedLoD = 0.0;

uniform float Ambiant = 0.2;

int Steps; // Max. ray steps before bailing out
float maxLoD = log2(32) - 1;

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

bool lodTrace(vec3 a, vec3 u, out vec3 p, out vec3 n, out int i)
{
	p = a + Tex3DRes / 2;
	float step = 0.0;
	float LoD = maxLoD;
	float depth = 0.0;
	vec3 dist = vec3(0);
	vec3 abs_dir = abs(u);
	ivec3 dirSign;
	dirSign.x = u.x < 0.0f ? -1 : 1;
    dirSign.y = u.y < 0.0f ? -1 : 1;
    dirSign.z = u.z < 0.0f ? -1 : 1;
	
	ivec3 rayPos = ivec3(p);
    vec3 absDir = abs(u);
    vec3 projLength = 1.0 / (absDir + 0.0001);
    vec3 d = p - rayPos;
    if(dirSign.x == 1) d.x = 1-d.x;
    if(dirSign.y == 1) d.y = 1-d.y;
    if(dirSign.z == 1) d.z = 1-d.z;
	projLength *= pow(2, LoD);
    d *= projLength;
	
	float v = texelFetch(Data0, ivec3(rayPos / pow(2, LoD)), int(LoD)).x;
	for(i = 0; i < Steps; i++)
	{
		if (v > 0.0)
		{
			if(LoD < displayedLoD + 0.5)
			{
				p = rayPos + 0.5;
				
				d = abs(d - projLength);
				if(d.x < 0.0001) n = -dirSign.x * vec3(1, 0, 0);
				if(d.y < 0.0001) n = -dirSign.y * vec3(0, 1, 0);
				if(d.z < 0.0001) n = -dirSign.z * vec3(0, 0, 1);
				n = normalize(n);
				
				return true;
			}
			LoD = clamp(LoD - 1.0, 0.0, maxLoD);
			projLength = 1.0 / (absDir + 0.0001) * pow(2, LoD);
		} else {
            if(d.x < d.y || d.z < d.y) {
                if(d.x < d.z) {
                    rayPos.x += dirSign.x * int(pow(2, LoD));
                    d.yz -= d.x;
                    d.x = projLength.x;
                } else {
                    rayPos.z += dirSign.z * int(pow(2, LoD));
                    d.xy -= d.z;
                    d.z = projLength.z;
                }
            } else {
                rayPos.y += dirSign.y * int(pow(2, LoD));
                d.xz -= d.y;
                d.y = projLength.y;
            }

			//LoD = clamp(LoD + 1.0, 0.0, maxLoD);
		}
		
		if(any(greaterThanEqual(rayPos, ivec3(Tex3DRes, Tex3DRes, Tex3DRes))) || 
			any(lessThan(rayPos, ivec3(0, 0, 0))))
            return false;
			
		v = texelFetch(Data0, ivec3(rayPos / pow(2, LoD)), int(LoD)).x;
	}

	return false;
}

bool trace(vec3 a, vec3 u, out vec3 p, out vec3 n, out int i)
{
	p = a + Tex3DRes / 2;
	float step = 0.0;
	float depth = 0.0;
	vec3 dist = vec3(0);
	vec3 abs_dir = abs(u);
	ivec3 dirSign;
	dirSign.x = u.x < 0.0f ? -1 : 1;
    dirSign.y = u.y < 0.0f ? -1 : 1;
    dirSign.z = u.z < 0.0f ? -1 : 1;
	
	ivec3 rayPos = ivec3(p);
    vec3 absDir = abs(u);
    vec3 projLength = 1.0 / (absDir + 0.0001);
    vec3 d = p - rayPos;
    if(dirSign.x == 1) d.x = 1-d.x;
    if(dirSign.y == 1) d.y = 1-d.y;
    if(dirSign.z == 1) d.z = 1-d.z;
    d *= projLength;
	
	float v = texelFetch(Data0, ivec3(rayPos / pow(2, displayedLoD)), int(displayedLoD)).x;
	for(i = 0; i < Steps; i++)
	{
		if (v > 0.0)
		{
			p = rayPos + 0.5;
			
			d = abs(d - projLength);
			if(d.x < 0.0001) n = -dirSign.x * vec3(1, 0, 0);
			if(d.y < 0.0001) n = -dirSign.y * vec3(0, 1, 0);
			if(d.z < 0.0001) n = -dirSign.z * vec3(0, 0, 1);
			n = normalize(n);
			
			return true;
		} else {
            if(d.x < d.y || d.z < d.y) {
                if(d.x < d.z) {
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
		}
		
		if(any(greaterThanEqual(rayPos, ivec3(Tex3DRes, Tex3DRes, Tex3DRes))) || 
			any(lessThan(rayPos, ivec3(0, 0, 0))))
            return false;
			
		v = texelFetch(Data0, ivec3(rayPos / pow(2, displayedLoD)), int(displayedLoD)).x;
	}

	return false;
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
			
			rgb *= 0.5f; // Debug
			
			if(lodTrace(ro + ts[i] * rd - box_positions[i], rd, pos, n, s))
			//if(trace(ro + ts[i] * rd - box_positions[i], rd, pos, n, s))
			{
				hit = true;
				/*
				vec3 center = box_positions[i] + ivec3(pos);
				float t = 0;
				hits[i] = traceBox(ro, rd, center + vec3(-0.5), center + vec3(0.5), t);
				pos = ro + rd * t;
				*/
				rgb = box_colors[i];
				current_hit = i;
				/*
				pos += vec3(0.5 * Tex3DRes);
				vec3 n = pos - (ivec3(pos) + vec3(0.5));
				vec3 an = abs(n);
				if(an.x > an.y && an.x > an.z) n = sign(n.x) * vec3(1.0, 0.0, 0.0);
				else if(an.y > an.x && an.y > an.z) n = sign(n.y) * vec3(0.0, 1.0, 0.0);
				else n = sign(n.z) * vec3(0.0, 0.0, 1.0);
				*/
			}
		}
	}

	if(hit)
	{
		// Get precise position
		float t;
		vec3 voxel_world_pos = pos - vec3(Tex3DRes / 2) + box_positions[current_hit];
		if(traceBox(ro, rd, voxel_world_pos - vec3(0.51), 
							voxel_world_pos + vec3(0.51), t))
		{
			pos = ro + t * rd;

			// Shadow casting
			vec3 dummy;
			bool shadow_hit = false;
			for(int i = 0; i < box_count; ++i)
			{
				if(traceBox(pos, -LightDirection, box_positions[i] + vec3(-0.5 * Tex3DRes), box_positions[i] + vec3(0.5 * Tex3DRes), t))
					if(trace(pos - t * LightDirection - box_positions[i] - 0.1 * LightDirection, -LightDirection, dummy, dummy, s))
					{
						shadow_hit = true;
						break;
					}
			}
			if(!shadow_hit)
				rgb *= max(Ambiant, dot(n, -LightDirection));
			else
				rgb *= Ambiant;
		}
	}
	
	outColor = vec4(rgb, 1.0);
}
