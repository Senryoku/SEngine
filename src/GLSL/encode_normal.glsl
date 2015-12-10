vec2 encode_normal(vec3 n)
{
    vec2 enc = normalize(n.xy) * (sqrt(-n.z * 0.5 + 0.5));
    return enc * 0.5 + 0.5;
}

vec3 decode_normal(vec2 enc)
{
    vec4 nn = vec4(enc, 0, 0) * vec4(2, 2, 0, 0) + vec4(-1, -1, 1, -1);
    float l = dot(nn.xyz,-nn.xyw);
    nn.z = l;
    nn.xy *= sqrt(l);
    return nn.xyz * 2 + vec3(0,0,-1);
}
