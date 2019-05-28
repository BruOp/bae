$input v_texcoord0

/*
 * Copyright 2011-2019 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
 */

#include "../common/common.sh"

SAMPLER2D(s_texColor, 0);
SAMPLER2D(s_texAvgLum, 1);

uniform vec4 u_tonemap;

void main()
{
  vec3 rgb = texture2D(s_texColor, v_texcoord0).rgb;
  float lum = texture2D(s_texAvgLum, v_texcoord0).r;

  vec3 Yxy = convertRGB2Yxy(rgb);

  float middleGray = u_tonemap.x;
  // float whiteSqr   = u_tonemap.y;

  Yxy.x /= (9.6 * lum + 0.0001);

  rgb = convertYxy2RGB(Yxy);

  rgb.x = Tonemap_Uchimura(rgb.x);
  rgb.y = Tonemap_Uchimura(rgb.y);
  rgb.z = Tonemap_Uchimura(rgb.z);
  gl_FragColor = toGammaAccurate(vec4(rgb, 1.0) );
}
