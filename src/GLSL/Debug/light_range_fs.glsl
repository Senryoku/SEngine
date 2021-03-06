#version 430

#pragma include ../raytracing.glsl

uniform float particle_size = 0.25;

layout(std140) uniform Camera
{
	mat4 ViewMatrix;
	mat4 ProjectionMatrix;
};

uniform vec3 CameraPosition;

in layout(location = 0) vec3 position;
in layout(location = 1) vec3 color;
in layout(location = 2) vec3 world_position;
in layout(location = 3) vec3 world_normal;
in layout(location = 4) vec2 texcoord;
in layout(location = 5) float range;

out layout(location = 0) vec4 colorMaterialOut;
out layout(location = 1) vec4 worldPositionOut;
out layout(location = 2) vec4 worldNormalOut;

uniform float R = 0.0;
uniform float F0 = 0.1;

vec2 encode_normal(vec3 n)
{
    vec2 enc = normalize(n.xy) * (sqrt(-n.z * 0.5 + 0.5));
    return enc * 0.5 + 0.5;
}

void main()
{
	vec3 p, n;
	if(traceSphere(CameraPosition, normalize(world_position - CameraPosition), position, range, p, n))
	{
		vec4 tmp = ProjectionMatrix * ViewMatrix * vec4(p, 1.0);
		gl_FragDepth = ((gl_DepthRange.diff * tmp.z / tmp.w) + gl_DepthRange.near + gl_DepthRange.far) / 2.0;
	} else {
		discard;
	}
	
	colorMaterialOut = vec4((0.5 + 0.5 * dot(n, normalize(vec3(1.0)))) * color, 0.0);
	worldPositionOut = vec4(p, gl_FragDepth);
	worldNormalOut = vec4(encode_normal(n), 1.0, 1.0);
}
