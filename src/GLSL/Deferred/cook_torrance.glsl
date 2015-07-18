/**
 * p : position
 * n : normal
 * rd : view vector
 * c : color
 * lp : light postion
 * lc : light color
 * R : roughness
 * F0 : Fresnel reflectance
 * k : diffuse reflection
**/
vec3 cookTorrance(vec3 p, vec3 n, vec3 rd, vec3 c, vec3 lp, vec3 lc, float R, float F0)
{ 
    vec3 lightDirection = normalize(lp - p);

    float NdotL = dot(n, lightDirection);
    	
    if(NdotL > 0.0)
    {
		NdotL = max(NdotL, 0.000001);
        // calculate intermediary values
        vec3 halfVector = normalize(lightDirection + rd);
        const float NdotH = max(dot(n, halfVector), 0.000001); 
        const float NdotV = max(dot(n, rd), 0.000001); // note: this could also be NdotL, which is the same value
        const float VdotH = max(dot(rd, halfVector), 0.000001);
        const float mSquared = R * R;
        
        // geometric attenuation
        float NH2 = 2.0 * NdotH;
        float g1 = (NH2 * NdotV) / VdotH;
        float g2 = (NH2 * NdotL) / VdotH;
        float geoAtt = min(1.0, min(g1, g2));
     
        // roughness (or: microfacet distribution function)
        // beckmann distribution function
        float r1 = 1.0 / ( 4.0 * mSquared * pow(NdotH, 4.0));
        float r2 = (NdotH * NdotH - 1.0) / (mSquared * NdotH * NdotH);
        float roughness = r1 * exp(r2);
        
        // fresnel
        // Schlick approximation
        float fresnel = pow(1.0 - VdotH, 5.0);
        fresnel *= (1.0 - F0);
        fresnel += F0;
        
		// Standard Cook-Torrance also has NdotL in the denominator, 
		// but we can skip it has specular should be multiplied by NdotL after.
        float specular = (fresnel * geoAtt * roughness) / (NdotV * 3.14159);
		vec3 diffuse = NdotL * c;
		
		// Full diffuse and specular reflection
		//return lc * (diffuse + specular);
		
		// Aproximate energy conservation
		return lc * ((1.0 - F0) * diffuse + specular);
    } else {
		return vec3(0.0);
	}
}
