#version 430

layout(points) in;
layout(triangle_strip) out;
layout(max_vertices = 4) out;

layout(std140) uniform Camera
{
	mat4 ViewMatrix;
	mat4 ProjectionMatrix;
};

uniform mat4 ModelMatrix = mat4(1.0);
uniform vec3 CameraPosition;

uniform vec3 CameraRight = vec3(0.0, 0.0, 0.0);

in layout(location = 0) vec3  in_position[];
in layout(location = 1) vec3  in_color[];
in layout(location = 2) float in_draw[];
in layout(location = 3) float in_range[];

out layout(location = 0) vec3 position;
out layout(location = 1) vec3 color;
out layout(location = 2) vec3 world_position;
out layout(location = 3) vec3 world_normal;
out layout(location = 4) vec2 texcoord;
out layout(location = 5) float range;

void main()
{
	if(in_draw[0] > 0)
	{
		mat4 VP = ProjectionMatrix * ViewMatrix;
		vec3 n = normalize(CameraPosition - in_position[0].xyz);
		vec3 up, right;
		if(CameraRight == vec3(0.0))
		{
			up = vec3(0.0, 1.0, 0.0);
			right = 0.5 * in_range[0] * normalize(cross(up, n));
			up = 0.5 * in_range[0] * normalize(cross(right, n));
		} else {
			right = CameraRight;
			up = 0.5 * in_range[0] * normalize(cross(right, n));
			right *= 0.5 * in_range[0];
		}
		
		// Stupidly expending the billboard to ease raytracing, must simpler than figuring out the real exact size.
		right *= 2.0;
		up *= 2.0;
		
		world_position = in_position[0].xyz - up - right;
		gl_Position = VP * vec4(world_position, 1.0);
		position = in_position[0];
		color = in_color[0];
		range = in_range[0];
		world_normal = n;
		texcoord = vec2(0.0, 0.0);
		EmitVertex();
		
		world_position = in_position[0].xyz - up + right;
		gl_Position = VP * vec4(world_position, 1.0);
		position = in_position[0];
		color = in_color[0];
		range = in_range[0];
		world_normal = n;
		texcoord = vec2(1.0, 0.0);
		EmitVertex();
		
		world_position = in_position[0].xyz + up - right;
		gl_Position = VP * vec4(world_position, 1.0);
		position = in_position[0];
		color = in_color[0];
		range = in_range[0];
		world_normal = n;
		texcoord = vec2(0.0, 1.0);
		EmitVertex();
		
		world_position = in_position[0].xyz + up + right;
		gl_Position = VP * vec4(world_position, 1.0);
		position = in_position[0];
		color = in_color[0];
		range = in_range[0];
		world_normal = n;
		texcoord = vec2(1.0, 1.0);
		EmitVertex();
		
		EndPrimitive();
	}
}
