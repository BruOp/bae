$input v_position, v_texcoord, v_normal, v_tangent, v_bitangent

#include "../common/common.sh"

SAMPLER2D(diffuseMap, 0);
SAMPLER2D(normalMap, 1);
SAMPLER2D(specularMap, 2);

void main()
{
    mat3 tbn = mat3(
        normalize(v_bitangent),
        normalize(v_tangent),
        normalize(v_normal)
    );
    vec3 normal = texture2D(normalMap, v_texcoord).xyz * 2.0 - 1.0;
    normal = normalize(mul(tbn, normal));

    vec3 color = 0.5 * normal + 0.5;
    gl_FragColor = vec4(color, 1.0);
}
