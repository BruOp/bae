$input v_position, v_normal, v_tangent, v_bitangent, v_texcoord

#include "../common/common.sh"

// Material
SAMPLER2D(diffuseMap, 0);
SAMPLER2D(normalMap, 1);
SAMPLER2D(metallicRoughnessMap, 2);

void main()
{
    mat3 tbn = mat3FromCols(
        normalize(v_tangent),
        normalize(v_bitangent),
        normalize(v_normal)
    );
    vec3 normal = texture2D(normalMap, v_texcoord).xyz * 2.0 - 1.0;
    normal = normalize(mul(tbn, normal));

    vec3 diffuse = texture2D(diffuseMap, v_texcoord).rgb;
    vec3 OccRoughMetal = texture2D(metallicRoughnessMap, v_texcoord).xyz;

    gl_FragData[0] = vec4(diffuse, OccRoughMetal.y);
    gl_FragData[1] = vec4(normal, OccRoughMetal.z);
    gl_FragData[2] = gl_FragCoord.z;
}
