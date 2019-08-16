$input v_position, v_normal, v_tangent, v_bitangent, v_texcoord

#include "../common/common.sh"
#include "pbr_helpers.sh"

#define MAX_LIGHT_COUNT 255
#define DIELECTRIC_SPECULAR 0.04
#define BLACK vec3(0.0, 0.0, 0.0)

// Scene
uniform vec4 u_envParams;
#define numEnvLevels u_envParams.x

uniform vec4 u_cameraPos;

// Material
SAMPLER2D(s_diffuseMap, 0);
SAMPLER2D(s_normalMap, 1);
SAMPLER2D(s_metallicRoughnessMap, 2);

// IRB Stuff
SAMPLER2D(s_brdfLUT, 3);
SAMPLERCUBE(s_prefilteredEnv, 4);
SAMPLERCUBE(s_irradiance, 5);

void main()
{
    mat3 tbn = mat3FromCols(
        normalize(v_tangent),
        normalize(v_bitangent),
        normalize(v_normal)
    );
    vec3 normal = texture2D(s_normalMap, v_texcoord).xyz * 2.0 - 1.0;
    normal = normalize(mul(tbn, normal));

    vec3 viewDir = normalize(u_cameraPos.xyz - v_position);

    vec4 baseColor = texture2D(s_diffuseMap, v_texcoord);
    vec3 OccRoughMetal = texture2D(s_metallicRoughnessMap, v_texcoord).xyz;

    vec3 lightDir = reflect(-viewDir, normal);
    vec3 H = normalize(lightDir + viewDir);
    float VoH = clampDot(viewDir, H);
    float NoV = clamp(dot(normal, viewDir), 1e-5, 1.0);

    float roughness = OccRoughMetal.y;
    float metalness = OccRoughMetal.z;
    // From Aguera 2019
    vec3 F0 = mix(vec3_splat(DIELECTRIC_SPECULAR), baseColor.xyz, metalness);

    vec2 f_ab = texture2D(s_brdfLUT, vec2(NoV, roughness)).xy;
    float lodLevel = roughness * numEnvLevels;
    vec3 radiance = textureCubeLod(s_prefilteredEnv, lightDir, lodLevel).xyz;
    vec3 irradiance = textureCubeLod(s_irradiance, normal, 0).xyz;

    vec3 Fr = max(vec3_splat(1.0 - roughness), F0) - F0;
    vec3 k_S = F0 + F0 * pow(1.0 - NoV, 5.0);

    vec3 FssEss = k_S * f_ab.x + f_ab.y;

    float Ess = f_ab.x + f_ab.y;
    float Ems = 1.0 - Ess;
    vec3 F_avg = F0 + (1.0 - F0) / 21.0;
    vec3 Fms = FssEss / (1.0 - (1.0 - Ess) * F_avg);

    // // For Dielectrics
    // float diffuseColor = mix(baseColor.rgb * (1.0 - DIELECTRIC_SPECULAR), BLACK, metalness) * irradiance;
    // vec3 color = radiance * (F0 * f_ab.x + f_ab.y) + diffuseColor;

    vec3 k_D = baseColor.xyz * (1 - (FssEss + Fms * Ems));
    vec3 color = FssEss * radiance + (Fms * Ems + k_D) * irradiance * baseColor.w * OccRoughMetal.x;

    gl_FragColor = vec4(color, baseColor.w);
}
