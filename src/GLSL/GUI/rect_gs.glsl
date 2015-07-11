#version 430

layout(points) in;
layout(triangle_strip) out;
layout(max_vertices = 4) out;

void main()
{
	vec2 min = gl_in[0].gl_Position.xy;
	vec2 max = gl_in[0].gl_Position.zw;
	
	gl_Position = vec4(min, 0.0, 1.0);
	EmitVertex();
	
	gl_Position = vec4(min.x, max.y, 0.0, 1.0);
	EmitVertex();
	
	gl_Position = vec4(max.x, min.y, 0.0, 1.0);
	EmitVertex();
	
	gl_Position = vec4(max, 0.0, 1.0);
	EmitVertex();
	
	EndPrimitive();
}
