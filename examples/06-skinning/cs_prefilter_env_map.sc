#include "bgfx_compute.sh"
#include "../common/pbr_helpers.sh"

#define PI 3.141592653589793

#define THREADS 8
#define NUM_SAMPLES 64u

uniform vec4 u_params;
// u_params.x == roughness
// u_params.y == imgSize

SAMPLERCUBE(s_source, 0);
IMAGE2D_ARRAY_WR(s_target, rgba16f, 1);

// From Karis, 2014
vec3 prefilterEnvMap(float roughness, vec3 R, float imgSize)
{
    // Isotropic approximation: we lose stretchy reflections :(
    vec3 N = R;
    vec3 V = R;
    vec3 prefilteredColor = vec3_splat(0.0);
    float totalWeight = 0.0;

    for (uint i = 0u; i < NUM_SAMPLES; i++) {
        vec2 Xi = hammersley(i, NUM_SAMPLES);
        vec3 H = importanceSampleGGX(Xi, roughness, N);
        float VoH = dot(V, H);
        float NoH = VoH; // Since N = V in our approximation
        vec3 L = 2.0 * VoH * H - V;
        float NoL = saturate(dot(N, L));
        NoH = saturate(NoH);

        if (NoL > 0.0) {
            // Based off https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch20.html
            // Typically you'd have the following:
            // float pdf = D_GGX(NoH, roughness) * NoH / (4.0 * VoH);
            // but since V = N => VoH == NoH
            float pdf = D_GGX(NoH, roughness) / 4.0 + 0.001;
            // Solid angle of current sample -- bigger for less likely samples
            float omegaS = 1.0 / (float(NUM_SAMPLES) * pdf);
            // Solid angle  of pixel
            float omegaP = 4.0 * PI / (6.0 * imgSize * imgSize);
            float mipLevel = roughness == 0.0 ? 0.0 : max(0.5 * log2(omegaS / omegaP), 0.0);
            prefilteredColor += textureCubeLod(s_source, L, mipLevel).rgb * NoL;
            totalWeight += NoL;
        }
    }
    return prefilteredColor / totalWeight;
}

NUM_THREADS(THREADS, THREADS, 6)
void main()
{
    float mipLevel = u_params.y;
    float imgSize = u_params.z;
    float mipImageSize = imgSize / pow(2.0, mipLevel);
    ivec3 globalId = ivec3(gl_GlobalInvocationID.xyz);

    if (globalId.x >= mipImageSize || globalId.y >= mipImageSize)
    {
        return;
    }

    vec3 R = normalize(toWorldCoords(globalId, mipImageSize));

    // Don't need to integrate for roughness == 0, since it's a perfect reflector
    if (u_params.x == 0.0) {
        vec4 color = textureCubeLod(s_source, R, 0);
        imageStore(s_target, globalId, color);
        return;
    }

    vec3 color = prefilterEnvMap(u_params.x, R, imgSize);
    imageStore(s_target, globalId, vec4(color, 1.0));
}
