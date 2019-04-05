#include <bgfx_compute.sh>

#define GROUP_SIZE 256
#define THREADS_X 16
#define THREADS_Y 16

#define EPSILON 0.0001
// Taken from RTR vol 4 pg. 278
#define RGB_TO_LUM float3(0.2125, 0.7154, 0.0721)

// Uniforms:
IMAGE2D_RW(s_target, r16f, 0);
BUFFER_RW(histogram, uint, 1);

#define minLogLum u_params.x
#define logLumRange u_params.y
#define timeCoeff u_params.z
#define numPixels u_params.w
vec4 u_params;

// Shared
SHARED uint histogramShared[GROUP_SIZE];

#define groupIndex gl_LocalInvocationIndex

NUM_THREADS(THREADS_X, THREADS_Y, 1)
void main() {
  uint countForThisBin = histogram[groupIndex];
  histogramShared[groupIndex] = countForThisBin * groupIndex;

  groupMemoryBarrier();

  histogram[groupIndex] = 0;

  UNROLL
  for (uint binIndex = (GROUP_SIZE >> 1); binIndex > 0; binIndex >>= 1) {
    if (uint(groupIndex) < binIndex) {
      histogramShared[groupIndex] += histogramShared[groupIndex + binIndex];
    }

    groupMemoryBarrier();
  }

  if (groupIndex == 0) {
    float weightedLogAverage = (histogramShared[0] / max(numPixels - float(countForThisBin), 1.0)) - 1.0;
    float weightedAvgLum = exp2(((weightedLogAverage / 254.0) * logLumRange) + minLogLum);
    float lumLastFrame = imageLoad(s_target, ivec2(0, 0)).x;
    float adaptedLum = lumLastFrame + (weightedAvgLum - lumLastFrame) * timeCoeff;
    imageStore(s_target, ivec2(0, 0), vec4(adaptedLum, 0., 0., 0.));
  }
}
