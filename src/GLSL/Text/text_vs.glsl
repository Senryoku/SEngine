#version 430 core

uniform vec2	Resolution = vec2(1366, 720);
uniform vec2	Position = vec2(0.0);

in layout(location = 0) vec2 position;
in layout(location = 1) vec2 texcoord;

out vec2 texcoords;

void main()
{
    gl_Position = vec4(2.0 * ((Position + position) / Resolution) - 1.0, 0.0, 1.0);
	
	texcoords = texcoord;
}
