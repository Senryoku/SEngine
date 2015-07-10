#version 430 core
 
uniform samplerCube SkyBox;

in layout(location = 0) vec3 texcoords;
out vec4 colorOut;
 
void main()
{	
	colorOut = vec4(texture(SkyBox, texcoords).rgb, 0.0);
}
