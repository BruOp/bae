// $input v_position, v_normal, v_tangent, v_bitangent, v_texcoord
$input v_position, v_texcoord, v_normal

#include "../common/common.sh"

void main()
{
    vec3 color = 0.5 * normalize(v_normal) + 0.5;
    gl_FragColor = vec4(color, 1.0);
}
