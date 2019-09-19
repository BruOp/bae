$input a_position, a_normal, a_texcoord0, a_tangent
$output v_position, v_texcoord, v_normal, v_tangent, v_bitangent


#include "../common/common.sh"

uniform mat4 u_normalTransform;

void main()
{
    v_position = mul(u_model[0], vec4(a_position, 1.0)).xyz;

    v_normal    = normalize(mul(u_normalTransform, vec4(a_normal, 0.0)).xyz);
    v_tangent   = normalize(mul(u_model[0], vec4(a_tangent.xyz, 0.0)).xyz);
    v_bitangent = normalize(cross(v_normal, v_tangent)) * a_tangent.w;

    v_texcoord = a_texcoord0;

    gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));
}
