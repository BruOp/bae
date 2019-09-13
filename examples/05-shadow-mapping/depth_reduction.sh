#include "bgfx_compute.sh"

#define GROUP_SIZE 64
#define THREADS_X 8
#define THREADS_Y 8

// Uniforms:
uniform vec4 u_params;
#define nearZ u_params.z
#define farZ u_params.w
uniform mat4 u_projection;

#ifdef INITIAL_PASS
SAMPLER2D(s_inputDepthMap, 0);
#else
IMAGE2D_RO(s_inputDepthMap, rg16f, 0);
#endif

IMAGE2D_WR(s_output, rg16f, 1);

// Shared
SHARED vec2 depthShared[GROUP_SIZE];

NUM_THREADS(THREADS_X, THREADS_Y, 1)
void main() {

#ifdef INITIAL_PASS
  vec2 sampleUV = vec2(gl_GlobalInvocationID.xy) / (u_params.xy - 1.0);
#else
  ivec2 imageDim = ivec2(u_params.xy);
  ivec2 sampleUV = min(imageDim - 1, ivec2(gl_GlobalInvocationID.xy));
#endif

#ifdef INITIAL_PASS
  // NOTE: This doesn't support multisampled depth maps.
  float sampledDepth = texture2DLod(s_inputDepthMap, sampleUV, 0).r;
  if (sampledDepth < 1.0) {
    // Transform to linear depth, taken from MJP
#if BGFX_SHADER_LANGUAGE_GLSL
    sampledDepth = u_projection[3][2] / (sampledDepth - u_projection[2][2]);
#else
    sampledDepth = u_projection[2][3] / (sampledDepth - u_projection[2][2]);
#endif
    sampledDepth = saturate((sampledDepth - nearZ) / (farZ - nearZ));
    depthShared[gl_LocalInvocationIndex] = vec2_splat(sampledDepth);
  } else {
    depthShared[gl_LocalInvocationIndex] = vec2(1.0, 0.0);
  }

#else

  vec2 sampledDepth = imageLoad(s_inputDepthMap, sampleUV).rg;
  if (sampledDepth.x == 0.0) {
    sampledDepth.x = 1.0;
  }
  depthShared[gl_LocalInvocationIndex] = sampledDepth;

#endif

  groupMemoryBarrier();

  UNROLL
  for (uint binIndex = (GROUP_SIZE >> 1); binIndex > 0; binIndex >>= 1) {
    if (uint(gl_LocalInvocationIndex) < binIndex) {
      // Check two mins, store the min
      depthShared[gl_LocalInvocationIndex].x = min(
        depthShared[gl_LocalInvocationIndex].x,
        depthShared[gl_LocalInvocationIndex + binIndex].x
      );
      // Check two maxes, store the max
      depthShared[gl_LocalInvocationIndex].y = max(
        depthShared[gl_LocalInvocationIndex].y,
        depthShared[gl_LocalInvocationIndex + binIndex].y
      );
    }

    groupMemoryBarrier();
  }

  if (gl_LocalInvocationIndex == 0) {
    // Here we using the gl_WorkGroupID since each thread group will have it's
    // own red-green pixel in the stored image
    imageStore(
      s_output,
      gl_WorkGroupID.xy,
      vec4(depthShared[0], 0.0, 0.0)
    );
  }
}
