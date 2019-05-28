$input v_texcoord

/*
 * Copyright 2011-2019 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
 */

#include "../examples/common/common.sh"

SAMPLER2D(s_texColor, 0);
SAMPLER2D(s_texAvgLum, 1);

void main()
{
  vec3 color = texture2D(s_texColor, v_texcoord).rgb;
  float lum = texture2D(s_texAvgLum, v_texcoord).r;

  vec3 Yxy = convertRGB2Yxy(color);

  Yxy.x /= (9.6 * lum + 0.0001);

  color = convertYxy2RGB(Yxy);

  color.r = Tonemap_Uchimura(color.r);
  color.g = Tonemap_Uchimura(color.g);
  color.b = Tonemap_Uchimura(color.b);
  gl_FragColor = toGammaAccurate(vec4(color, 1.0));
}
