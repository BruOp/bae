$input v_position, v_normal, v_tangent, v_bitangent, v_texcoord

#include "../common/common.sh"
#include "../common/pbr_helpers.sh"

#define MAX_LIGHT_COUNT 255u

// Scene
uniform vec4 u_cameraPos;
uniform vec4 pointLight_params;
uniform vec4 pointLight_colorIntensity[MAX_LIGHT_COUNT];
uniform vec4 pointLight_pos[MAX_LIGHT_COUNT];


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


vec3 specular(vec3 lightDir, vec3 viewDir, vec3 normal, vec3 baseColor, float roughness, float metallic) {
    vec3 h = normalize(lightDir + viewDir);
    float NoV = clamp(dot(normal, viewDir), 1e-5, 1.0);
    float NoL = clampDot(normal, lightDir);
    float NoH = clampDot(normal, h);
    float VoH = clampDot(viewDir, h);

    // Needs to be a uniform

    float D = D_GGX(NoH, roughness);
    vec3  F = F_Schlick(VoH, metallic, baseColor);
    float V = V_SmithGGXCorrelated(NoV, NoL, roughness);
    return vec3(D * V * F);
}

float karisFalloff(float dist, float lightRadius) {
    return pow(clamp(1.0 - pow(dist/lightRadius, 4), 0.0, 1.0), 2) / (dist * dist + 1);
}

void main()
{
    vec3 normal = texture2D(s_normal, v_texcoord).xyz * 2.0 - 1.0;
    // From the MikkTSpace docs!
    normal = normalize( normal.x * v_tangent + normal.y * v_bitangent + normal.z * v_normal );

    vec3 viewDir = normalize(u_cameraPos.xyz - v_position);

    vec4 baseColor = toLinear(texture2D(s_baseColor, v_texcoord)) * u_baseColorFactor;
    vec2 roughnessMetal = texture2D(s_metallicRoughness, v_texcoord).yz;
    float roughness = max(roughnessMetal.x * u_roughnessFactor, MIN_ROUGHNESS);
    float metallic = roughnessMetal.y * u_metallicFactor;
    float occlusion = texture2D(s_occlusion, v_texcoord).x;
    vec3 emissive = toLinear(texture2D(s_emissive, v_texcoord)).xyz * u_emissiveFactor;

    vec3 color = vec3(0.0, 0.0, 0.0);
    uint numLights = min(floatBitsToUint(pointLight_params.x), MAX_LIGHT_COUNT);
    for (uint i = 0; i < numLights; i++) {
        vec3 lightPos = pointLight_pos[i].xyz;
        float lightRadius = pointLight_pos[i].w;
        vec4 colorIntensity = pointLight_colorIntensity[i];
        vec3 lightDir = lightPos - v_position;
        float dist = length(lightDir);
        lightDir = lightDir / dist;

        float attenuation = karisFalloff(dist, lightRadius) * colorIntensity.w;
        if (attenuation == 0.0) {
            continue;
        }

        vec3 light = attenuation * colorIntensity.xyz * clampDot(normal, lightDir);

        color += (
            diffuseColor(baseColor.xyz, metallic) +
            PI * specular(lightDir, viewDir, normal, baseColor.xyz, roughness, metallic)
        ) * light;
    }
    gl_FragColor = vec4(color * occlusion + emissive, 1.0);
}
