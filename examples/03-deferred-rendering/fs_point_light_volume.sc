
#include "../common/common.sh"

SAMPLER2D(diffuseRT, 0);
SAMPLER2D(normalRT, 1);

uniform vec4 colorIntensity;
uniform vec4 posRadius;

void main()
{
  vec2 texcoord = gl_FragCoord.xy / u_viewRect.xy;
  vec3 diffuse = texture2D(diffuseRT, texcoord).rgb;
  gl_FragColor = vec4(diffuse, 1.0);
}
