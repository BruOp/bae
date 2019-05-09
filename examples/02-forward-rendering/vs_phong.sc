// $input a_position, a_normal, a_tangent, a_bitangent, a_color0, a_texcoord0
// $output v_position, v_normal, v_tangent, v_bitangent, v_texcoord
$input a_position, a_normal, a_texcoord0
$output v_position, v_texcoord, v_normal

#include "../common/common.sh"

void main()
{
    v_position = mul(u_model[0], vec4(a_position, 1.0)).xyz;

    v_normal = a_normal;
    // v_normal    = mul(u_model[0], vec4(a_normal, 1.0)).xyz;
    // v_tangent   = mul(u_model[0], a_tangent).xyz;
    // v_bitangent = mul(u_model[0], a_bitangent).xyz;

    v_texcoord = a_texcoord0;

    gl_Position = mul(u_viewProj, vec4(v_position, 1.0));
}
