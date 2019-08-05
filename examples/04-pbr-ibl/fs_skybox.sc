$input v_position, v_texcoord

#include "../common/common.sh"

SAMPLERCUBE(s_envMap, 0);
uniform mat4 u_invRotationViewProj;

void main()
{
    vec4 clipSpace = vec4(2.0 * v_texcoord - 1.0, 1.0, 1.0);
    vec3 dir = normalize(mul(u_invRotationViewProj, clipSpace).xyz);
    gl_FragColor = textureCube(s_envMap, dir);
}
