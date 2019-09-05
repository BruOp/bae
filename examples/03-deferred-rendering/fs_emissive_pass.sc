$input v_texcoord

#include "../common/common.sh"

SAMPLER2D(s_emissiveOcclusion, 0);

void main()
{
    vec3 emissive = texture2D(s_emissiveOcclusion, v_texcoord).rgb;
    gl_FragColor = vec4(emissive, 1.0);
}
