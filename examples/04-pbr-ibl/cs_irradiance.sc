#include "bgfx_compute.sh"
#include "pbr_helpers.sh"

#define PI      3.141592653589793
#define TWO_PI  6.283185307179586
#define HALF_PI 1.570796326794896

#define THREADS 8

SAMPLERCUBE(s_source, 0);
IMAGE2D_ARRAY_WR(s_target, rgba16f, 1);

NUM_THREADS(THREADS, THREADS, 6)
void main()
{
    int imgSize = 64;
    ivec3 globalId = ivec3(gl_GlobalInvocationID.xyz);
    if (globalId.x >= imgSize || globalId.y >= imgSize)
    {
        return;
    }

    vec3 N = normalize(toWorldCoords(globalId, float(imgSize)));

    vec3 up = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    const vec3 right = normalize(cross(up, N));
    up = cross(N, right);

    vec3 color = vec3_splat(0.0);
    float sampleCount = 0.0;
    float deltaPhi = TWO_PI / 180.0;
    float deltaTheta = HALF_PI / 64.0;
    for (float phi = 0.0; phi < TWO_PI; phi += deltaPhi) {
        for (float theta = 0.0; theta < HALF_PI; theta += deltaPhi) {
            // Spherical to World Space in two steps...
            vec3 tempVec = cos(phi) * right + sin(phi) * up;
            vec3 sampleVector = cos(theta) * N + sin(theta) * tempVec;
            color += textureCubeLod(s_source, sampleVector, 0).rgb * cos(theta) * sin(theta);
            sampleCount++;
        }
    }
    imageStore(s_target, globalId, vec4(PI * color / sampleCount, 1.0));
}
