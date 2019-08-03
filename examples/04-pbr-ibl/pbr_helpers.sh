#ifndef PI
#define PI 3.141592653589793
#endif // PI

// Taken from https://github.com/SaschaWillems/Vulkan-glTF-PBR/blob/master/data/shaders/genbrdflut.frag
// Based on http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
vec2 hammersley(uint i, uint N)
{
    uint bits = (i << 16u) | (i >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    float rdi = float(bits) * 2.3283064365386963e-10;
    return vec2(float(i) /float(N), rdi);
}

float D_GGX(float NoH, float linearRoughness) {
    float a = NoH * linearRoughness;
    float k = linearRoughness / (1.0 - NoH * NoH + a * a);
    return k * k * (1.0 / PI);
}

vec3 F_Schlick(float VoH, float reflectance, float metallic, vec3 baseColor) {
    vec3 f0 = mix(vec3_splat(reflectance), baseColor, metallic);
    float f = pow(1.0 - VoH, 5.0);
    return f + f0 * (1.0 - f);
}

// From the filament docs. Geometric Shadowing function
// https://google.github.io/filament/Filament.html#toc4.4.2
float G_Smith(float NoV, float NoL, float roughness)
{
    float k = (roughness * roughness) / 2.0;
    float GGXL = NoL / (NoL * (1.0 - k) + k);
    float GGXV = NoV / (NoV * (1.0 - k) + k);
    return GGXL * GGXV;
}

// Based on Karis 2014
vec3 importanceSampleGGX(vec2 Xi, float roughness, vec3 N)
{
    float a = roughness * roughness;
    // Sample in spherical coordinates
    float phi = 2.0 * PI * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
    // Construct tangent space vector
    vec3 H;
    H.x = sinTheta * cos(phi);
    H.y = sinTheta * sin(phi);
    H.z = cosTheta;

    // Tangent to world space
    vec3 upVector = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangentX = normalize(cross(upVector, N));
    vec3 tangentY = cross(N, tangentX);
    return tangentX * H.x + tangentY * H.y + N * H.z;
}

vec3 toWorldCoords(ivec3 globalId, float size)
{
    vec2 uvc = (vec2(globalId.xy) + 0.5) / size;
    uvc = 2.0 * uvc - 1.0;
    uvc.y *= -1.0;
    switch (globalId.z) {
    case 0:
        return vec3(1.0, uvc.y, -uvc.x);
    case 1:
        return vec3(-1.0, uvc.y, uvc.x);
    case 2:
        return vec3(uvc.x, 1.0, -uvc.y);
    case 3:
        return vec3(uvc.x, -1.0, uvc.y);
    case 4:
        return vec3(uvc.x, uvc.y, 1.0);
    default:
        return vec3(-uvc.x, uvc.y, -1.0);
    }
}
