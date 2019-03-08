$input a_position, a_normal, a_tangent, a_texcoord0
$output v_position, v_normal, v_texcoord, v_tangent, v_bitangent

#include <bgfx_shader.sh>

void main()
{
    v_position = mul(u_model[0], vec4(a_position, 1.0)).xyz;

    v_normal = mul(u_model[0], vec4(a_normal, 0.0)).xyz;

    v_tangent = mul(u_model[0], vec4(a_tangent.xyz, 0.0)).xyz;

    // glTF uses a_tangent.w to define handedness
    vec3 bitangent = cross(a_normal, a_tangent.xyz) * a_tangent.w;
    v_bitangent = mul(u_model[0], vec4(bitangent, 0.0)).xyz;

    v_texcoord = a_texcoord0;

    gl_Position = mul(u_viewProj, vec4(v_position, 1.0));
}
