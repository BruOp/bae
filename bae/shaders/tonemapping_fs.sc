$input v_texcoord

/*
 * Copyright 2011-2019 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
 */

#include "./common/common.sh"

uniform vec4 u_params;

SAMPLER2D(s_texColor, 0);
SAMPLER2D(s_texAvgLuminance, 1);

float reinhard2(float _x, float _whiteSqr)
{
	return (_x * (1.0 + _x/_whiteSqr) ) / (1.0 + _x);
}

vec3 reinhard2(vec3 _x, float _whiteSqr)
{
	return (_x * (1.0 + _x/_whiteSqr) ) / (1.0 + _x);
}

void main()
{
  vec3 rgb = texture2D(s_texColor, v_texcoord).rgb;
  float avgLum = texture2D(s_texAvgLuminance, v_texcoord).r;

  vec3 Yxy = convertRGB2Yxy(rgb);

  float middleGray = u_params.x;
  float whiteSqr   = u_params.y;

  float lp = Yxy.x * middleGray / (avgLum + 0.0001);
  Yxy.x = reinhard2(lp, whiteSqr);

  rgb = convertYxy2RGB(Yxy);

  gl_FragColor = vec4(toGamma(rgb), 1.0);
}
