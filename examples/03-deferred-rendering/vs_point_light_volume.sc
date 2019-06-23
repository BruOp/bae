$input a_position
$output v_screenPos

#include "../common/common.sh"

void main()
{
	gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0) );
	v_screenPos = gl_Position;
}
