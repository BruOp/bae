#include <bgfx_compute.sh>

#define GROUP_SIZE 256
#define THREADS_X 16
#define THREADS_Y 16

#define EPSILON 0.0001
// Taken from RTR vol 4 pg. 278
#define RGB_TO_LUM vec3(0.2125, 0.7154, 0.0721)

// Uniforms:
IMAGE2D_RO(s_texColor, rgba16f, 0);
BUFFER_RW(histogram, uint, 1);

#define minLogLum u_params.x
#define inverseLogLumRange u_params.y
vec4 u_params;

// Shared
SHARED uint histogramShared[GROUP_SIZE];

#define groupIndex gl_LocalInvocationIndex
#define threadId gl_GlobalInvocationID.xy

uint colorToBin(vec3 hdrColor) {
  float luminence = dot(hdrColor, RGB_TO_LUM);
  if (luminence < EPSILON) {
    return 0;
  }

  float logLum = clamp((log2(luminence) - minLogLum) * inverseLogLumRange, 0.0, 1.0);
  return uint(logLum * 254.0 + 1.0);
}

NUM_THREADS(THREADS_X, THREADS_Y, 1)
void main() {
  histogramShared[groupIndex] = 0;

  groupMemoryBarrier();
  uvec2 dim = imageSize(s_texColor).xy;
	if (threadId.x < dim.x && threadId.y < dim.y) {
    vec3 hdrColor = imageLoad(s_texColor, ivec2(threadId)).xyz;
    uint binIndex = colorToBin(hdrColor);
    atomicAdd(histogramShared[binIndex], 1);
  }

  groupMemoryBarrier();

  atomicAdd(histogram[groupIndex], histogramShared[groupIndex]);
}
