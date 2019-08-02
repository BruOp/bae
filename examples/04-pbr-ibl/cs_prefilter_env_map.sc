#include "bgfx_compute.sh"
#include "pbr_helpers.sh"
#include "../common/shaderlib.sh"

#define PI 3.141592653589793

#define GROUP_SIZE 64
#define THREADS 8
#define NUM_SAMPLES 2048u

uniform vec4 u_params;
// u_params.x == roughness
// u_params.y == imgSize

SAMPLERCUBE(s_source, 0);
IMAGE2D_ARRAY_WR(s_target, rgba16f, 1);

vec3 toWorldCoords(ivec3 globalId, float size)
{
    float uc = 2.0 * float(globalId.x) / size - 1.0;
    float vc = -(2.0 * float(globalId.y) / size - 1.0);
    switch (globalId.z) {
    case 0:
        return vec3(1.0, vc, -uc);
    case 1:
        return vec3(-1.0, vc, uc);
    case 2:
        return vec3(uc, 1.0, -vc);
    case 3:
        return vec3(uc, -1.0, vc);
    case 4:
        return vec3(uc, vc, 1.0);
    default:
        return vec3(-uc, vc, -1.0);
    }
}



// From Karis, 2014
vec3 prefilterEnvMap(float roughness, vec3 R, float imgSize)
{
    vec3 N = R;
    vec3 V = R;
    vec3 prefilteredColor = vec3_splat(0.0);
    float totalWeight = 0.0;

    for (uint i = 0u; i < NUM_SAMPLES; i++) {
        vec2 Xi = hammersley(i, NUM_SAMPLES);
        vec3 H = importanceSampleGGX(Xi, roughness, N);
        vec3 L = 2.0 * dot(V, H) * H - V;
        float NoL = saturate(dot(N, L));

        if (NoL > 0.0) {
            prefilteredColor += textureCubeLod(s_source, L, 0).rgb * NoL;
            totalWeight += NoL;
        }
    }
    return prefilteredColor / totalWeight;
}

vec3 sphericalToCartesian(vec2 phiTheta)
{
  return vec3(
    sin(phiTheta.y) * cos(phiTheta.x),
    sin(phiTheta.y) * sin(phiTheta.x),
    cos(phiTheta.y)
  );
}

NUM_THREADS(THREADS, THREADS, 6)
void main()
{
    int mipLevel = u_params.y;
    float imgSize = u_params.z;
    int i_imgSize = round(imgSize);
    ivec3 globalId = ivec3(gl_GlobalInvocationID.xyz);

    if (globalId.x >= i_imgSize || globalId.y >= i_imgSize)
    {
        return;
    }

    vec3 R = normalize(toWorldCoords(globalId, imgSize / pow(2.0, mipLevel)));

    // Don't need to integrate for roughness == 0, since it's a perfect reflector
    if (u_params.x == 0.0) {
        vec4 color = textureCubeLod(s_source, R, 0);
        imageStore(s_target, globalId, color);
        return;
    }

    vec3 color = prefilterEnvMap(u_params.x, R, imgSize);
    imageStore(s_target, globalId, vec4(color, 1.0));
}
