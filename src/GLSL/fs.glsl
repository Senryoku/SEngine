#version 430 core

uniform vec4 Color = vec4(1.0);

out layout(location = 0) vec4 ColorOut;

void main(void)
{
	ColorOut = Color;
}
