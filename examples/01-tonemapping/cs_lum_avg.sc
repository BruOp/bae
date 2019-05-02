#include "bgfx_compute.sh"

#define GROUP_SIZE 256
// Uniforms:
uniform vec4 u_params;
#define minLogLum u_params.x
#define logLumRange u_params.y
#define timeCoeff u_params.z
#define numPixels u_params.w

IMAGE2D_RW(s_target, r16f, 0);
BUFFER_RW(histogram, uint, 1);

// Shared
SHARED uint histogramShared[GROUP_SIZE];

NUM_THREADS(GROUP_SIZE, 1, 1)
void main() {
  uint countForThisBin = histogram[gl_LocalInvocationIndex];
  histogramShared[gl_LocalInvocationIndex] = countForThisBin * gl_LocalInvocationIndex;

  groupMemoryBarrier();

  histogram[gl_LocalInvocationIndex] = 0;

  UNROLL
  for (uint binIndex = (GROUP_SIZE >> 1); binIndex > 0; binIndex >>= 1) {
    if (uint(gl_LocalInvocationIndex) < binIndex) {
      histogramShared[gl_LocalInvocationIndex] += histogramShared[gl_LocalInvocationIndex + binIndex];
    }

    groupMemoryBarrier();
  }

  if (gl_LocalInvocationIndex == 0) {
    float weightedLogAverage = (histogramShared[0] / max(numPixels - float(countForThisBin), 1.0)) - 1.0;
    float weightedAvgLum = exp2(weightedLogAverage / 254.0 * logLumRange + minLogLum);
    float lumLastFrame = imageLoad(s_target, ivec2(0, 0)).x;
    float adaptedLum = lumLastFrame + (weightedAvgLum - lumLastFrame) * timeCoeff;
    imageStore(s_target, ivec2(0, 0), vec4(adaptedLum, 0.0, 0.0, 0.0));
  }
}
