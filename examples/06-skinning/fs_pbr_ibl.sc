$input v_position, v_normal, v_tangent, v_bitangent, v_texcoord

#include "../common/common.sh"
#include "../common/pbr_helpers.sh"

// Scene
uniform vec4 u_envParams;
#define numEnvLevels u_envParams.x
#define iblMode u_envParams.y

uniform vec4 u_cameraPos;

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

// IBL Stuff
SAMPLER2D(s_brdfLUT, 5);
SAMPLERCUBE(s_prefilteredEnv, 6);
SAMPLERCUBE(s_irradiance, 7);


void main()
{
    vec4 baseColor = toLinear(texture2D(s_baseColor, v_texcoord)) * u_baseColorFactor;
#ifdef MASKING_ENABLED
    if (baseColor.w < u_alphaCutoff) {
        discard;
        return;
    }
#endif // MASKING_ENABLED

    vec3 normal = texture2D(s_normal, v_texcoord).xyz * 2.0 - 1.0;
    // From the MikkTSpace docs! (check mikktspace.h)
    normal = normalize(normal.x * v_tangent + normal.y * v_bitangent + normal.z * v_normal);

    vec3 viewDir = normalize(u_cameraPos.xyz - v_position);
    vec3 lightDir = reflect(-viewDir, normal);
    vec3 H = normalize(lightDir + viewDir);
    float VoH = clampDot(viewDir, H);
    float NoV = clamp(dot(normal, viewDir), 1e-5, 1.0);


    vec2 roughnessMetal = texture2D(s_metallicRoughness, v_texcoord).yz;
    float roughness = max(roughnessMetal.x * u_roughnessFactor, MIN_ROUGHNESS);
    float metallic = roughnessMetal.y * u_metallicFactor;
    float occlusion = texture2D(s_occlusion, v_texcoord).x;
    vec3 emissive = toLinear(texture2D(s_emissive, v_texcoord)).xyz * u_emissiveFactor.xyz;

    // From GLTF spec
    vec3 c_diff = diffuseColor(baseColor.rgb, metallic);
    vec3 F0 = mix(vec3_splat(DIELECTRIC_SPECULAR), baseColor.xyz, metallic);

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
        vec3 k_D = c_diff * (1.0 - FssEss - FmsEms);
        color = FssEss * radiance + (FmsEms + k_D) * irradiance;
    } else {
        // Single scattering, from Karis
        color = FssEss * radiance + c_diff * irradiance;
    }

    gl_FragColor = vec4(color * occlusion + emissive, baseColor.w);
}
