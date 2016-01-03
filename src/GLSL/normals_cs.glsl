#version 430

uniform int size_x;
uniform int size_y;

layout(std140, binding = 5) buffer VerticesBlock
{
	vec4 vertices[];
};

layout(std140, binding = 6) buffer NormalsBlock
{
	vec4 normals[];
};


const ivec2 offsets[4] = {             ivec2(0, -1), 
						  ivec2(-1, 0),             ivec2(1, 0),
						               ivec2(0,  1)};

int to1D(ivec2 coord)
{
	return coord.y * size_x + coord.x;
}

bool valid(ivec2 coord)
{
	return (coord.x >= 0 && coord.y >= 0 && coord.x < size_x && coord.y < size_y);
}

layout (local_size_x = 16, local_size_y = 16) in;
void main(void)
{
	ivec2 coord = ivec2(gl_GlobalInvocationID.xy);
	
	if(valid(coord))
	{
		vec3 neighbors[4];
		
		for(int i = 0; i < 4; ++i)
		{
			ivec2 c = coord + offsets[i];
			if(!valid(c)) c = coord;
			
			neighbors[i] = vertices[to1D(c)].xyz;
		}
		normals[to1D(coord)] = vec4(normalize(cross((neighbors[0] - neighbors[3]), (neighbors[2] - neighbors[1]))), 1.0);
	}
}
