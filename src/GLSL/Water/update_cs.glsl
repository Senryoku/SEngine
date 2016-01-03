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

layout (local_size_x = 16, local_size_y = 16) in;
void main()
{
	float t = time / iterations;
	coord = ivec2(gl_GlobalInvocationID.xy);
	int idx = to1D(coord);
	bool inbound = coord.x < size_x && coord.y < size_y && idx < size_x * size_y;
	
	vec4 local = Ins[idx].data;
	for(int it = 0; it < iterations; ++it)
	{
		if(inbound) // Advect water height (data.x) and velocity (data.zw)
		{
			// Ground
			// ... Ground is constant.
			
			vec2 mod_coord = coord - t * local.zw / cell_size;
			vec2 fract_mod_coord = fract(mod_coord);
			
			vec4 v00;
			vec4 v01;
			vec4 v10;
			vec4 v11;
			
			ivec2 trunc_coord;
			trunc_coord = ivec2(mod_coord);
			v00 = (!valid(trunc_coord)) ?
				vec4(moyheight, 0.0, 0.0, 0.0)
			:
				Ins[to1D(trunc_coord)].data;
				
			trunc_coord = ivec2(mod_coord + vec2(0.0, 1.0));
			v01 = (!valid(trunc_coord)) ?
				vec4(moyheight, 0.0, 0.0, 0.0)
			:
				Ins[to1D(trunc_coord)].data;
				
			trunc_coord = ivec2(mod_coord + vec2(1.0, 0.0));
			v10 = (!valid(trunc_coord)) ?
				vec4(moyheight, 0.0, 0.0, 0.0)
			:
				Ins[to1D(trunc_coord)].data;
				
			trunc_coord = ivec2(mod_coord + vec2(1.0, 1.0));
			v11 = (!valid(trunc_coord)) ?
				vec4(moyheight, 0.0, 0.0, 0.0)
			:
				Ins[to1D(trunc_coord)].data;
			
			local.xzw = interpolate(fract_mod_coord, v00.xzw, v01.xzw, v10.xzw, v11.xzw);
			
			//Ins[idx].data.xzw = local.xzw; // Bug ? Oô
			Ins[idx].data = local;
		}
		
		barrier();
		
		if(inbound)
		{
			// Update Height
			vec2 grad;
			
			grad.x = (coord.x == size_x - 1) ?
				0.0 - local.z
			:
				Ins[to1D(ivec2(coord.x + 1, coord.y))].data.z - local.z;
				
			grad.y = (coord.y == size_y - 1) ?
				0.0 - local.w
			:
				Ins[to1D(ivec2(coord.x, coord.y + 1))].data.w - local.w;
			
			grad = grad / cell_size;
				
			float div = grad.x + grad.y;
			local.x -= local.x * t * div;
			Ins[idx].data.x = local.x;
		}
		
		barrier();
		
		if(inbound)
		{
			// Update velocities, works on Water Height (.x) + Ground Height (.y)
			float h = local.x + local.y;
			float h2 = moyheight;
			if(coord.x > 0)
			{
				vec2 val = Ins[to1D(ivec2(coord.x - 1, coord.y))].data.xy;
				h2 = val.x + val.y;
			}
			
			float h3 = moyheight;
			if(coord.y > 0)
			{
				vec2 val = Ins[to1D(ivec2(coord.x, coord.y - 1))].data.xy;
				h3 = val.x + val.y;
			}
			
			local.zw *= (1.0 - damping*t);
			
			local.z += 9.81 * ( (h2 - h) / cell_size ) * t;
			local.w += 9.81 * ( (h3 - h) / cell_size ) * t;
			
			Ins[idx].data = local;
		}
	}
}
