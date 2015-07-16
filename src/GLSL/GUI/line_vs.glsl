#version 430 core

uniform vec2	Resolution = vec2(1366, 720);

uniform vec2	Position = vec2(0.0);

vec2 s(vec2 v)
{
	return 2.0 * (v / Resolution) - 1.0;
}

in layout(location = 0) vec2 vertex_position;

void main()
{
    gl_Position = vec4(s(vertex_position + Position), 0.0, 1.0);
}
