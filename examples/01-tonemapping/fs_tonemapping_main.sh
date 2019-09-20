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

  float whiteSqr   = u_tonemap.x;

  Yxy.x /= (9.6 * lum + 0.0001);

  rgb = convertYxy2RGB(Yxy);

#ifdef USE_REINHARD
  rgb = reinhard2(rgb, whiteSqr);
  gl_FragColor = toGammaAccurate(vec4(rgb, 1.0) );
#endif

#ifdef USE_LOTTES
  rgb.x = Tonemap_Lottes(rgb.x);
  rgb.y = Tonemap_Lottes(rgb.y);
  rgb.z = Tonemap_Lottes(rgb.z);
  gl_FragColor = toGammaAccurate(vec4(rgb, 1.0) );
#endif

#ifdef USE_UCHIMURA
  rgb.x = Tonemap_Uchimura(rgb.x);
  rgb.y = Tonemap_Uchimura(rgb.y);
  rgb.z = Tonemap_Uchimura(rgb.z);
  gl_FragColor = toGammaAccurate(vec4(rgb, 1.0) );
#endif

#ifdef USE_UNREAL
  // Gamma is baked in
  rgb.x = Tonemap_Unreal(rgb.x);
  rgb.y = Tonemap_Unreal(rgb.y);
  rgb.z = Tonemap_Unreal(rgb.z);
  gl_FragColor = vec4(rgb, 1.0);
#endif
}
