$input a_position, a_texcoord0
$output v_texcoord
/*
 * Copyright 2011-2019 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
 */

#include "../examples/common/common.sh"

uniform vec4 u_tonemap;

void main()
{
	gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0) );
	v_texcoord = a_texcoord0;
}


