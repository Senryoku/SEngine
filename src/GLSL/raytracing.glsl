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

/// pp : plane point
/// pn : plane normal
bool tracePlane(vec3 ro, vec3 rd, vec3 pp, vec3 pn, out vec3 p, out vec3 n)
{
	if(dot(pn, rd) == 0) return false;
	float t = dot(pn, pp - ro) / dot(pn, rd);
	if(t > 0)
	{
		p = ro + t * rd;
		n = pn;
	}
	return (t > 0);
}
