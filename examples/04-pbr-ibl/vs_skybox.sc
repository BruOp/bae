$input a_position, a_texcoord0
$output v_position, v_texcoord

#include "../common/common.sh"

void main()
{
	gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));
    gl_Position.z = 1.0;
    v_texcoord = a_texcoord0;
}
