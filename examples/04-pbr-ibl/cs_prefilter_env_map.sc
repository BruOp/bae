#include "bgfx_compute.sh"
#include "pbr_helpers.sh"
#include "../common/shaderlib.sh"

#define PI 3.141592653589793

#define GROUP_SIZE 64
#define THREADS 8
#define NUM_SAMPLES 1024u

uniform vec4 u_params;
// u_params.x == roughness
// u_params.y == imgSize

IMAGE2D_ARRAY_RO(s_envMap, rgba16f, 0);
IMAGE2D_ARRAY_WR(s_target, rgba16f, 1);

vec3 toWorldCoords(ivec3 globalId, float size)
{
    float uc = 2.0 * float(globalId.x) / size - 1.0;
    float vc = 2.0 * float(globalId.y) / size - 1.0;
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

// Taken from Wikipedia: https://en.wikipedia.org/wiki/Cube_mapping
ivec3 convertDirToIndex(vec3 dir, float size)
{
    vec3 absDir = abs(dir);

    bool isXPositive = dir.x > 0;
    bool isYPositive = dir.y > 0;
    bool isZPositive = dir.z > 0;
    int faceId = 0;
    float maxAxis, uc, vc;

    // POSITIVE X
    if (isXPositive && absDir.x >= absDir.y && absDir.x >= absDir.z) {
        // u (0 to 1) goes from +z to -z
        // v (0 to 1) goes from -y to +y
        maxAxis = absDir.x;
        uc = -dir.z;
        vc = dir.y;
        faceId = 0;
    }
    // NEGATIVE X
    if (!isXPositive && absDir.x >= absDir.y && absDir.x >= absDir.z) {
        // u (0 to 1) goes from -z to +z
        // v (0 to 1) goes from -y to +y
        maxAxis = absDir.x;
        uc = dir.z;
        vc = dir.y;
        faceId = 1;
    }
    // POSITIVE Y
    if (isYPositive && absDir.y >= absDir.x && absDir.y >= absDir.z) {
        // u (0 to 1) goes from -x to +x
        // v (0 to 1) goes from +z to -z
        maxAxis = absDir.y;
        uc = dir.x;
        vc = -dir.z;
        faceId = 2;
    }
    // NEGATIVE Y
    if (!isYPositive && absDir.y >= absDir.x && absDir.y >= absDir.z) {
        // u (0 to 1) goes from -x to +x
        // v (0 to 1) goes from -z to +z
        maxAxis = absDir.y;
        uc = dir.x;
        vc = dir.z;
        faceId = 3;
    }
    // POSITIVE Z
    if (isZPositive && absDir.z >= absDir.x && absDir.z >= absDir.y) {
        // u (0 to 1) goes from -x to +x
        // v (0 to 1) goes from -y to +y
        maxAxis = absDir.z;
        uc = dir.x;
        vc = dir.y;
        faceId = 4;
    }
    // NEGATIVE Z
    if (!isZPositive && absDir.z >= absDir.x && absDir.z >= absDir.y) {
        // u (0 to 1) goes from +x to -x
        // v (0 to 1) goes from -y to +y
        maxAxis = absDir.z;
        uc = -dir.x;
        vc = dir.y;
        faceId = 5;
    }

    // Convert range from -1 to 1 to 0 to 1
    return ivec3(
        round(0.5 * (uc / maxAxis + 1.0) * size),
        round(0.5 * (vc / maxAxis + 1.0) * size),
        faceId
    );
}

// So we need to create a mip map chain of our environment map
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
            L = fixCubeLookup(L, u_params.y, u_params.z);
            ivec3 imagePos = convertDirToIndex(L, imgSize);
            prefilteredColor += imageLoad(s_envMap, imagePos).rgb * NoL;
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

    // For the 0th mip level,
    if (u_params.x == 0.0) {
        vec4 color = imageLoad(s_envMap, globalId);
        imageStore(s_target, globalId, color);
        return;
    }

    vec3 R = normalize(toWorldCoords(globalId, imgSize / pow(2.0, mipLevel)));
    vec3 color = prefilterEnvMap(u_params.x, R, imgSize);
    imageStore(s_target, globalId, vec4(color, 1.0));
}
