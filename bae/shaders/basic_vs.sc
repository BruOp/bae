$input a_position, a_normal
$output v_color0

uniform vec4 color;

#include <bgfx_shader.sh>

void main()
{
    gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0) );
    v_color0 = vec4(a_normal, 1.0);
}
