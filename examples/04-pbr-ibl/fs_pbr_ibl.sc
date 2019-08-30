$input v_position, v_normal, v_tangent, v_bitangent, v_texcoord

#include "../common/common.sh"
#include "../common/pbr_helpers.sh"

// Scene
uniform vec4 u_envParams;
#define numEnvLevels u_envParams.x
#define iblMode u_envParams.y

uniform vec4 u_cameraPos;

// Material
SAMPLER2D(s_diffuseMap, 0);
SAMPLER2D(s_normalMap, 1);
SAMPLER2D(s_metallicRoughnessMap, 2);

// IBL Stuff
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
    vec3 lightDir = reflect(-viewDir, normal);
    vec3 H = normalize(lightDir + viewDir);
    float VoH = clampDot(viewDir, H);
    float NoV = clamp(dot(normal, viewDir), 1e-5, 1.0);

    vec4 baseColor = texture2D(s_diffuseMap, v_texcoord);
    vec3 OccRoughMetal = texture2D(s_metallicRoughnessMap, v_texcoord).xyz;
    float roughness = max(OccRoughMetal.y, MIN_ROUGHNESS);
    float metalness = OccRoughMetal.z;

    // From GLTF spec
    vec3 diffuseColor = baseColor.rgb * (1.0 - DIELECTRIC_SPECULAR) * (1.0 - metalness);
    vec3 F0 = mix(vec3_splat(DIELECTRIC_SPECULAR), baseColor.xyz, metalness);

    // Load env textures
    vec2 f_ab = texture2D(s_brdfLUT, vec2(NoV, roughness)).xy;
    float lodLevel = roughness * numEnvLevels;
    vec3 radiance = textureCubeLod(s_prefilteredEnv, lightDir, lodLevel).xyz;
    vec3 irradiance = textureCubeLod(s_irradiance, normal, 0).xyz;

    vec3 k_S = F0;
    if (iblMode == 2.0) {
        // Roughness dependent fresnel, from Fdez-Aguera
        vec3 Fr = max(vec3_splat(1.0 - roughness), F0) - F0;
        k_S += Fr * pow(1.0 - NoV, 5.0);
    }

    vec3 FssEss = k_S * f_ab.x + f_ab.y;

    vec3 color;
    if (iblMode >= 1.0) {
        // Multiple scattering, from Fdez-Aguera
        float Ems = (1.0 - (f_ab.x + f_ab.y));
        vec3 F_avg = F0 + (1.0 - F0) / 21.0;
        vec3 FmsEms = Ems * FssEss * F_avg / (1.0 - F_avg * Ems);
        vec3 k_D = diffuseColor * (1.0 - FssEss - FmsEms);
        color = FssEss * radiance + (FmsEms + k_D) * irradiance;
    } else {
        // Single scattering, from Karis
        color = FssEss * radiance + diffuseColor * irradiance;
    }

    gl_FragColor = vec4(color, baseColor.w);
}