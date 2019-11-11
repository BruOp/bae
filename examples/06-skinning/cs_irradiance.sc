#include "bgfx_compute.sh"
#include "../common/pbr_helpers.sh"

#define TWO_PI 6.2831853071795864769252867665590
#define HALF_PI 1.5707963267948966192313216916398

#define THREADS 8

SAMPLERCUBE(s_source, 0);
IMAGE2D_ARRAY_WR(s_target, rgba16f, 1);

NUM_THREADS(THREADS, THREADS, 6)
void main()
{
    int imgSize = 64;
    ivec3 globalId = gl_GlobalInvocationID.xyz;

    vec3 N = normalize(toWorldCoords(globalId, float(imgSize)));

    vec3 up = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    const vec3 right = normalize(cross(up, N));
    up = cross(N, right);

    vec3 color = vec3_splat(0.0);
    uint sampleCount = 0u;
    float deltaPhi = TWO_PI / 360.0;
    float deltaTheta = HALF_PI / 90.0;
    for (float phi = 0.0; phi < TWO_PI; phi += deltaPhi) {
        for (float theta = 0.0; theta < HALF_PI; theta += deltaTheta) {
            // Spherical to World Space in two steps...
            vec3 tempVec = cos(phi) * right + sin(phi) * up;
            vec3 sampleVector = cos(theta) * N + sin(theta) * tempVec;
            color += textureCubeLod(s_source, sampleVector, 0).rgb * cos(theta) * sin(theta);
            sampleCount++;
        }
    }
    imageStore(s_target, globalId, vec4(PI * color / float(sampleCount), 1.0));
}
