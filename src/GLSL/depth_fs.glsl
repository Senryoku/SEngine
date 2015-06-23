#version 430 core

out layout(location = 0) float fragmentdepth;
void main()
{
	fragmentdepth = gl_FragCoord.z;
}
