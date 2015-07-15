#version 430 core
 
uniform samplerCube SkyBox;

in layout(location = 0) vec3 texcoords;
out vec4 colorOut;
out vec4 Out1;
out vec4 Out2;
 
void main()
{	
	colorOut = vec4(texture(SkyBox, texcoords).rgb, -1.0);
	Out1 = vec4(0.0);
	Out2 = vec4(0.0);
}
