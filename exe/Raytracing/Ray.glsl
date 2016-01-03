#version 430

layout(binding = 0, rgba32f) uniform readonly image2D Position;
layout(binding = 1, rgba32f) uniform readonly image2D Normal;
layout(binding = 2, rgba32f) uniform readonly image2D Albedo;
layout(binding = 3, rgba32f) uniform image2D ColorOut;


struct Material
{
    int type;
    float reflectivity;
    
    vec3 diffuse;
    
    // Cook-Torrance
    float roughness;
    float fresnelReflectance;
    float diffuseReflection;
    
    // Blinn-Phong
    float specular;
    
    vec4 infos;
    vec2 uv;
};
    
struct Plane
{
    vec3 p;
    vec3 n;
};

struct Sphere
{
    vec3 c;
    float r;
};
    
Sphere makeSphere(vec3 c, float r) { Sphere s; s.c = c; s.r = r; return s; }

struct Ray
{
    vec3 o;
    vec3 d;
    float m;
};
    
struct Output
{
    vec3 p;
    vec3 n;
    float d;
    Material m;
};
    
struct Light
{
    vec3 color;
    vec3 position;
    float radius;
};

bool tracePlane(Plane p, Ray r, inout Output o)
{
    float d = dot(r.o, p.p);
    
    float l = dot(p.n, (p.p - r.o)) / dot(p.n, r.d);
    
    vec3 h = r.o + l * r.d;
        
   	if(l < 0.0 || l > o.d || l > r.m)
        return false;
     
    // Hit
    o.p = h;
    o.n = p.n;
    o.d = abs(l);
    o.m = Material(1, 0.1, vec3(0.0), 0.8, 0.4, 0.1, 0.0, vec4(0.0), vec2(0.0));
    
    return true;
}

bool traceSphere(Sphere s, Ray r, inout Output o)
{	
    vec3 d = r.o - s.c;
	
	float a = dot(r.d, r.d);
	float b = dot(r.d, d);
	float c = dot(d, d) - s.r * s.r;
	
	float g = b*b - a*c;
	
	if(g > 0.0)
    {
		float dis = (-sqrt(g) - b) / a;
		if(dis > 0.0 && dis < o.d)
        {
			o.p = r.o + r.d * dis;
			o.n = (o.p - s.c) / s.r;
            o.d = dis;
            o.m = Material(0, 0.15, vec3(0.0), 0.2, 0.9, 0.5, 64.0, vec4(s.c, 0.0), vec2(0.0));
            return true;
		}
	}
    return false;
}

bool traceScene(Ray r, inout Output o)
{
    Sphere s;
    bool b = false;
    
    b = tracePlane(Plane(vec3(0.0, -5.0, 0.0), vec3(0.0, 1.0, 0.0)), r, o) || b;
    
    s = makeSphere(vec3(0.0), 2.0);
    b = traceSphere(s, r, o) || b;
    
    return b;
}

void main()
{
	uvec2 pixel = gl_GlobalInvocationID.xy;
	uvec2 local_pixel = gl_LocalInvocationID.xy;
	ivec2 image_size = imageSize(ColorMaterial).xy;
	
	vec3 p = imageLoad(Position, ivec2(pixel)).xyz;
	vec3 n = imageLoad(Normal, ivec2(pixel)).xyz;
	vec3 a = imageLoad(Albedo, ivec2(pixel)).xyz;
	
	vec3 out = imageLoad(ColorOut, ivec2(pixel)).xyz;
	Ray r;
	r.d = ...:
	r.o = p;
	if(traceScene(r, o))
	{
		out += dot(r.d, n) * o.m.color * a;
	}
	
	imageStore(ColorOut, ivec2(pixel), vec4(out, 1.0));
}
