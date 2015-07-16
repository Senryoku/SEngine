#version 430 core

uniform vec4	Color = vec4(1.0, 1.0, 1.0, 0.1);

out vec4 ColorOut;

void main(void)
{   
	ColorOut = Color;
}
