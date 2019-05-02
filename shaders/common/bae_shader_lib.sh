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

vec3 toLinearAccurate(vec3 _rgb)
{
    vec3 lo = _rgb / 12.92;
    vec3 hi = pow( (_rgb + 0.055) / 1.055, vec3_splat(2.4) );
    vec3 rgb = mix(hi, lo, vec3(lessThanEqual(_rgb, vec3_splat(0.04045) ) ) );
    return rgb;
}

vec4 toLinearAccurate(vec4 _rgba)
{
    return vec4(toLinearAccurate(_rgba.xyz), _rgba.w);
}

// ---------------LIGHTING---------------

vec4 getLightDirDistance(vec3 pos, vec3 lightPos) {
    vec3 pointToLight = lightPos - pos;
    float _distance = length(pointToLight);
    return vec4(pointToLight / _distance, _distance);
}

#endif // __BAESHADERLIB_SH__
