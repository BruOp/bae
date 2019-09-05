$input v_position, v_normal, v_tangent, v_bitangent, v_texcoord

#include "../common/common.sh"

// Material
SAMPLER2D(s_baseColor, 0);
SAMPLER2D(s_normal, 1);
SAMPLER2D(s_metallicRoughness, 2);
SAMPLER2D(s_emissive, 3);
SAMPLER2D(s_occlusion, 4);
uniform vec4 u_factors[3];
#define u_baseColorFactor u_factors[0]
#define u_emissiveFactor u_factors[1]
#define u_alphaCutoff u_factors[2].x
#define u_metallicFactor u_factors[2].y
#define u_roughnessFactor u_factors[2].z

void main()
{
    vec4 baseColor = toLinear(texture2D(s_baseColor, v_texcoord)) * u_baseColorFactor;
    if (baseColor.w < u_alphaCutoff) {
        discard;
        return;
    }

    vec3 normal = texture2D(s_normal, v_texcoord).xyz * 2.0 - 1.0;
    // From the MikkTSpace docs! (check mikktspace.h)
    normal = normalize(normal.x * v_tangent + normal.y * v_bitangent + normal.z * v_normal);

    vec2 roughnessMetal = texture2D(s_metallicRoughness, v_texcoord).yz;
    float roughness = roughnessMetal.x * u_roughnessFactor;
    float metallic = roughnessMetal.y * u_metallicFactor;
    float occlusion = texture2D(s_occlusion, v_texcoord).x;
    vec3 emissive = toLinear(texture2D(s_emissive, v_texcoord)).xyz * u_emissiveFactor;

    gl_FragData[0] = vec4(baseColor.xyz, roughness);
    gl_FragData[1] = vec4(normal, metallic);
    gl_FragData[2] = vec4(emissive, occlusion);
    gl_FragData[3] = gl_FragCoord.z;

}
