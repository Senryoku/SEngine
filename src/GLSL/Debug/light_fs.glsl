#version 430

uniform float particle_size = 0.25;

layout(std140) uniform Camera
{
	mat4 ViewMatrix;
	mat4 ProjectionMatrix;
};

uniform vec3 cameraPosition;

in layout(location = 0) vec3 position;
in layout(location = 1) vec3 color;
in layout(location = 2) vec3 world_position;
in layout(location = 3) vec3 world_normal;
in layout(location = 4) vec2 texcoord;

out layout(location = 0) vec4 colorMaterialOut;
out layout(location = 1) vec4 worldPositionOut;
out layout(location = 2) vec4 worldNormalOut;

uniform float R = 0.4;
uniform float F0 = 0.1;
uniform float k = 0.5;

vec2 encode_normal(vec3 n)
{
    vec2 enc = normalize(n.xy) * (sqrt(-n.z * 0.5 + 0.5));
    return enc * 0.5 + 0.5;
}

bool traceSphere(vec3 ro, vec3 rd, vec3 ce, float r, out vec3 p, out vec3 n)
{	
    vec3 d = ro - ce;
	
	float a = dot(rd, rd);
	float b = dot(rd, d);
	float c = dot(d, d) - r * r;
	
	float g = b*b - a*c;
	
	if(g > 0.0)
    {
		float dis = (-sqrt(g) - b) / a;
		if(dis > 0.0 && dis < 10000)
        {
			p = ro + rd * dis;
			n = (p - ce) / r;
            return true;
		}
	}
    return false;
}

void main()
{
	vec3 p, n;
	if(traceSphere(cameraPosition, normalize(world_position - cameraPosition), position, particle_size * 0.5, p, n))
	{
		vec4 tmp = ProjectionMatrix * ViewMatrix * vec4(p, 1.0);
		gl_FragDepth = ((gl_DepthRange.diff * tmp.z / tmp.w) + gl_DepthRange.near + gl_DepthRange.far) / 2.0;
	} else {
		discard;
	}
	// Can't emit light for now :(
	colorMaterialOut = vec4(color, R);
	worldPositionOut = vec4(p, gl_FragDepth);
	worldNormalOut = vec4(encode_normal(n), F0, k);
}
