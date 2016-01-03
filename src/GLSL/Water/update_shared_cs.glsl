#version 430

uniform int iterations = 10;
uniform float time = 0.0;
uniform int size_x = 200;
uniform int size_y = 200;
uniform float cell_size;
uniform float moyheight = 2.0;
uniform vec3 acceleration = vec3(0.0, -9.0, 1.0);
uniform float damping = 0.1;

ivec2 coord;
	
struct WaterCell
{
	vec4 data; // Water Height, Ground Height, Speed(xy)
};

layout(std140, binding = 4) buffer InBuffer
{
	WaterCell	Ins[];
};

const ivec2 offsets[4] = {             ivec2(0, -1), 
						  ivec2(-1, 0),             ivec2(1, 0),
						               ivec2(0,  1)};

int to1D(ivec2 c)
{
	return c.y * size_x + c.x;
}

float interpolate(vec2 c, float v00, float v01, float v10, float v11)
{
	vec2 v = vec2(v00 * (1.0 - c.y) + v01 * c.y, v10 * (1.0 - c.y) + v11 * c.y);
	return v.x * (1.0 - c.x) + v.y * c.x;
}

vec3 interpolate(vec2 c, vec3 v00, vec3 v01, vec3 v10, vec3 v11)
{
	vec3 v[2] = {v00 * (1.0 - c.y) + v01 * c.y, v10 * (1.0 - c.y) + v11 * c.y};
	return v[0] * (1.0 - c.x) + v[1] * c.x;
}

bool valid(vec2 c)
{
	return (c.x >= 0 && c.y >= 0 && c.x < size_x && c.y < size_y);
}

shared vec4 neighbors[16][16];

bool inWorkgroup(ivec2 local_c)
{
	return all(greaterThanEqual(local_c, ivec2(0, 0))) && all(lessThan(local_c, ivec2(16, 16)));
}

ivec2 getLocal(ivec2 c)
{
	return ivec2(gl_LocalInvocationID.xy) + (c - coord);
}

vec4 get(ivec2 c)
{
	ivec2 local = getLocal(c);
	vec4 r;
	(inWorkgroup(local)) ?
		r = neighbors[local.x][local.y]
	:
		r = Ins[to1D(c)].data;
	return r;
}

layout (local_size_x = 16, local_size_y = 16) in;
void main()
{
	float t = time / iterations;
	coord = ivec2(gl_GlobalInvocationID.xy);
	int idx = to1D(coord);
	bool inbound = coord.x < size_x && coord.y < size_y && idx < size_x * size_y;

	if(inbound)
		neighbors[gl_LocalInvocationID.x][gl_LocalInvocationID.y] = Ins[idx].data;
	barrier();
	
	vec4 local = neighbors[gl_LocalInvocationID.x][gl_LocalInvocationID.y];
	for(int it = 0; it < iterations; ++it)
	{
		if(inbound) // Advect water height (data.x) and velocity (data.zw)
		{			
			vec2 mod_coord = coord - t * local.zw / cell_size;
			vec2 fract_mod_coord = fract(mod_coord);
			
			vec4 v00;
			vec4 v01;
			vec4 v10;
			vec4 v11;
			
			ivec2 trunc_coord;
			trunc_coord = ivec2(mod_coord);
			(!valid(trunc_coord)) ?
				v00 = vec4(moyheight, 0.0, 0.0, 0.0)
			:
				v00 = get(trunc_coord);
				
			trunc_coord = ivec2(mod_coord + vec2(0.0, 1.0));
			(!valid(trunc_coord)) ?
				v01 = vec4(moyheight, 0.0, 0.0, 0.0)
			:
				v01 = get(trunc_coord);
				
			trunc_coord = ivec2(mod_coord + vec2(1.0, 0.0));
			(!valid(trunc_coord)) ?
				v10 = vec4(moyheight, 0.0, 0.0, 0.0)
			:
				v10 = get(trunc_coord);
				
			trunc_coord = ivec2(mod_coord + vec2(1.0, 1.0));
			(!valid(trunc_coord)) ?
				v11 = vec4(moyheight, 0.0, 0.0, 0.0)
			:
				v11 = get(trunc_coord);
			
			local.xzw = interpolate(fract_mod_coord, v00.xzw, v01.xzw, v10.xzw, v11.xzw);
			
			/* // Swizzle bug ? Oô
			Ins[idx].data.xzw = local.xzw;
			neighbors[gl_LocalInvocationID.x][gl_LocalInvocationID.y].xzw = local.xzw;
			*/
			Ins[idx].data = local;
			neighbors[gl_LocalInvocationID.x][gl_LocalInvocationID.y] = local;
		}
		
		barrier();
		
		if(inbound)
		{
			// Update Height
			vec2 grad;
			
			(coord.x == size_x - 1) ?
				grad.x = 0.0 - local.z
			:
				grad.x = get(ivec2(coord.x + 1, coord.y)).z - local.z;
				
			(coord.y == size_y - 1) ?
				grad.y = 0.0 - local.w
			:
				grad.y = get(ivec2(coord.x, coord.y + 1)).w - local.w;
			
			grad = grad / cell_size;
				
			float div = grad.x + grad.y;
			local.x -= local.x * t * div;

			/* // Swizzle bug ? Oô
			Ins[idx].data.x = local.x;
			neighbors[gl_LocalInvocationID.x][gl_LocalInvocationID.y].x = local.x;
			*/
			Ins[idx].data = local;
			neighbors[gl_LocalInvocationID.x][gl_LocalInvocationID.y] = local;
		}
		
		barrier();
		
		if(inbound)
		{
			// Update velocities, works on Water Height (.x) + Ground Height (.y)
			float h = local.x + local.y;
			float h2 = moyheight;
			if(coord.x > 0)
			{
				vec2 xy = get(ivec2(coord.x - 1, coord.y)).xy;
				h2 = xy.x + xy.y;
			}
			
			float h3 = moyheight;
			if(coord.y > 0)
			{
				vec2 xy = get(ivec2(coord.x, coord.y - 1)).xy;
				h3 = xy.x + xy.y;
			}
			
			local.zw *= (1.0 - damping*t);
			
			local.z += 9.81 * ( (h2 - h) / cell_size ) * t;
			local.w += 9.81 * ( (h3 - h) / cell_size ) * t;
			
			Ins[idx].data = local;
			neighbors[gl_LocalInvocationID.x][gl_LocalInvocationID.y] = local;
		}
	}
}
