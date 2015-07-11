#version 430 core

uniform vec2	Resolution = vec2(1366, 720);

uniform vec2	Position = vec2(0.0);

uniform vec2	Min = vec2(0.0);
uniform vec2	Max = vec2(100.0);

vec2 s(vec2 v)
{
	return 2.0 * (v / Resolution) - 1.0;
}

void main()
{
    gl_Position = vec4(s(Min + Position), s(Max + Position));
}
