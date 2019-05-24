$input a_position, a_normal, a_texcoord0, a_tangent, a_bitangent
$output v_position, v_texcoord, v_normal, v_tangent, v_bitangent

#include "../examples/common/common.sh"

uniform mat4 normalTransform;

void main()
{
    v_position = mul(u_model[0], vec4(a_position, 1.0)).xyz;

    v_normal    = normalize(mul(normalTransform, vec4(a_normal, 0.0)).xyz);
    v_tangent   = normalize(mul(u_model[0], vec4(a_tangent, 0.0)).xyz);
    v_bitangent = normalize(mul(u_model[0], vec4(a_bitangent, 0.0)).xyz);

    v_texcoord = a_texcoord0;

    gl_Position = mul(u_viewProj, vec4(v_position, 1.0));
}
