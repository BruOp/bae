$input v_texcoord

#include "../common/common.sh"

SAMPLER2D(s_input, 0);

void main()
{
  gl_FragColor = vec4_splat(texture2D(s_input, v_texcoord).r);
}
