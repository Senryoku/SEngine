#version 430 core

in layout(location = 0) vec2 position;
in layout(location = 1) vec2 texcoord;

out vec2 texcoords;

void main()
{
    gl_Position = vec4(2.0 * (position / vec2(1366, 720)) - 1.0, 0.0, 1.0);
	
	texcoords = texcoord;
}
