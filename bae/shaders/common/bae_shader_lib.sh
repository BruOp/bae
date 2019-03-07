#ifndef __BAESHADERLIB_SH__
#define __BAESHADERLIB_SH__

#define PI 3.141592653589793

// -------------- UTILS -----------------

float clampDot(vec3 v1, vec3 v2) {
    return clamp(dot(v1, v2), 0.0, 1.0);
}

vec3 toGamma(vec3 _rgb)
{
	return pow(abs(_rgb), vec3_splat(1.0/2.2) );
}

vec3 toLinear(vec3 _rgb)
{
	return pow(abs(_rgb), vec3_splat(2.2) );
}

vec4 toLinear(vec4 _rgba)
{
	return vec4(toLinear(_rgba.xyz), _rgba.w);
}


// ---------------LIGHTING---------------

vec4 getLightDirDistance(vec3 pos, vec3 lightPos) {
    vec3 pointToLight = lightPos - pos;
    float _distance = length(pointToLight);
    return vec4(pointToLight / _distance, _distance);
}

#endif // __BAESHADERLIB_SH__
