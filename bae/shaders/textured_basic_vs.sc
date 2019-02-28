$input a_position, a_texcoord0
$output v_texcoord

#include <bgfx_shader.sh>

void main()
{
    v_texcoord = a_texcoord0;

    gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));
}
