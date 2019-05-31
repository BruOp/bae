$input v_texcoord

/*
 * Copyright 2011-2019 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
 */

#include "../common/common.sh"

SAMPLER2D(s_diffuseRT, 0);

void main()
{
  vec3 rgb = texture2D(s_diffuseRT, v_texcoord).rgb;
  gl_FragColor = toGammaAccurate(vec4(rgb, 1.0));
}
