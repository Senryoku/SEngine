#version 430 core

layout(binding = 0, rgba32f) uniform image2D Texture;

#define WORKGROUP_SIZE 32

const int kernel_size = 9;
const int kernel_radius = kernel_size / 2;

uniform int offset[kernel_radius + 1] = int[](0, 1, 2, 3, 4);
uniform float weight[kernel_radius + 1] = float[](0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162);

shared vec4 cache[WORKGROUP_SIZE + kernel_radius * 2];

layout (local_size_x = WORKGROUP_SIZE) in;
void main(void)
{
	ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);
	
	vec4 color;
	int cache_id = int(gl_LocalInvocationID.x) + kernel_radius;
	
	// Horizontal Pass
	
	// Loading data in shared memory
	cache[cache_id] = imageLoad(Texture, pixel);
	if(gl_LocalInvocationID.x < kernel_radius)
		cache[gl_LocalInvocationID.x] = imageLoad(Texture, pixel - ivec2(kernel_radius, 0));
	else if(gl_LocalInvocationID.x >= WORKGROUP_SIZE - kernel_radius)
		cache[gl_LocalInvocationID.x + kernel_radius * 2] = imageLoad(Texture, pixel + ivec2(kernel_radius, 0));
	
	barrier();
	
	color = cache[cache_id] * weight[0];
	for(int i = 1; i <= kernel_radius; i++)
	{
		color += cache[cache_id - offset[i]] * weight[i];
		color += cache[cache_id + offset[i]] * weight[i];
	}
	
	imageStore(Texture, pixel, color);
}
