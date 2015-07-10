#version 430 core
 
uniform samplerCube SkyBox;

in vec2 texcoord;
in mat4 cubeMatrix;
 
out vec4 colorOut;
 
void main()
{
	vec2 pixel = texcoord - 0.5;
	vec3 rd = mat3(cubeMatrix) * normalize(vec3(pixel.x, pixel.y, 0.5)).xyz;
	
	colorOut = vec4(texture(SkyBox, rd).rgb, 0.0);
	//colorOut = vec4(1.0, 0.0, 0.0, 1.0);
}
