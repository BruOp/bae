$input a_position, a_normal
$output v_position, v_normal

#include <bgfx_shader.sh>

void main()
{
    v_position = mul(u_model[0], vec4(a_position, 1.0)).xyz;
    gl_Position = mul(u_viewProj, vec4(v_position, 1.0));

    v_normal = mul(u_model[0], vec4(a_normal, 0.0)).xyz;
}
