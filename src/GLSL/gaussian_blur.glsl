// Specify:
//  const int kernel_size;
//  const int kernel_radius = kernel_size / 2;
//  uniform int offset[kernel_radius + 1];
//  uniform float weight[kernel_radius + 1];
// and define HORIZONTAL_PASS or VERTICAL_PASS
// then include this file to our shader

layout(binding = 0, rgba32f) uniform image2D Texture;

#define WORKGROUP_SIZE 32

shared vec4 cache[WORKGROUP_SIZE + kernel_radius * 2];

#ifdef VERTICAL_PASS
layout (local_size_x = 1, local_size_y = WORKGROUP_SIZE) in;
const uint		pass = 1;
const ivec2	rad = ivec2(0, kernel_radius);
#else
layout (local_size_x = WORKGROUP_SIZE) in;
const uint		pass = 0;
const ivec2	rad = ivec2(kernel_radius, 0);
#endif

void main(void)
{
	ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);
	
	vec4 color;
	int cache_id = int(gl_LocalInvocationID[pass]) + kernel_radius;
	
	// Loading data in shared memory
	cache[cache_id] = imageLoad(Texture, pixel);
	if(gl_LocalInvocationID[pass] < kernel_radius)
		cache[gl_LocalInvocationID[pass]] = imageLoad(Texture, pixel - rad);
	else if(gl_LocalInvocationID[pass] >= WORKGROUP_SIZE - kernel_radius)
		cache[gl_LocalInvocationID[pass] + kernel_radius * 2] = imageLoad(Texture, pixel + rad);
	
	barrier();
	
	color = cache[cache_id] * weight[0];
	for(int i = 1; i <= kernel_radius; i++)
	{
		color += cache[cache_id - offset[i]] * weight[i];
		color += cache[cache_id + offset[i]] * weight[i];
	}
	
	imageStore(Texture, pixel, color);
}
